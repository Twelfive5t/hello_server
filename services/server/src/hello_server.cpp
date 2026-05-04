#include "logger/logger.hpp"
#include "routes/routes.hpp"
#include "telemetry/telemetry.hpp"

#include <cstdlib>
#include <exception>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

namespace
{

auto otlp_endpoint() -> std::string
{
    if (const char *endpoint = std::getenv("TELEMETRY_OTLP_ENDPOINT"); endpoint != nullptr) {
        return endpoint;
    }

    return "localhost:4317";
}

void run_server()
{
    const std::string server_address = "0.0.0.0:50051";

    init_tracer({ .service_name = "hello_server", .endpoint = otlp_endpoint() });

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    spdlog::info("Initializing gRPC ServerBuilder...");

    // 1. 构建 builder
    grpc::ServerBuilder builder;

    // 2. 绑定端口（不安全模式）
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // 3. 注册 service
    ServerMessages::server_service service;
    builder.RegisterService(&service);

    // 4. 启动 server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    if (!server) {
        spdlog::error(
                "BuildAndStart() returned nullptr - failed to start server on {}", server_address
        );
        return;
    }

    spdlog::info("Server listening on {}", server_address);

    // 5. 阻塞等待
    server->Wait();

    spdlog::info("Server shut down.");
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