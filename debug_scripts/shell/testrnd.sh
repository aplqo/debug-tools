#!/bin/bash

readonly program=''
readonly gen=''
readonly std=''

./bin/random-$type -program $program -tmpdir "$1" -generator $gen -gen-args [ '{input}' ] -standard $std -std-args [ '{input}' '{answer}' ] -times $4 -stop-on-error -test diff -test-args [ "{output}" "{answer}" -a -w -u ]
