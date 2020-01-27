#!/bin/bash

./bin/random $1 -tmpdir tmp -generator $2 -genargs $3 -times $4 -stop-on-error -test ".\scripts\test.cmd" -testargs 3 [input] [output] [answer]