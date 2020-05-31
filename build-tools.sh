#!/bin/bash

if [ -z "$1" -o -z "$2" ]
then
	echo "Make select version"
	exit 1
fi
readonly build='/tmp/debug-tols_build'
readonly src=$(pwd)
if [ ! -e "$build"  ]
then
	mkdir -p "$build"
fi

cmake -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX="$2" -Dcolor=on -Dver="$1" -S "${src}" -B "$build"
cd "$build"
make
make install
