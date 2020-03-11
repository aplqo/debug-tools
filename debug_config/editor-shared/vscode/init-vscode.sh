#!/bin/bash

function default()
{
    mkdir "$1/.vscode"
    cp -r "$2/*" "$1/.vscode/"
}

. ./config/compiler-init.sh

init "$1" vscode "$2" "$3"