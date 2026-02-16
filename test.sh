#!/bin/bash

./build.sh

./build/game 2> >(tee -a test.log >&2)
