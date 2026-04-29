#include "server_messages.grpc.pb.h"

#include <atomic>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>

#pragma once
namespace ServerMessages
{
// 调用接口
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class server_service final : public ServerMessagesService::Service
{

private:
    std::atomic<bool> stop_flag_; // 停止标志
public:
    explicit server_service()
    {
        stop_flag_.store(false);
    }
    ~server_service() override
    {
        stop_flag_.store(true);
    }
    server_service(const server_service &) = delete;
    auto operator=(const server_service &) -> server_service & = delete;
    server_service(server_service &&) = delete;
    auto operator=(server_service &&) -> server_service & = delete;

    auto CheckOnline(
            ServerContext *context,
            const CheckOnlineRequest *request,
            CheckOnlineReply *reply
    ) -> Status override;
};

} // namespace ServerMessages
