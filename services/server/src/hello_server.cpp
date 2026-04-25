#include "logger/logger.hpp"

auto main(int /*argc*/, char * /*argv*/[]) -> int
{
    init_logger();
    spdlog::info("Hello, Server!");
    return 0;
}