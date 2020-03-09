#!/bin/bash

readonly typ=$(cat ./.config/vscode)
if [ -e ".dtors/deinit-$typ.sh" ]
then
    ./.dtors/deinit-$typ.sh
    rm -rf ./.dtors/deinit-$typ.sh
fi
rm -rf .vscode