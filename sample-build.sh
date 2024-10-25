#!/bin/bash

# This is a sample build file that can be adapted and used to build C++ apps.

# optional parameters:
#
# --release : perform a RELEASE build instead of DEBUG

APP_NAME="SampleApp"

BUILD_TYPE="Debug"
if [[ $1 = "--release" ]]; then
	BUILD_TYPE="Release"
fi

BUILD_DIR="build/$BUILD_TYPE"

if [ ! -d $BUILD_DIR ]; then
	mkdir -p $BUILD_DIR
fi

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

if [ ! -d src/version ]; then
	mkdir src/version
fi
VERSION=`git describe`
if [[ $? = 0 ]]; then
	printf "#pragma once\n#include <string>\n\nstatic const std::string VERSION = \"$VERSION\";\n> > src/version/_version.h
else
	# otherwise this is not a git repo, so the _version.h file must have been already written by who archived the sources.
	if [ ! -f src/version/_version.h ]; then
		echo "Missing src/version/_version.h and git describe doesn't work to create it."
		echo "If you are building from a source archive (not git repository), this file should have been included in the archive."
		exit 1
	fi
fi;

cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -G "$MAKEFILE_GENERATOR" ../..
RESULT=$?
if [[ $RESULT != 0 ]]; then
	printf "\nCMake error. exiting.\n"
	exit 1
fi
$MAKE_EXECUTABLE -j8
RESULT=$?

cd ../..

if [[ $RESULT = 0 ]]; then
	if [[ $BUILD_TYPE == "Release" ]]; then
		if [ -d $BUILD_DIR/dist ]; then
			rm -rf $BUILD_DIR/dist
		fi
		mkdir -p $BUILD_DIR/dist
		cp "$BUILD_DIR/$APP_NAME*" "$BUILD_DIR/dist/"
	fi

	printf "\n Success.\n\n"
	exit 0
else
	printf "\n Errors encountered. \n\n"
	exit 1
fi
