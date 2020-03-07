#!/bin/bash

readonly prefix="./config/vscode"
mkdir "$1/.vscode"
if [ -e "$perifx/$2/$2-init.sh" ]
then
    $perfix/$2/$2-init.sh "$1"
else
    cp "$prefix/$2/*" "$1/.vscode/"
fi

echo "$2" > "$1/.config/vscode"

if [ -e "$prefix/$2/$2-deinit.sh" ]
then
    cp "$prefix/$2/$2-deinit.sh" "$1/.dtors/"
    rm -rf "$1/scripts/$2-deinit.sh"
fi