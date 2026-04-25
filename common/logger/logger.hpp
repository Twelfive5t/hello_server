#ifndef SRC_UTILS_LOGGERS_HPP
#define SRC_UTILS_LOGGERS_HPP
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

inline void InitLogger() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

    constexpr auto kFileSize = 1048576;
    constexpr auto kMaxFiles = 30;
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/logs.txt", kFileSize, kMaxFiles);
    file_sink->set_level(spdlog::level::debug);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

    auto logger = std::make_shared<spdlog::logger>("main",
        spdlog::sinks_init_list{console_sink, file_sink});
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);

    spdlog::flush_on(spdlog::level::debug);
    spdlog::flush_every(std::chrono::seconds(1));

    spdlog::info("logger init success!");
}
#endif