#include "logger/logger.hpp"
#include "routes/routes.hpp"
#include "telemetry/telemetry.hpp"

#include <csignal>
#include <cstdlib>
#include <exception>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <pthread.h>
#include <thread>

namespace
{

constexpr std::string_view K_SERVER_ADDRESS = "0.0.0.0:50051";
constexpr auto K_SHUTDOWN_TIMEOUT = std::chrono::seconds(5);

auto otlp_endpoint() -> std::string
{
    if (const char *endpoint = std::getenv("TELEMETRY_OTLP_ENDPOINT"); endpoint != nullptr) {
        return endpoint;
    }
    return "localhost:4317";
}

// 屏蔽 SIGTERM/SIGINT，所有后续子线程继承此掩码，
// 确保信号仅由专用 signal_thread 通过 sigwait() 接收。
auto block_shutdown_signals() -> sigset_t
{
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGUSR1);
    (void)pthread_sigmask(SIG_BLOCK, &signal_set, nullptr);
    return signal_set;
}

auto build_server(ServerMessages::server_service &service) -> std::unique_ptr<grpc::Server>
{
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;
    install_grpc_server_metrics(builder);
    builder.AddListeningPort(std::string(K_SERVER_ADDRESS), grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    return builder.BuildAndStart();
}

// 等待 sem 触发后优雅关机；signal_thread 负责将系统信号转为 sem_post，
// shutdown_thread 负责在收到通知后以 deadline 调用 Shutdown。
void wait_for_shutdown(grpc::Server &server, sem_t &sem, sigset_t &signal_set)
{
    std::thread signal_thread([&sem, &signal_set]() {
        int sig = 0;
        (void)sigwait(&signal_set, &sig);
        if (sig != SIGUSR1) {
            spdlog::info("Signal {} received, initiating graceful shutdown...", sig);
            (void)sem_post(&sem);
        }
    });

    std::thread shutdown_thread([&server, &sem]() {
        (void)sem_wait(&sem);
        server.Shutdown(std::chrono::system_clock::now() + K_SHUTDOWN_TIMEOUT);
    });

    server.Wait();
    spdlog::info("Server shut down.");

    (void)pthread_kill(signal_thread.native_handle(), SIGUSR1);
    signal_thread.join();
    shutdown_thread.join();
}

void run_server()
{
    auto signal_set = block_shutdown_signals();

    init_tracer({ .service_name = "hello_server", .endpoint = otlp_endpoint() });

    ServerMessages::server_service service;
    auto server = build_server(service);
    if (!server) {
        spdlog::error(
                "BuildAndStart() returned nullptr - failed to start server on {}", K_SERVER_ADDRESS
        );
        return;
    }
    spdlog::info("Server listening on {}", K_SERVER_ADDRESS);

    sem_t sem;
    (void)sem_init(&sem, 0, 0);
    service.set_sem(&sem);

    wait_for_shutdown(*server, sem, signal_set);

    (void)sem_destroy(&sem);
    spdlog::default_logger()->flush();
    cleanup_tracer();
}

} // namespace

auto main(int /*argc*/, char * /*argv*/[]) -> int
{
    init_logger();
    spdlog::info("Hello, Server!");
    try {
        run_server();
    } catch (const std::exception &e) {
        spdlog::error("Exception in run_server: {}", e.what());
        return 1;
    } catch (...) {
        spdlog::error("Unknown exception in run_server");
        return 1;
    }
    return 0;
}
