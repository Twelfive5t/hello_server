#include "logger/logger.hpp"

auto main(int /*argc*/, char * /*argv*/[]) -> int {
    InitLogger();
    spdlog::info("Hello, Server!");
    return 0;
}