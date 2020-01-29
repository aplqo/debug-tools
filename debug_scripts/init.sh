#!/bin/bash

if [ ! -e $1 ]
then
	mkdir $1
fi

mkdir $1/scripts

cp -r ./bin $1/bin
cp ./scripts/*.sh $1/scripts/
cp -r ./debug_tools $1/debug_tools
cp ./config/.clang-format $1