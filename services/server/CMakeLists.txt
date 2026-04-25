find_package(spdlog CONFIG REQUIRED)

add_executable(main
    src/hello_server.cpp
)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(main PRIVATE spdlog::spdlog)

install(TARGETS main DESTINATION bin${VERSION_RELEASE})