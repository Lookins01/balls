#!/bin/sh

set -xe

mkdir -p ./build/

CFLAGS="-O3 -Wall -Wextra -g -pedantic `pkg-config --libs raylib`"
CLIBS="`pkg-config --libs raylib` -lm"

gcc $CFLAGS -o ./build/balls ./src/balls.c $CLIBS
