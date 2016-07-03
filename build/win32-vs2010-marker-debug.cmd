
call win32-vs2010-variables.cmd

set QTDIR=C:\Qt\4.8.0-win32-vs2010-MT
set BOOSTROOT=C:\boost_1_45_0-vs2005

C:
cd C:\tmp\
rmdir win32-vs2010-marker-debug /S /Q
mkdir win32-vs2010-marker-debug
cd win32-vs2010-marker-debug

cmake C:\ar\forge\marker\ -G "Visual Studio 10" -DCMAKE_BUILD_TYPE=Debug
marker.sln
cd Release
cmd.exe