#!/bin/bash

./bin/random $1 -tmpdir "$5" -generator "$2" -genargs $3 -times $4 -stop-on-error -test "./bin/diff" -testargs 17 -quiet -limit 0 -files 1 [input] -diff [differ] -test diff -testargs 5 [output] [answer] -a -w -u
