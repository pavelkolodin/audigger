#!/bin/bash

SRC_PATH=${1}
rm -Rf /tmp/audigger_rc
mkdir -p /tmp/audigger_rc
cd /tmp/audigger_rc
cmake ${SRC_PATH} -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make -j5
#cmake /mnt/forge/marker/ -G "Eclipse CDT4 - Unix Makefiles" -DSUPPORT_MP3=1 -DAUDIO_VIRT=1

