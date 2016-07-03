

NAME="box-resize"


DIR_SRC=`pwd`
DIR_BUILD=/tmp/$NAME

rm -Rf $DIR_BUILD
mkdir $DIR_BUILD
cd $DIR_BUILD

cmake -G "Eclipse CDT4 - Unix Makefiles" $DIR_SRC -DCMAKE_BUILD_TYPE=Release
make -j5

