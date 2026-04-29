#include "routes/routes.hpp"

namespace ServerMessages
{

auto server_service::CheckOnline(
        ServerContext * /*context*/,
        const CheckOnlineRequest * /*request*/,
        CheckOnlineReply * /*reply*/
) -> Status
{
    return Status::OK;
}

} // namespace ServerMessages