#!/bin/bash

if [ ! -e "$1" ]
then
	mkdir "$1"
fi

mkdir "$1/scripts"
mkdir "$1/.config"
mkdir "$1/.dtors"

cp -r ./bin "$1/bin"
cp ./scripts/shell/*.sh "$1/scripts/"
cp -r ./debug_tools "$1/debug_tools"
cp ./config/.clang-format "$1"
echo `pwd` > "$1/.config/src"

./config/config-init.sh "$1" "$2" "$3" "$4"