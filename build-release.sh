

NAME="audigger-release"


DIR_SRC=`pwd`
DIR_BUILD=~/build-forge/$NAME

rm -Rf $DIR_BUILD
mkdir -p $DIR_BUILD
cd $DIR_BUILD

cmake -G "Eclipse CDT4 - Unix Makefiles" $DIR_SRC -DCMAKE_BUILD_TYPE=Release
make -j5

cpack -G DEB
dpkg-deb -f *.deb
