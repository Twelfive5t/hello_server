cmake_minimum_required(VERSION 3.22.1)

add_executable(main
    src/hello_server.c
)

install(TARGETS main DESTINATION ${CMAKE_INSTALL_PREFIX}/bin${VERSION_RELEASE})