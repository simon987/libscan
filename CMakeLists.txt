cmake_minimum_required(VERSION 3.15)

project(scan)
set(CMAKE_C_STANDARD 11)

add_library(
        scan
        libscan/util.c libscan/util.h
        libscan/scan.h
        libscan/macros.h

        libscan/text/text.c libscan/text/text.h
        libscan/arc/arc.c libscan/arc/arc.h
        libscan/ebook/ebook.c libscan/ebook/ebook.h
        libscan/cbr/cbr.c libscan/cbr/cbr.h
        libscan/ooxml/ooxml.c libscan/ooxml/ooxml.h
        libscan/media/media.c libscan/media/media.h
        libscan/font/font.c libscan/font/font.h

        third-party/utf8.h
        libscan/mobi/scan_mobi.c libscan/mobi/scan_mobi.h)
set_target_properties(scan PROPERTIES LINKER_LANGUAGE C)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
target_link_directories(scan PUBLIC /usr/share/vcpkg/installed/x64-linux/lib/)

find_package(LibArchive REQUIRED)
find_package(BZip2 REQUIRED)
find_package(lz4 REQUIRED)

find_package(Threads REQUIRED)
find_package(Tesseract CONFIG REQUIRED)
find_library(HARFBUZZ_LIBRARIES harfbuzz)
find_package(OpenJPEG CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(FFMPEG REQUIRED)
#find_package(OpenSSL REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(ZLIB REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_library(JBIG2DEC_LIB NAMES jbig2decd jbig2dec)


target_compile_options(
        scan
        PRIVATE
        -Werror
        -g
)

include(ExternalProject)
find_program(MAKE_EXE NAMES gmake nmake make)
ExternalProject_Add(
        libmobi
        GIT_REPOSITORY https://github.com/bfabiszewski/libmobi.git
        GIT_TAG "public"

        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CONFIGURE_COMMAND ./autogen.sh && ./configure
        INSTALL_COMMAND ""

        PREFIX "third-party/ext_libmobi"
        SOURCE_DIR "third-party/ext_libmobi/src/libmobi"
        BINARY_DIR "third-party/ext_libmobi/src/libmobi"

        BUILD_COMMAND make -j 3 --silent
)

SET(MOBI_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/third-party/ext_libmobi/src/libmobi/src/.libs/)
SET(MOBI_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/third-party/ext_libmobi/src/libmobi/src/)

string(REGEX REPLACE "-lvdpau" "" FFMPEG_LIBRARIES "${FFMPEG_LIBRARIES}")
string(REGEX REPLACE "-lX11" "" FFMPEG_LIBRARIES "${FFMPEG_LIBRARIES}")

target_link_libraries(
        scan
        PUBLIC

        ${LibArchive_LIBRARIES}
        ZLIB::ZLIB
        BZip2::BZip2
        lz4::lz4
        zstd
        lzo2
        LibLZMA::LibLZMA

        ${HARFBUZZ_LIBRARIES}
        libmupdf

        ${MOBI_LIB_DIR}/libmobi.a

        freetype
        ${HARFBUZZ_LIBRARIES}
        ${JBIG2DEC_LIB}
        #        OpenSSL::SSL OpenSSL::Crypto

        stdc++

        -Wl,--whole-archive
        m
        -Wl,--no-whole-archive

        ${JPEG_LIBRARIES}
        ${Tesseract_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        z

        # TODO: compile ffmpeg with those disabled?
        va
        va-drm
        va-x11
        X11
        vdpau


        ${CMAKE_THREAD_LIBS_INIT}

        uuid
)

target_include_directories(
        scan
        PUBLIC
        ${MUPDF_INC_DIR}
        ${JPEG_INCLUDE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        ${FFMPEG_INCLUDE_DIR}
        ${MOBI_INCLUDE_DIR}
)

# Testing
find_package(GTest CONFIG REQUIRED)

add_executable(scan_ub_test test/main.cpp test/test_util.cpp test/test_util.h)
target_compile_options(scan_ub_test PRIVATE -g -fsanitize=undefined -fno-omit-frame-pointer)
target_link_libraries(scan_ub_test PRIVATE GTest::gtest GTest::gtest_main -fsanitize=undefined scan)

add_executable(scan_a_test test/main.cpp test/test_util.cpp test/test_util.h)
target_compile_options(scan_a_test PRIVATE -g -fsanitize=address -fno-omit-frame-pointer)
target_link_libraries(scan_a_test PRIVATE GTest::gtest GTest::gtest_main -fsanitize=address scan)
