cmake_minimum_required(VERSION 3.15)

project(scan)
set(CMAKE_C_STANDARD 11)

option(BUILD_TESTS "Build tests" off)

add_library(
        scan
        libscan/util.c libscan/util.h
        libscan/scan.h
        libscan/macros.h

        libscan/text/text.c libscan/text/text.h
        libscan/arc/arc.c libscan/arc/arc.h
        libscan/ebook/ebook.c libscan/ebook/ebook.h
        libscan/comic/comic.c libscan/comic/comic.h
        libscan/ooxml/ooxml.c libscan/ooxml/ooxml.h
        libscan/media/media.c libscan/media/media.h
        libscan/font/font.c libscan/font/font.h

        third-party/utf8.h
        libscan/mobi/scan_mobi.c libscan/mobi/scan_mobi.h libscan/raw/raw.c libscan/raw/raw.h)
set_target_properties(scan PROPERTIES LINKER_LANGUAGE C)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib .so)
target_link_directories(scan PUBLIC /usr/share/vcpkg/installed/x64-linux/lib/)

find_package(LibArchive REQUIRED)
find_package(BZip2 REQUIRED)
find_package(lz4 REQUIRED)

find_package(Threads REQUIRED)
find_package(Tesseract CONFIG REQUIRED)
find_package(OpenJPEG CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(FFMPEG REQUIRED)
#find_package(OpenSSL REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(ZLIB REQUIRED)

find_library(JBIG2DEC_LIB NAMES jbig2decd jbig2dec)
find_library(HARFBUZZ_LIB NAMES harfbuzz harfbuzzd)
find_library(FREETYPE_LIB NAMES freetype freetyped)
find_package(unofficial-brotli CONFIG REQUIRED)
find_library(LZO2_LIB NAMES lzo2)

find_library(CMS_LIB NAMES lcms)
find_library(JAS_LIB NAMES jasper)
find_library(GOMP_LIB NAMES libgomp.a gomp PATHS /usr/lib/gcc/x86_64-linux-gnu/5/ /usr/lib/gcc/x86_64-linux-gnu/10/)


target_compile_options(
        scan
        PRIVATE
        -g
)

include(ExternalProject)
find_program(MAKE_EXE NAMES gmake nmake make)
ExternalProject_Add(
        libmobi
        GIT_REPOSITORY https://github.com/simon987/libmobi.git
        GIT_TAG "public"

        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CONFIGURE_COMMAND ./autogen.sh && ./configure
        INSTALL_COMMAND ""

        PREFIX "third-party/ext_libmobi"
        SOURCE_DIR "third-party/ext_libmobi/src/libmobi"
        BINARY_DIR "third-party/ext_libmobi/src/libmobi"

        BUILD_COMMAND ${MAKE_EXE} -j 8 --silent
)

SET(MOBI_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/third-party/ext_libmobi/src/libmobi/src/.libs/)
SET(MOBI_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/third-party/ext_libmobi/src/libmobi/src/)

ExternalProject_Add(
        ffmpeg
        GIT_REPOSITORY https://git.ffmpeg.org/ffmpeg.git
        GIT_TAG "master"

        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CONFIGURE_COMMAND ./configure --disable-shared --enable-static --disable-ffmpeg --disable-ffplay
        --disable-ffprobe --disable-doc --disable-manpages --disable-postproc --disable-avfilter --disable-alsa
        --disable-lzma --disable-xlib --disable-debug --disable-vdpau --disable-vaapi --disable-sdl2
        --disable-network --extra-cflags=-fPIC
        INSTALL_COMMAND ""

        PREFIX "third-party/ext_ffmpeg"
        SOURCE_DIR "third-party/ext_ffmpeg/src/ffmpeg"
        BINARY_DIR "third-party/ext_ffmpeg/src/ffmpeg"

        BUILD_COMMAND ${MAKE_EXE} -j 8 --silent
)

SET(FFMPEG_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/third-party/ext_ffmpeg/src/ffmpeg)

add_dependencies(
        scan
        libmobi
        ffmpeg
)

target_link_libraries(
        scan
        PUBLIC

        ${LibArchive_LIBRARIES}
        ZLIB::ZLIB
        BZip2::BZip2
        lz4::lz4
        ${LZO2_LIB}
        LibLZMA::LibLZMA

        libmupdf
        openjp2

        ${MOBI_LIB_DIR}/libmobi.a

        ${FREETYPE_LIB}
        ${HARFBUZZ_LIB}
        ${JBIG2DEC_LIB}
        #        OpenSSL::SSL OpenSSL::Crypto

        stdc++

        -Wl,--whole-archive
        m
        -Wl,--no-whole-archive

        ${JPEG_LIBRARIES}
        ${Tesseract_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        ${FREETYPE_LIB}
        unofficial::brotli::brotlidec-static

        ${FFMPEG_LIB_DIR}/libavformat/libavformat.a
        ${FFMPEG_LIB_DIR}/libavcodec/libavcodec.a
        ${FFMPEG_LIB_DIR}/libavutil/libavutil.a
        ${FFMPEG_LIB_DIR}/libswresample/libswresample.a
        ${FFMPEG_LIB_DIR}/libswscale/libswscale.a

        z

        ${CMAKE_THREAD_LIBS_INIT}

        raw
        ${GOMP_LIB}
        ${CMS_LIB}
        ${JAS_LIB}
        dl
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

if (BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)

    add_executable(scan_ub_test test/main.cpp test/test_util.cpp test/test_util.h)
    target_compile_options(scan_ub_test PRIVATE -g -fsanitize=undefined -fno-omit-frame-pointer)
    target_link_libraries(scan_ub_test PRIVATE GTest::gtest GTest::gtest_main -fsanitize=undefined scan)

    add_executable(scan_a_test test/main.cpp test/test_util.cpp test/test_util.h)
    target_compile_options(scan_a_test PRIVATE -g -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(scan_a_test PRIVATE GTest::gtest GTest::gtest_main -fsanitize=address scan)
endif()
