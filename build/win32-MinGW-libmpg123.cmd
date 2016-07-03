
set LIB=C:\dev\curl-7.26.0-mingw;C:\windows;C:\windows\system32;C:\MinGW\lib
set LIBPATH=C:\dev\curl-7.26.0-mingw;C:\MinGW\lib;C:\windows;C:\windows\system32;
set INCLUDE=C:\dev\curl-7.26.0-mingw\include;C:\MinGW\include
set QTDIR=C:\Qt\4.8.0-win32-mingw-static

set BOOSTROOT=C:\dev\boost_1_45_0-mingw
set BOOST_ROOT=%BOOSTROOT%
set Path=C:\MinGW\bin;C:\work\Python32;C:\Windows\system32;C:\Windows;C:\Program Files\doxygen\bin;C:\Program Files\CMake 2.8\bin;%BOOST_ROOT%\lib;C:\tmp\depends22_x86\;%QTDIR%\bin
set Path=%Path%;C:\cygwin\bin;C:\MinGW\bin;C:\Program Files (x86)\eclipse;C:\ar\ffmpeg\devdevdev\bin;C:\Perl\bin\;


C:
cd C:\dev\build\mpg123\mpg123-1.14.3

@rem SHARED
@rem bash configure --enable-buffer=no --enable-shared=yes --enable-static=no --enable-network=no

@rem STATIC
bash configure --enable-buffer=no --enable-shared=no --enable-static=yes --enable-network=no

cd src\libmpg123
sed 's/MAKE=make/MAKE=mingw32-make/g' Makefile
mingw32-make clean
mingw32-make all

cmd.exe