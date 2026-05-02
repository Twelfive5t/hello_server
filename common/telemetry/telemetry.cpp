#include "telemetry.hpp"
#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h"
#include "opentelemetry/sdk/resource/semantic_conventions.h"
#include "opentelemetry/sdk/trace/batch_span_processor.h"
#include "opentelemetry/sdk/trace/batch_span_processor_factory.h"
#include "opentelemetry/sdk/trace/batch_span_processor_options.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_context.h"
#include "opentelemetry/sdk/trace/tracer_context_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/sdk/trace/sampler.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/provider.h"
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/batch_span_processor_options.h>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace trace = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace otlp = opentelemetry::exporter::otlp;
namespace nostd = opentelemetry::nostd;
namespace resource = opentelemetry::sdk::resource;

// 自定义采样器：用于过滤掉不需要的 Span (例如高频但无关紧要的函数)
class IgnoreSampler : public opentelemetry::sdk::trace::Sampler
{
  public:
    explicit IgnoreSampler(std::vector<std::string> ignored_names) : ignored_names_(std::move(ignored_names))
    {
    }

    // 采样决策逻辑：在 Span 创建前调用
    opentelemetry::sdk::trace::SamplingResult ShouldSample(
        const opentelemetry::trace::SpanContext & /*parent_context*/,
        opentelemetry::trace::TraceId /*trace_id*/,
        opentelemetry::nostd::string_view name,
        opentelemetry::trace::SpanKind /*span_kind*/,
        const opentelemetry::common::KeyValueIterable & /*attributes*/,
        const opentelemetry::trace::SpanContextKeyValueIterable & /*links*/) noexcept override
    {
        std::string name_str(name.data(), name.size());
        // 遍历过滤列表，如果 Span 名称包含任意一个关键词，则丢弃
        for (const auto &ignored : ignored_names_)
        {
            if (name_str.find(ignored) != std::string::npos)
            {
                // Decision::DROP 表示完全丢弃该 Span，不记录也不导出
                return {opentelemetry::sdk::trace::Decision::DROP, {}, {}};
            }
        }
        // 默认行为：记录并采样 (RECORD_AND_SAMPLE)
        return {opentelemetry::sdk::trace::Decision::RECORD_AND_SAMPLE, {}, {}};
    }

    opentelemetry::nostd::string_view GetDescription() const noexcept override
    {
        return "IgnoreSampler";
    }

  private:
    std::vector<std::string> ignored_names_;
};


void InitTracer(const TelemetryConfig& config)
{
    // 1. 创建 Exporter: 负责将 Trace 数据发送到后端 (如 Jaeger, Zipkin, OTel Collector)
    // 这里使用 OTLP gRPC Exporter，它是 OpenTelemetry 的标准协议
    opentelemetry::exporter::otlp::OtlpGrpcExporterOptions opts;
    opts.endpoint = config.endpoint; 

    auto exporter = otlp::OtlpGrpcExporterFactory::Create(opts);

    // 2. 创建 Processor: 负责处理 Span (如批量发送，减少网络开销)
    // BatchSpanProcessor 会在后台缓冲 Span，并批量发送给 Exporter
    trace_sdk::BatchSpanProcessorOptions options{};
    auto processor = trace_sdk::BatchSpanProcessorFactory::Create(std::move(exporter), options);

    std::vector<std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>> processors;
    processors.push_back(std::move(processor));

    // 3. 创建 Resource: 描述产生 Trace 的实体 (如服务名、版本、环境)
    // 这些信息会附加到所有的 Span 上，方便在 UI 上筛选和定位
    std::string service_instance_id = config.service_instance_id;
    if (service_instance_id.empty()) {
        // 如果未指定实例ID，则自动生成：ServiceName + 时间戳
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "_%Y%m%d_%H%M%S");
        service_instance_id = config.service_name + oss.str();
    }

    resource::ResourceAttributes attributes = {{resource::SemanticConventions::kServiceName, config.service_name},
                                               {resource::SemanticConventions::kServiceInstanceId, service_instance_id},
                                               {resource::SemanticConventions::kServiceVersion, config.version},
                                               {resource::SemanticConventions::kDeploymentEnvironment, config.environment},
                                               {resource::SemanticConventions::kHostName, "test"}}; // 实际项目中可获取真实 Hostname
    auto resource = opentelemetry::sdk::resource::Resource::Create(attributes);

    // 4. 创建 TracerProvider: 管理 Tracer 的生命周期和配置
    auto sampler = std::make_unique<IgnoreSampler>(config.ignored_spans);
    std::unique_ptr<opentelemetry::sdk::trace::TracerContext> context =
        opentelemetry::sdk::trace::TracerContextFactory::Create(std::move(processors), resource, std::move(sampler));
    std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
        trace_sdk::TracerProviderFactory::Create(std::move(context));

    // 5. 设置全局 TracerProvider: 让后续代码可以通过 Provider::GetTracerProvider() 获取
    trace::Provider::SetTracerProvider(provider);

    // 6. 设置全局 Propagator: 负责跨进程/跨服务传递 Trace Context (如 TraceId, SpanId)
    // HttpTraceContext 支持 W3C Trace Context 标准，用于在 HTTP Header 中传递上下文
    opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
        opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
            new opentelemetry::trace::propagation::HttpTraceContext()));
}

void CleanupTracer()
{
    std::shared_ptr<trace::TracerProvider> none;
    trace::Provider::SetTracerProvider(none);
}

auto get_tracer() -> nostd::shared_ptr<trace::Tracer>
{
    auto provider = trace::Provider::GetTracerProvider();
    return provider->GetTracer("acrn_rtos_lib", OPENTELEMETRY_SDK_VERSION);
}

class Trace::Impl
{ // NOLINT
  public:
    nostd::shared_ptr<opentelemetry::trace::Span> span_;
    trace::Scope outer_scope_;

    auto before(const std::string & /*str*/) -> void
    { // NOLINT
    }

    auto addEvent(const std::string &name) -> void
    { // NOLINT
        span_->AddEvent(name);
    }

    auto after() const -> void
    { // NOLINT
        span_->End();
    }

    explicit Impl(const std::string &str)
        // trace::Scope 是 RAII 对象且不可移动，必须在初始化列表中构造
        // StartSpan: 开始一个新的 Span
        // WithActiveSpan: 将该 Span 设为当前线程的活跃 Span，作用域结束时自动恢复上一个 Span
        : span_(get_tracer()->StartSpan(str)), outer_scope_(get_tracer()->WithActiveSpan(span_))
    {
        (void)before(str);
    }
    ~Impl()
    {
        after();
    }
};

auto Trace::before(const std::string &str) -> void
{ // NOLINT
    impl_->before(str);
}

auto Trace::addEvent(const std::string &name) -> void
{ // NOLINT
    impl_->addEvent(name);
}

auto Trace::after() -> void
{ // NOLINT
    impl_->after();
}

Trace::Trace(const std::string &str) : impl_(std::make_unique<Impl>(str))
{
}
Trace::~Trace() = default;

Trace::Trace(Trace &&) noexcept = default;
auto Trace::operator=(Trace &&) noexcept -> Trace & = default;
