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

} // namespace ServerMessages