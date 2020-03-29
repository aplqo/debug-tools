#!/bin/bash

$typ=$(cat "./.config/editor")
command "./.config/deinit-$typ.sh"
rm -f "./.dtors/deinit-$typ.ps1"