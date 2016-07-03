
set LIB=C:\curl-7.21.6-mingw\lib\;C:\curl-7.21.6-mingw;C:\windows;C:\windows\system32;C:\MinGW\lib
set LIBPATH=C:\curl-7.21.6-mingw;C:\MinGW\lib;C:\windows;C:\windows\system32;
set INCLUDE=C:\curl-7.21.6-mingw\include;C:\MinGW\include
set CURL_INCLUDE_DIR=C:\curl-7.21.6-mingw\include
set QTDIR=C:\Qt\4.7.3

set BOOSTROOT=C:\dev\boost_1_45_0-mingw
set BOOST_ROOT=%BOOSTROOT%
set Path=C:\MinGW\bin;C:\work\Python32;C:\Windows\system32;C:\Windows;C:\Program Files\doxygen\bin;C:\Program Files\CMake 2.8\bin;%BOOST_ROOT%\lib;C:\tmp\depends22_x86\;%QTDIR%\bin
set Path=%Path%;C:\MinGW\bin;C:\Program Files (x86)\eclipse;C:\ar\ffmpeg\devdevdev\bin

C:
cd C:\tmp\
rmdir win32-MinGW-marker-qtdynamic-eclipse-release /S /Q
mkdir win32-MinGW-marker-qtdynamic-eclipse-release
cd win32-MinGW-marker-qtdynamic-eclipse-release

cmake C:\ar\forge\marker\ -G "Eclipse CDT4 - MinGW Makefiles" -DDISABLE_CONSOLE=1 -DSUPPORT_MP3=1
set MAKE_COMMAND=mingw32-make -j3
mingw32-make -j5
cmd.exe