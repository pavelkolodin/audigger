
set LIB=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\ATLMFC\LIB;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\lib;C:\Program Files\Microsoft Visual Studio 10.0\VC\PlatformSDK\lib;C:\Program Files\Microsoft Visual Studio 10.0\SDK\v2.0\lib;C:\curl-7.21.6-vs2005;C:\windows;C:\windows\system32;
set LIBPATH=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\ATLMFC\LIB;c:\Program Files (x86)\microsoft visual studio 10.0\vc\platformsdk\lib;C:\curl-7.21.6-vs2005
set INCLUDE=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include\;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\INCLUDE;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\PlatformSDK\include;C:\Program Files (x86)\Microsoft Visual Studio 10.0\SDK\v2.0\include;C:\Program Files (x86)\Intel\MKL\10.1.0.018\include;C:\curl-7.21.6\include

set QTDIR=C:\Qt\4.8.0-win32-mingw-static

set BOOSTROOT=C:\dev\boost_1_45_0
set BOOST_ROOT=%BOOSTROOT%
set Path=C:\Program Files\Oracle\VirtualBox;C:\work\Python32;C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Program Files\doxygen\bin;C:\Program Files\Vim\vim73;C:\Program Files\CMake 2.8\bin;%BOOST_ROOT%\lib;C:\tmp\depends22_x86\;%QTDIR%\bin;C:\Program Files\Far2;C:\Program Files\Ffmpeg For Audacity;C:\Program Files\Lame For Audacity
set Path=%Path%;C:\MinGW\bin;C:\Program Files (x86)\eclipse;C:\ar\ffmpeg\devdevdev\bin

C:
cd C:\tmp\
rmdir marker-win32-MinGW-eclipse /S /Q
mkdir marker-win32-MinGW-eclipse
cd marker-win32-MinGW-eclipse

@rem cmake C:\ar\forge\marker\ -G "Eclipse CDT4 - MinGW Makefiles" -DDISABLE_CONSOLE=1
cmake C:\ar\forge\marker\ -G "Eclipse CDT4 - MinGW Makefiles"
eclipse
cmd.exe