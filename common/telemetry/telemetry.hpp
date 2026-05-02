#pragma once

#include <memory>
#include <string>
#include <vector>

#define FILE_LINE (__FILE__ + std::string(":") + std::to_string(__LINE__))
#define FILE_LINE_FUNC (__FILE__ + std::string(":") + std::to_string(__LINE__) + ", " + std::string(__FUNCTION__))

#define TRACE_POINT Trace trace_point_instance(FILE_LINE_FUNC);
#define TRACE_POINT_ADD trace_point_instance.addEvent(FILE_LINE_FUNC);
#define TRACE_POINT_ADD_MSG(msg) trace_point_instance.addEvent(msg);

struct TelemetryConfig {
    std::string service_name = "acrn_rtos";
    std::string service_instance_id = ""; // Empty defaults to auto-generated
    std::string endpoint = "localhost:4317";
    std::string version = "0.0.1";
    std::string environment = "test";
    std::vector<std::string> ignored_spans = {};
};

// 初始化 Tracer，支持传入配置
void InitTracer(const TelemetryConfig& config = TelemetryConfig{});
void CleanupTracer();

class Trace
{ // NOLINT
  public:
    explicit Trace(const std::string &str = FILE_LINE);
    ~Trace();

    // 禁用拷贝，防止 Span 意外共享导致生命周期混乱
    Trace(const Trace &) = delete;
    auto operator=(const Trace &) -> Trace & = delete;

    // 允许移动，支持所有权转移
    Trace(Trace &&) noexcept;
    auto operator=(Trace &&) noexcept -> Trace &;

    auto before(const std::string &str) -> void;
    auto addEvent(const std::string &name) -> void;
    auto after() -> void;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_; // 使用 unique_ptr 管理 Pimpl，更高效且语义明确
};
