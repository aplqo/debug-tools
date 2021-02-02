#!/bin/bash

conf='./config/default.yaml'

if [ -n "$2" ]
then
    conf="$2"
fi

./bin/single-$type $conf "$1.in" "$1.ans" 
