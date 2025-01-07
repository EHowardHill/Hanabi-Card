#!/bin/bash

qmake
make
./myapp -platform eglfs
