#!/bin/bash

function updateVersion {
	versionString=$(cat fw-version)
	IFS='.' read -ra versionNo <<< "$versionString"
	typeset -i major=${versionNo[0]}
	typeset -i minor=${versionNo[1]}
	minor=$minor+1
	echo $major.$minor > build/dist/fw-version

}

printf "\n Building Debug...\n\n"
if [ ! -d build ]; then
    mkdir build
fi
cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..
RESULT=$?

if [[ $RESULT != 0 ]]; then
	printf "\nCMake error. exiting.\n"
	exit 1
fi

if [[ "$1" = "-R" ]]; then
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
