#!/bin/sh

set -xe

FILES="./src/main.c ./src/ray_helpers.c"

gcc $FILES -lraylib -o main -Wall -Wextra
