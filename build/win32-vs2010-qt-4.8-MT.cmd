
set LIB=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\ATLMFC\LIB;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\lib;C:\Program Files\Microsoft Visual Studio 10.0\VC\PlatformSDK\lib;C:\Program Files\Microsoft Visual Studio 10.0\SDK\v2.0\lib;C:\curl-7.21.6-vs2005;C:\windows;C:\windows\system32;
set LIBPATH=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\ATLMFC\LIB;c:\Program Files (x86)\microsoft visual studio 10.0\vc\platformsdk\lib;C:\curl-7.21.6-vs2005
set INCLUDE=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include\;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\INCLUDE;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\PlatformSDK\include;C:\Program Files (x86)\Microsoft Visual Studio 10.0\SDK\v2.0\include;C:\Program Files (x86)\Intel\MKL\10.1.0.018\include;C:\curl-7.21.6\include

set QTDIR=C:\Qt\4.8.0-win32-vs2010-MT
@rem set BOOSTROOT=C:\boost_1_45_0-vs2005
set Path=
set Path=%PATH%;C:\Program Files\Oracle\VirtualBox
set	Path=%PATH%;C:\work\Python32
set Path=%PATH%;c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\BIN
set Path=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE
set Path=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin
set Path=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\VCPackages
set Path=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools
set Path=%PATH%;C:\Program Files (x86)/Microsoft SDKs/Windows/v7.0A/bin
set Path=%PATH%;C:\Windows\system32
set Path=%PATH%;C:\Windows
set Path=%PATH%;C:\Windows\System32\Wbem
set Path=%PATH%;C:\Program Files\doxygen\bin
set Path=%PATH%;C:\Program Files\Vim\vim73
set Path=%PATH%;C:\Program Files\CMake 2.8\bin
set Path=%PATH%;%BOOSTROOT%\lib
set Path=%PATH%;C:\tmp\depends22_x86\
set Path=%PATH%;%QTDIR%\bin
set Path=%PATH%;C:\Program Files\Far2
set Path=%PATH%;C:\Program Files\Ffmpeg For Audacity
set Path=%PATH%;C:\Program Files\Lame For Audacity
set Path=%PATH%;C:\Perl\bin


cd C:\Qt\4.8.0-win32-vs2010-MT
configure.exe -platform win32-msvc2010 -shared -opensource -release -fast -exceptions -no-phonon -no-phonon-backend -no-webkit -no-scripttools -no-multimedia -no-script -no-audio-backend -no-libmng -no-gif -qt-libpng -no-libtiff -no-accessibility -no-declarative -no-declarative-debug -no-openssl
nmake clean
jom

cmd.exe