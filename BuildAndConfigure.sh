mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. -G "Ninja"
cmake --build .
cd ..
cd bin
