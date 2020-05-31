#!/bin/bash

./bin/random $1 -tmpdir "$5" -generator "$2" -genargs $3 -times $4 -stop-on-error -test "./bin/diff" -testargs [ -quiet -limit 0 -files  "<input>" -diff "<differ>" -test diff -testargs [ "<output>" "<answer>" -a -w -u ] ]
