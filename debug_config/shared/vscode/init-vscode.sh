#!/bin/bash

readonly prefix="./config/vscode"
mkdir "$1/.vscode"
echo "$2" > "$1/.config/vscode"
if [ -e "$perifx/$2/init-$2.sh" ]
then
    $perfix/$2/init-$2.sh "$1"
else
    cp "$prefix/$2/*" "$1/.vscode/"
fi