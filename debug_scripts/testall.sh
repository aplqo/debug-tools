#!/bin/bash

inreg='in$'
ansreg='ans$'
readonly test="-test diff -testargs 6 [output] [answer] -a -w --color -u"
readonly limfiles="-files 2 [output] [answer]"
readonly diff="-diff [input].diff"

if [ -n "$4" ]
then
    inreg="$4"
fi
if [ -n "$5" ]
then
    ansreg="$5"
fi
./bin/group $1 -test-regex "$3" -indir "$2" -in-regex "$inreg" -ansdir "$2" -ans-regex "$ansreg" -test '.\bin\diff' -testargs 16 $limfiles $diff $test