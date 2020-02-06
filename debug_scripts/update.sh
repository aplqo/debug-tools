#!/bin/bash

readonly di=$(cat .config/src)
readonly cur=$(pwd)

rm .clang-format
rm -rf bin
rm -rf .vscode
rm -rf debug_tools
rm -rf .config
rm -rf scripts

pushd "$di"
./init.sh "$cur"
popd