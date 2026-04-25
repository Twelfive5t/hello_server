# clang-format：手动目标，统一格式化 services/ 和 common/ 下所有源文件
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(format ALL
        COMMAND find
            ${CMAKE_SOURCE_DIR}/services
            ${CMAKE_SOURCE_DIR}/common
            -type f \( -name "*.cpp" -o -name "*.hpp" \)
            | xargs -r ${CLANG_FORMAT_EXE} -i --style=file
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "clang-format: formatting all project sources"
        VERBATIM
    )
    message(STATUS "clang-format enabled: ${CLANG_FORMAT_EXE}")
else()
    message(WARNING "clang-format not found, formatting disabled")
endif()
