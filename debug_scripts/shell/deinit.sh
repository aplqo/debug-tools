#!/bin/bash

cd ../
bash -c ./scripts/clean.sh
rm .clang-format
rm -rf bin
rm -rf .vscode
rm -rf debug_tools
rm -rf .config
rm -rf scripts