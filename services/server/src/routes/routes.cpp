#include "routes/routes.hpp"

#include "telemetry/telemetry.hpp"

namespace ServerMessages
{

auto server_service::CheckOnline(
        ServerContext *context,
        const CheckOnlineRequest * /*request*/,
        CheckOnlineReply * /*reply*/
) -> Status
{
    trace_span trace(*context);
    return Status::OK;
}

auto server_service::ExitServer(
        ServerContext *context,
        const ExitServerRequest * /*request*/,
        ExitServerReply * /*reply*/
) -> Status
{
    trace_span trace(*context);
    if (exit_sem_ != nullptr) {
        (void)sem_post(exit_sem_);
    }
    return Status::OK;
}

} // namespace ServerMessages
