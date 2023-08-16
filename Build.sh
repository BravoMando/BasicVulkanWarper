#if [ ! -d ./build ]; then
#    mkdir build
#fi

mkdir -p build

cd build
cmake ..
cmake --build .

./App
