

NAME="audigger-release-noconsole"


DIR_SRC=`pwd`
DIR_BUILD=~/build/$NAME

rm -Rf $DIR_BUILD
mkdir -p $DIR_BUILD
cd $DIR_BUILD

cmake -G "Eclipse CDT4 - Unix Makefiles" $DIR_SRC -DCMAKE_BUILD_TYPE=Release -DDISABLE_CONSOLE=1
make -j5

cpack -G DEB
dpkg-deb -f *.deb
