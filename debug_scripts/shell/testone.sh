#!/bin/bash

./bin/single-$type -program $1 -in $2.in -ans $2.ans -test 'diff' -test-args [ '{output}' '{answer}' -a -w -u --color ] -autodiff [ -diff '{input}.diff' -files [ '{output}' '{answer}' ] ]
