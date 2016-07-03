#!/bin/bash

SRC_PATH=${1}
rm -Rf /tmp/audigger_r
mkdir -p /tmp/audigger_r
cd /tmp/audigger_r
cmake ${SRC_PATH} -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DDISABLE_CONSOLE=1
make -j5
#cmake /mnt/forge/marker/ -G "Eclipse CDT4 - Unix Makefiles" -DSUPPORT_MP3=1 -DAUDIO_VIRT=1

