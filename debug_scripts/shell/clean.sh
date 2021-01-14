#!/bin/bash

if [ -n "$1" ]
then
    cd "$1"
fi

rm -fv *.ilk *.obj *.pdb *.exe
rm -fv *.out *.diff