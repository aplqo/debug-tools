#!/bin/bash

./bin/single $1 -in $2.in -out $2.out -ans $2.ans $3 -test './scripts/test.sh' -testargs [ "<input>" "<output>" "<answer>" ]
