#include "routes/routes.hpp"

#include "opentelemetry/common/key_value_iterable_view.h"
#include "opentelemetry/metrics/provider.h"
#include "telemetry/telemetry.hpp"

#include <chrono>
#include <map>
#include <string>

namespace ServerMessages
{

namespace
{

auto meter() -> opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter>
{
    static auto meter =
            opentelemetry::metrics::Provider::GetMeterProvider()->GetMeter("hello_server", "1.0.0");
    return meter;
}

auto request_counter()
        -> opentelemetry::nostd::unique_ptr<opentelemetry::metrics::Counter<std::uint64_t>> &
{
    static auto counter = meter()->CreateUInt64Counter(
            "rpc.server.requests",
            "Total number of gRPC requests handled by the server",
            "{request}"
    );
    return counter;
}

auto request_duration_histogram()
        -> opentelemetry::nostd::unique_ptr<opentelemetry::metrics::Histogram<double>> &
{
    static auto histogram = meter()->CreateDoubleHistogram(
            "rpc.server.duration", "End-to-end gRPC server request latency", "ms"
    );
    return histogram;
}

} // namespace

auto server_service::CheckOnline(
        ServerContext *context,
        const CheckOnlineRequest * /*request*/,
        CheckOnlineReply * /*reply*/
) -> Status
{
    const auto started_at = std::chrono::steady_clock::now();

    std::map<std::string, std::string> metadata;
    for (const auto &kv : context->client_metadata()) {
        metadata.emplace(
                std::string(kv.first.data(), kv.first.size()),
                std::string(kv.second.data(), kv.second.size())
        );
    }
    TRACE_POINT_WITH_CONTEXT(metadata);

    Status status = Status::OK;
    const auto duration_millis = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - started_at
    );
    const std::array<
            std::pair<opentelemetry::nostd::string_view, opentelemetry::common::AttributeValue>,
            4>
            attributes = { {
                    { "rpc.system", "grpc" },
                    { "rpc.service", "ServerMessagesService" },
                    { "rpc.method", "CheckOnline" },
                    { "rpc.grpc.status_code", static_cast<std::int64_t>(status.error_code()) },
            } };
    const opentelemetry::common::KeyValueIterableView attributes_view{ attributes };
    const auto &metric_attributes =
            static_cast<const opentelemetry::common::KeyValueIterable &>(attributes_view);

    request_counter()->Add(1, metric_attributes);
    request_duration_histogram()->Record(
            duration_millis.count(), metric_attributes, opentelemetry::context::Context{}
    );

    return status;
}

} // namespace ServerMessages