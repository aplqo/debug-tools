#!/bin/bash

readonly typ=$(cat ./.config/vscode)
if [ -e ".dtors/$typ-deinit.sh" ]
then
    ./.dtors/$typ-deinit.sh
    rm -rf ./.dtors/$typ-deinit.sh
fi
rm -rf .vscode