#include "server_messages.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>
#include <semaphore.h>
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
    sem_t *exit_sem_{};

public:
    explicit server_service() = default;
    ~server_service() override = default;
    server_service(const server_service &) = delete;
    auto operator=(const server_service &) -> server_service & = delete;
    server_service(server_service &&) = delete;
    auto operator=(server_service &&) -> server_service & = delete;

    auto CheckOnline(
            ServerContext *context,
            const CheckOnlineRequest *request,
            CheckOnlineReply *reply
    ) -> Status override;

    auto ExitServer(
            ServerContext *context,
            const ExitServerRequest *request,
            ExitServerReply *reply
    ) -> Status override;

    void set_sem(sem_t *sem)
    {
        exit_sem_ = sem;
    }
};

} // namespace ServerMessages
