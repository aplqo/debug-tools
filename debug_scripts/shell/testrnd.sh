#!/bin/bash

readonly random='./bin/random-tra'

$random -program $1 -tmpdir "$5" -generator "$2" -gen-args $3 -times $4 -stop-on-error -test diff -test-args [ "{output}" "{answer}" -a -w -u ]
