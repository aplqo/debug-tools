#!/bin/bash

readonly test="-test diff -testargs 6 [output] [answer] -a -w --color -u"
readonly limfiles="-files 2 [output] [answer]"
readonly diff="-diff [input].diff"

./bin/random $1 -tmpdir tmp -generator $2 -genargs $3 -times $4 -stop-on-error -test ".\bin\diff" -testargs 16 $limfiles $diff $test