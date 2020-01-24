#!/bin/bash

./bin/single.exe $1 -in $2.in -out $2.out $3 -test diff -testargs 6 $2.out $2.ans -a -w --color -u