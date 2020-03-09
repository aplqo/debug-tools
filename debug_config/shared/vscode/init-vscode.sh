#!/bin/bash

readonly prefix="./config/vscode"
mkdir "$1/.vscode"
echo "$2" > "$1/.config/vscode"
if [ -e "$perifx/$2/$2-init.sh" ]
then
    $perfix/$2/$2-init.sh "$1"
else
    cp "$prefix/$2/*" "$1/.vscode/"
fi