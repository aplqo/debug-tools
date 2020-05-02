#!/bin/bash

base=$(basename -s .cpp "$1")
fdir=$(dirname "$1")
readonly args="-std=c++11"

if [ "$2" = 'test' ]
then
	$CC -DAPTEST -lpthread -I. $args "$fdir/$base.cpp" -o "$fdir/$base"
else
	$CC -DAPDEBUG $args "$fdir/$base.cpp" -g  -o "$fdir/$base"
fi
