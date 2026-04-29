#include "logger/logger.hpp"
#include "routes/routes.hpp"

#include <grpcpp/grpcpp.h>

namespace
{

void run_server()
{
    const std::string SERVER_ADDRESS = "0.0.0.0:50051";

    // 1. 构建 builder
    grpc::ServerBuilder builder;

    // 2. 绑定端口（不安全模式）
    builder.AddListeningPort(SERVER_ADDRESS, grpc::InsecureServerCredentials());

    // 3. 注册 service
    ServerMessages::server_service service;
    builder.RegisterService(&service);

    // 4. 启动 server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    spdlog::info(std::string("Server listening on ") + SERVER_ADDRESS);

    // 5. 阻塞等待
    server->Wait();
}

} // namespace

auto main(int /*argc*/, char * /*argv*/[]) -> int
{
    init_logger();
    spdlog::info("Hello, Server!");
    run_server();
    return 0;
}