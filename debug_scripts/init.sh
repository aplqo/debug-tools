#!/bin/bash

if [ ! -e "$1" ]
then
	mkdir "$1"
fi

mkdir "$1/scripts"
mkdir "$1/.config"

cp -r ./bin "$1/bin"
cp ./scripts/*.sh "$1/scripts/"
cp -r ./debug_tools "$1/debug_tools"
cp ./config/.clang-format "$1"
echo `pwd` > "$1/.config/src"