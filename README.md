
```bash
vcpkg install libarchive pthread tesseract

rm -rf CMakeFiles/ CMakeCache.txt
cmake -DCMAKE_TOOLCHAIN_FILE=/usr/share/vcpkg/scripts/buildsystems/vcpkg.cmake .
make -j 4
```