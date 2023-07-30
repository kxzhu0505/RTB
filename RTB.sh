#!/bin/bash

cd /build
./main ../benchmark/diffeq.blif

cd /LBTB/build/main
./main ../../rslt/run001diffeq.blif.out 1 100

