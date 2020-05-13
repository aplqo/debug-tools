#!/bin/bash

cd ../

./scripts/clean.sh
$(cat ./.config/src)/init deinit "$(pwd)" 