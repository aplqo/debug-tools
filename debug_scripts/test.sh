#!/bin/bash

$1 -in $2.in -out $2.out $3 -test diff -testargs 4 $2.out $2.ans -a -w
if [ $? -eq 0 ]
then
    rm $2.out
fi