cmake_minimum_required(VERSION 3.15)

project(scan C)
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
)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
target_link_directories(scan PRIVATE BEFORE /usr/share/vcpkg/installed/x64-linux/lib/)

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

SET(CMAKE_C_LINK_EXECUTABLE "g++ <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

string(REGEX REPLACE "-lvdpau" "" FFMPEG_LIBRARIES "${FFMPEG_LIBRARIES}")
string(REGEX REPLACE "-lX11" "" FFMPEG_LIBRARIES "${FFMPEG_LIBRARIES}")

target_link_libraries(
        scan

        ${LibArchive_LIBRARIES}
        ZLIB::ZLIB
        BZip2::BZip2
        lz4::lz4
        zstd
        lzo2
        LibLZMA::LibLZMA

        ${HARFBUZZ_LIBRARIES}
        libmupdf

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

        ${CMAKE_THREAD_LIBS_INIT}

        uuid
)

target_include_directories(
        scan
        BEFORE
        PUBLIC
        ${MUPDF_INC_DIR}
        ${JPEG_INCLUDE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        ${FFMPEG_INCLUDE_DIR}
)
