#!/bin/bash

echo "Running Record Sound test..."

arecord -vv -d 30 -fdat foo.wav

aplay -vv  foo.wav

rm foo.wav
