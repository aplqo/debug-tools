#!/bin/bash

# 1: file name 2:time 3: whether create .cpp file

if [ "$3" = "true" ]
then
    touch "$1.cpp"
fi
for ((i=0;$i<$2;i++))
do
    touch "$1$i.in"
    touch "$1$i.ans"
    $edit "$1$i.in"
    $edit "$1$i.ans"
done
