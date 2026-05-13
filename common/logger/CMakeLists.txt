project(LoggerLib)

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(LoggerLib STATIC logger.cpp)

target_include_directories(LoggerLib PUBLIC ${CMAKE_SOURCE_DIR}/common)

target_link_libraries(LoggerLib PUBLIC spdlog::spdlog nlohmann_json::nlohmann_json)