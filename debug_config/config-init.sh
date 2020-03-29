#!/bin/bash

function list-editor()
{
    for i in $(ls ./config/* -d && ls ./config//editor-shared/* -d)
    do
        if [ -d "$i" -a "$i" != "./config/compiler-shared" -a "$i" != "./config/editor-shared" ]
        then
            ls -d -l "$i"
        fi
    done
}
function find-editor() # $1: editor
{
    ret=""
    if [ -e "./config/$1" ]
    then
        ret="./config/$1"
        return 0
    elif [ -e "./config/editor-shared/$1" ]
    then
        ret="./config/editor-shared/$1"
        return 0
    fi
    echo "Can't find config for $1"
    return 1
}
function read-editor()
{
    local editor=""
    read -p "Enter editor" editor
    find-editor "$editor"
    unset ret
    while [ $? -gt 0 ]
    do
        read -p "Enter editor" editor
        find-editor "$editor"
    done
    unset ret
    ret="$editor"
}

editor="$2"
if [ -z "$2" ]
then
    list-editor
    read-editor
    editor=$ret
    unset ret
    echo "$editor" -n >> "$1/.config/cmd"
fi
echo "$editor" > "$1/.config/editor"
find-editor "$editor"
command "$ret/init-$editor.sh" "$1" "$3" "$4"
cp ./config/config-deinit.sh "$1/.dtors"