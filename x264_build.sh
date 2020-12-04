#!/bin/bash
path = "android-x264"
mkdir $path
./configure --prefix=`pwd`/$path --enable-shared --enable-static  --disable-opencl --disable-asm
make
make install