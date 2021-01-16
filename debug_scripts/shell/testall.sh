#!/bin/bash

regex='^(.*)\.[^\.]*' # match without extension
inreg='.*\.in$'
ansreg='.*\.ans$'

if [ -n "$3" ]
then
    regex="$3"
fi
if [ -n "$4" ]
then
    inreg="$4"
fi
if [ -n "$5" ]
then
    ansreg="$5"
fi
./bin/group-$type -program $1 -test-regex [ "$regex" -em 1 ';' ] -indir "$2" -in-regex [ "$inreg" -mm  ';' ] -ansdir "$2" -ans-regex [ "$ansreg" -mm ';' ] -test diff -test-args [ '{output}' '{answer}' -a -w -u --color ] -autodiff [ -diff '{input}.diff' -files [ '{output}' '{answer}' ] ] ';'
