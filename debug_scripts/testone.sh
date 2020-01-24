#!/bin/bash

readonly test="-test 7 diff $2.out $2.ans -a -w --color -u"
readonly limfiles="-files 2 $2.out $2.ans"
readonly diff="-diff \"$1.diff\""

./bin/single $1 -in $2.in -out $2.out $3 -test '.\bin\diff' -testargs 15 $limfiles $diff $test 