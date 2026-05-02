find_package(spdlog CONFIG REQUIRED)

add_executable(main
    src/hello_server.cpp
    src/routes/routes.cpp
)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_include_directories(main SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/services/proto/proto_cpp)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/services/server/src)
target_link_libraries(main PRIVATE spdlog::spdlog server_messages TelemetryLib)

install(TARGETS main DESTINATION bin${VERSION_RELEASE})