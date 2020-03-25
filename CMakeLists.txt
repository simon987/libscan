cmake_minimum_required(VERSION 3.15)
project(scan C)

set(CMAKE_C_STANDARD 11)


add_library(
        scan
        src/text.c src/text.h
        src/util.c src/util.h
        src/types.h
        src/macros.h

        third-party/utf8.h
)


# Third party
add_subdirectory(third-party/uuid/)
target_include_directories(
        scan PRIVATE
        third-party/uuid/src/
)

target_compile_options(
        scan PRIVATE
        -Werror
)

target_link_libraries(
        scan
        uuid
)