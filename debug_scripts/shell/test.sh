#!/bin/bash

bin/diff -no-version -files 2 "$2" "$3" -diff "$1.diff" -test diff -testargs 6 "$2" "$3" -a -w --color -u
exit $?
