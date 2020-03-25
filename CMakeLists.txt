cmake_minimum_required(VERSION 3.15)

project(scan C)
set(CMAKE_C_STANDARD 11)

find_package(LibArchive REQUIRED)

add_library(
        scan
        libscan/util.c libscan/util.h
        libscan/scan.c libscan/scan.h
        libscan/macros.h

        libscan/text/text.c libscan/text/text.h
        libscan/arc/arc.c libscan/arc/arc.h

        third-party/utf8.h
)

target_compile_options(
        scan
        PRIVATE
        -Werror
        -g
)

target_link_libraries(
        scan
        -static
        ${LibArchive_LIBRARIES}
)


# test executable
add_executable(
        scan_test
        test/main.c
)


target_link_libraries(
        scan_test
        scan
)

