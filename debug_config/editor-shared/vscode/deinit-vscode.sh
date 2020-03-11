#!/bin/bash

function default()
{
    rm -rf .vscode
}

. ./.dtors/compiler-deinit.sh

deinit