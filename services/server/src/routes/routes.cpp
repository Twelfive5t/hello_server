#include "routes/routes.hpp"

#include "telemetry/telemetry.hpp"

#include <map>
#include <string>

namespace ServerMessages
{

auto server_service::CheckOnline(
        ServerContext *context,
        const CheckOnlineRequest * /*request*/,
        CheckOnlineReply * /*reply*/
) -> Status
{
    std::map<std::string, std::string> metadata;
    for (const auto &kv : context->client_metadata()) {
        metadata.emplace(
                std::string(kv.first.data(), kv.first.size()),
                std::string(kv.second.data(), kv.second.size())
        );
    }
    TRACE_POINT_WITH_CONTEXT(metadata);
    return Status::OK;
}

} // namespace ServerMessages