#!/bin/bash

./bin/single $1 -in $2.in -out $2.out $3 -test './scripts/test.sh' -testargs 3 $2.in $2.out $2.ans
