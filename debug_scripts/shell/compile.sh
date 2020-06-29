#!/bin/bash

base=$(basename -s .cpp "$1")
fdir=$(dirname "$1")
readonly args="-std=c++11 -Wall -Wextra -Weffc++ -Wdisabled-optimization"

if [ "$2" = 'test' ]
then
	$CXX -DAPTEST -lpthread -I. $args "$fdir/$base.cpp" -o "$fdir/$base"
else
	$CXX -DAPDEBUG $args "$fdir/$base.cpp" -g  -o "$fdir/$base"
fi
