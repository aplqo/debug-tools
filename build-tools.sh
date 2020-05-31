#!/bin/bash

if [ -z "$1" -o -z "$2" ]
then
	echo "Make select version"
	exit 1
fi

readonly src=$(pwd)
cd /tmp
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX="$2" -Dcolor=on -Dver="$1" -S "${src}/debug-tools" -B .
make
make install
