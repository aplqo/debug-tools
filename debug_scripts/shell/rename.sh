#!/bin/bash

find "$1" -name '*.out' | xargs -i bash -c 'mv "{}" "$(dirname {})/$(basename -s .out {}).ans" -v'
