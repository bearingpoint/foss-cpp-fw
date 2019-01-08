#!/bin/bash

# optional parameters:
#
# --release		perform a RELEASE build instead of DEBUG
# -R			Rebuild (clean before build)
# --with-SDL	build with SDL support
# --no-GLFW		build without GLFW support
# --with-Box2D	build with Box2D support
#
# By default the script builds Debug with GLFW support
#

function updateVersion {
	versionString=$(cat fw-version)
	IFS='.' read -ra versionNo <<< "$versionString"
	typeset -i major=${versionNo[0]}
	typeset -i minor=${versionNo[1]}
	minor=$minor+1
	echo $major.$minor > build/dist/fw-version
}

REBUILD=0
RELEASE=0
WITH_SDL=0
WITH_GLFW=1
WITH_BOX2D=0
while test $# -gt 0
do
	echo "arg $1"
	
    case "$1" in
		--release) RELEASE=1
			;;
		-R) REBUILD=1
            ;;
        --with-SDL) WITH_SDL=1
            ;;
		--no-GLFW) WITH_GLFW=0
			;;
		--with-Box2D) WITH_BOX2D=1
			;;
        --*) echo "bad option $1"
            ;;
    esac
    shift
done

if [ $RELEASE = 1 ]; then
	printf "\n Building Release...\n\n"
else
	printf "\n Building Debug...\n\n"
fi

if [ ! -d build ]; then
    mkdir build
fi
cd build

CMAKE_PARAM=""
if [ $WITH_SDL = 1 ]; then
	printf "Building with SDL support.\n"
	CMAKE_PARAM="$CMAKE_PARAM -DWITH_SDL=ON"
else
	CMAKE_PARAM="$CMAKE_PARAM -DWITH_SDL=OFF"
fi
if [ $WITH_GLFW = 1 ]; then
	printf "Building with GLFW support.\n"
	CMAKE_PARAM="$CMAKE_PARAM -DWITH_GLFW=ON"
else
	CMAKE_PARAM="$CMAKE_PARAM -DWITH_GLFW=OFF"
fi
if [ $WITH_BOX2D = 1 ]; then
	printf "Building with Box2D support.\n"
	CMAKE_PARAM="$CMAKE_PARAM -DWITH_BOX2D=ON"
else
	CMAKE_PARAM="$CMAKE_PARAM -DWITH_BOX2D=OFF"
fi

CMAKE_BUILD_TYPE="Debug"
if [ $RELEASE = 1 ]; then
	CMAKE_BUILD_TYPE="Release"
fi

cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE $CMAKE_PARAM -G "Unix Makefiles" ..
RESULT=$?

if [[ $RESULT != 0 ]]; then
	printf "\nCMake error. exiting.\n"
	exit 1
fi

if [ $REBUILD = 1 ]; then
	printf "\nFull rebuild, performing clean...\n\n"
	make clean
else
	printf "\nPass -R to force a full rebuild (clean all first)\n\n"
fi

make -j4 && make install
RESULT=$?

cd ..

if [[ $RESULT = 0 ]]; then
	updateVersion
	printf "\n Success. Your files are in ./build/dist\n\n"
	exit 0
else
	printf "\n Errors encountered. \n\n"
	exit 1
fi
