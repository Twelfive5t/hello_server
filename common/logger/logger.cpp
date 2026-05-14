#include "logger/logger.hpp"

#include "nlohmann/json.hpp"
#include "spdlog/async.h"
#include "spdlog/formatter.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

namespace logger_detail
{

constexpr auto K_LOG_FILE_PREFIX = "spdlog_";
constexpr std::size_t K_BYTES_PER_MIB = 1024U * 1024U;
constexpr std::size_t K_MAX_FILE_SIZE_BYTES = 10U * K_BYTES_PER_MIB;
constexpr std::size_t K_MAX_FILES_PER_PROCESS = 5U;
constexpr std::size_t K_MAX_TOTAL_SIZE_BYTES = 512U * K_BYTES_PER_MIB;
constexpr std::size_t K_ASYNC_QUEUE_SIZE = 8192U;
constexpr std::size_t K_TIMESTAMP_BUFFER_SIZE = 32U;

struct log_file_entry {
    std::filesystem::path path;
    std::filesystem::file_time_type last_write_time;
    std::uintmax_t size;
};

class json_formatter final : public spdlog::formatter
{
public:
    void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override
    {
        const auto time_t_val = std::chrono::system_clock::to_time_t(msg.time);
        const auto ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(msg.time.time_since_epoch()) %
                1000;

        std::tm local_time{};
        localtime_r(&time_t_val, &local_time);

        std::array<char, K_TIMESTAMP_BUFFER_SIZE> ts_buf{};
        const auto ts_len =
                std::strftime(ts_buf.data(), ts_buf.size(), "%Y-%m-%d %H:%M:%S", &local_time);

        std::array<char, 5> ms_buf{}; // ".mmm\0"
        (void
        )std::snprintf(ms_buf.data(), ms_buf.size(), ".%03lld", static_cast<long long>(ms.count()));

        const nlohmann::json j = {
            { "timestamp", std::string(ts_buf.data(), ts_len) + ms_buf.data() },
            { "pid", static_cast<int>(::getpid()) },
            { "tid", msg.thread_id },
            { "level",
              [&] {
                  const auto sv = spdlog::level::to_string_view(msg.level);
                  return std::string{ sv.data(), sv.size() };
              }() },
            { "logger", std::string(msg.logger_name.data(), msg.logger_name.size()) },
            { "message", std::string(msg.payload.data(), msg.payload.size()) },
        };

        const auto json_str = j.dump() + "\n";
        dest.append(json_str.data(), json_str.data() + json_str.size());
    }

    [[nodiscard]] std::unique_ptr<spdlog::formatter> clone() const override
    {
        return std::make_unique<json_formatter>();
    }
};

auto is_managed_log_file(const std::filesystem::directory_entry &entry) -> bool
{
    if (!entry.is_regular_file()) {
        return false;
    }

    const auto filename = entry.path().filename().string();
    return filename.starts_with(K_LOG_FILE_PREFIX);
}

auto to_system_clock(std::filesystem::file_time_type timestamp
) -> std::chrono::system_clock::time_point
{
    return std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            timestamp - std::filesystem::file_time_type::clock::now() +
            std::chrono::system_clock::now()
    );
}

auto startup_timestamp() -> std::string
{
    const auto now = std::time(nullptr);
    std::tm local_time{};
    localtime_r(&now, &local_time);

    std::array<char, K_TIMESTAMP_BUFFER_SIZE> buffer{};
    const auto timestamp_length =
            std::strftime(buffer.data(), buffer.size(), "%Y%m%d_%H%M%S", &local_time);
    if (timestamp_length == 0U) {
        return { "unknown" };
    }

    return { buffer.data(), timestamp_length };
}

void prune_expired_logs(const std::filesystem::path &log_directory)
{
    const auto now = std::chrono::system_clock::now();
    const auto reserved_bytes =
            std::min(K_MAX_TOTAL_SIZE_BYTES, K_MAX_FILE_SIZE_BYTES * K_MAX_FILES_PER_PROCESS);
    const auto target_total_bytes = K_MAX_TOTAL_SIZE_BYTES - reserved_bytes;

    std::vector<log_file_entry> log_files;
    std::uintmax_t total_size = 0;

    for (const auto &entry : std::filesystem::directory_iterator(log_directory)) {
        if (!is_managed_log_file(entry)) {
            continue;
        }

        const auto file_timestamp = entry.last_write_time();
        if (now - to_system_clock(file_timestamp) >
            std::chrono::hours(24 * 30)) { // 30-day retention
            std::filesystem::remove(entry.path());
            continue;
        }

        const auto file_size = entry.file_size();
        total_size += file_size;
        log_files.push_back({ entry.path(), file_timestamp, file_size });
    }

    std::sort(
            log_files.begin(),
            log_files.end(),
            [](const log_file_entry &lhs, const log_file_entry &rhs) {
                return lhs.last_write_time < rhs.last_write_time;
            }
    );

    for (const auto &log_file : log_files) {
        if (total_size <= target_total_bytes) {
            break;
        }

        std::filesystem::remove(log_file.path);
        total_size -= log_file.size;
    }
}

auto build_log_path() -> std::filesystem::path
{
    const auto log_directory = std::filesystem::path("/workspace/logs");
    std::filesystem::create_directories(log_directory);
    prune_expired_logs(log_directory);
    return log_directory / (std::string(K_LOG_FILE_PREFIX) + startup_timestamp() + ".log");
}

} // namespace logger_detail

void init_logger()
{
    static const auto shutdown_registered = [] {
        return std::atexit([] {
                   spdlog::shutdown();
               }) == 0;
    }();
    (void)shutdown_registered;

    if (!spdlog::thread_pool()) {
        spdlog::init_thread_pool(
                logger_detail::K_ASYNC_QUEUE_SIZE, 1U // single background flush thread
        );
    }

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_formatter(std::make_unique<logger_detail::json_formatter>());

    const auto log_path = logger_detail::build_log_path();
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_path.string(),
            logger_detail::K_MAX_FILE_SIZE_BYTES,
            logger_detail::K_MAX_FILES_PER_PROCESS
    );
    file_sink->set_level(spdlog::level::debug);
    file_sink->set_formatter(std::make_unique<logger_detail::json_formatter>());

    auto logger = std::make_shared<spdlog::async_logger>(
            "main",
            spdlog::sinks_init_list{ console_sink, file_sink },
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::overrun_oldest
    );
    logger->set_level(spdlog::level::debug);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(2));

    spdlog::info(
            "logger init success: file={}, max_file_size={}MB, max_files_per_process={}, "
            "max_total_size={}MB, retention_days=30, async_queue={}",
            log_path.string(),
            logger_detail::K_MAX_FILE_SIZE_BYTES / logger_detail::K_BYTES_PER_MIB,
            logger_detail::K_MAX_FILES_PER_PROCESS,
            logger_detail::K_MAX_TOTAL_SIZE_BYTES / logger_detail::K_BYTES_PER_MIB,
            logger_detail::K_ASYNC_QUEUE_SIZE
    );
}
