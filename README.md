*(wip)*

```bash
vcpkg install libarchive[core,bzip2,libxml2,lz4,lzma,lzo] pthread tesseract libxml2 ffmpeg zstd gtest

cmake -DCMAKE_TOOLCHAIN_FILE=/usr/share/vcpkg/scripts/buildsystems/vcpkg.cmake .
make -j 4
```