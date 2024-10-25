#!/bin/bash

set -e # immediately exit on error

# select makefile generator for CMake based on operating system type
if [[ "$OSTYPE" == "msys" ]]; then
	MAKEFILE_GENERATOR="MinGW Makefiles"	# this is for windows/mingw/msys
	MAKE_EXECUTABLE="mingw32-make"
elif [[ "$OSTYPE" == "linux-gnu" ]] || [[ "$OSTYPE" == "darwin"* ]]; then
	MAKEFILE_GENERATOR="Unix Makefiles"		# standard linux
	MAKE_EXECUTABLE="make"
else
	echo "Unknown OS type $OSTYPE, don't know which makefile generator to use!"
	exit 1
fi

function getMinGWPath() {
	local GDB_SUFFIX=/bin/gdb
	local GDB_PATH=`which gdb`
	local PATH_LEN=$((${#GDB_PATH} - ${#GDB_SUFFIX}))
	local MINGW_PATH=${GDB_PATH:0:PATH_LEN}
	echo $MINGW_PATH
}

function getInstallPrefix() {
	if [[ "$OSTYPE" == "msys" ]]; then
		echo $(getMinGWPath)
	else
		echo "/usr/local"
	fi
}

mkdir -p deps/include
mkdir -p deps/lib
mkdir -p deps/bin
cd deps

# ZLIB
if [ ! -f "$(getInstallPrefix)/include/zlib.h" ]; then
	printf "\nBuilding zlib-1.2.13... +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n"
	if [ ! -d ./zlib-1.2.13 ]; then
		tar -xpf ../3rd-party/zlib-1.2.13.tar.gz
	fi
	cd zlib-1.2.13
	./configure --prefix=$(getInstallPrefix)
	${MAKE_EXECUTABLE} install
	cd ../
else
	printf "\nzlib-1.2.13 already installed\n\n"
fi

# PostgreSQL
if [ ! -f "$(getInstallPrefix)/include/pg_config.h" ]; then
	printf "\nBuilding postgresql-15.0... +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n"
	if [ ! -d ./postgresql-15.0 ]; then
		tar -xpf ../3rd-party/postgresql-15.0.tar.gz
	fi

	cd postgresql-15.0
	./configure --with-includes=../include/ --with-libraries=../lib/ --prefix=$(getInstallPrefix)
	${MAKE_EXECUTABLE} install
	cd ../
else
	printf "\npostgresql-15.0 already installed\n\n"
fi

# AMQP-CPP
if [ ! -f "$(getInstallPrefix)/include/amqpcpp.h" ]; then
	printf "\nBuilding AMQP-CPP...+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n"
	if [ ! -d ./AMQP-CPP ]; then
		git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
	fi
	cd AMQP-CPP
	if [ ! -d build ]; then
		mkdir build
	fi
	cd build
	cmake .. -G "${MAKEFILE_GENERATOR}" -DCMAKE_INSTALL_PREFIX=$(getInstallPrefix)
	${MAKE_EXECUTABLE} install
	cd ../../
else
	printf "\nAMQP-CPP already installed\n\n"
fi

# ASIO
if [ ! -f "$(getInstallPrefix)/include/asio.hpp" ]; then
	printf "\nBuilding ASIO...+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n"
	if [ ! -d ./asio ]; then
		git clone https://github.com/chriskohlhoff/asio
	fi
	cd asio/asio
	git checkout asio-1-14-0
	if [[ "$OSTYPE" == "linux-gnu" ]] || [[ "$OSTYPE" == "darwin"* ]]; then
		./autogen.sh
		./configure --prefix=$(getInstallPrefix)
		make install
	else
		cp -r include/* $(getInstallPrefix)/include/
	fi
	cd ../../
else
	printf "\nASIO already installed\n\n"
fi

# JSON
if [ ! -d ./json ]; then
	printf "\nBuilding JSON...+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n"
	if [ ! -d ./json ]; then
		git clone https://github.com/nlohmann/json
	fi
	cd json
	git checkout v3.7.3
	if [ ! -d build ]; then
		mkdir build
	fi
	cd build
	cmake .. -G "${MAKEFILE_GENERATOR}" -DJSON_BuildTests=OFF -DCMAKE_INSTALL_PREFIX=$(getInstallPrefix)
	${MAKE_EXECUTABLE} install
	cd ../../
else
	printf "\nJSON already installed\n\n"
fi

printf  "\n=====================================================\nDependencies installed.\n\n";

# get out of ./deps/
cd ../

rm -rf deps/