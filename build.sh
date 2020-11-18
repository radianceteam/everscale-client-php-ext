#!/bin/sh

mkdir -p build/deps/lib
cp -r src/* build
cp deps/include/tonclient.h build
cp -r deps/lib/x64 build/deps/lib/

cd build
phpize
./configure
make
sudo make install
