#!/bin/bash

# This script builds the library to ensure there are no errors before publishing.

BUILD_DIR="build/Release"

if [ ! -d $BUILD_DIR ]; then
	mkdir -p $BUILD_DIR
fi
cd $BUILD_DIR

# select makefile generator for CMake based on operating system type
if [[ "$OSTYPE" == "msys" ]]; then
	MAKEFILE_GENERATOR="MinGW Makefiles" # this is for windows/mingw/msys
	MAKE_EXECUTABLE="mingw32-make"
elif [[ "$OSTYPE" == "linux-gnu" ]] || [[ "$OSTYPE" == "darwin"* ]]; then
	MAKEFILE_GENERATOR="Unix Makefiles" # standard linux
	MAKE_EXECUTABLE="make"
else
	echo "Unknown OS type $OSTYPE, don't know which makefile generator to use!"
	exit 1
fi

cmake -DCMAKE_BUILD_TYPE=Release -G "$MAKEFILE_GENERATOR" ../..
RESULT=$?
if [[ $RESULT != 0 ]]; then
	printf "\nCMake error. exiting.\n"
	exit 1
fi
$MAKE_EXECUTABLE -j8
RESULT=$?

cd ../..

if [[ $RESULT = 0 ]]; then
	printf "\n Success.\n\n"
	exit 0
else
	printf "\n Errors encountered. \n\n"
	exit 1
fi
