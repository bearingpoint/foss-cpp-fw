cmake_minimum_required(VERSION 3.13)
project(FossCppFw)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(MACOSX TRUE)
endif()

if(WIN32)
	message("Detected platform: Windows")
	set(CMAKE_CXX_GLAGS "${CMAKE_CXX_FLAGS} -D__WIN32__ -mthreads")
elseif(MACOSX)
	message("Detected platform: MacOSX (Darwin)")
	link_directories("/usr/local/lib")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__MACOSX__")
else()
	message("Detected platform: Unix-like")
endif()

include_directories(src)

# add sources
file(GLOB_RECURSE headers src/*.h)
file(GLOB_RECURSE sources src/*.c src/*.cpp)

add_library(${PROJECT_NAME} ${headers} ${sources})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_PERF_PROFILING")

if(MACOSX)
	target_compile_options(${PROJECT_NAME} PUBLIC -Duint=unsigned)
endif()

# add platform specific libraries here
if(WIN32)
	target_compile_options(${PROJECT_NAME} PUBLIC -Duint=unsigned
		-DWIN32_LEAN_AND_MEAN # Don't include all the unused junk from the windows headers
		-D_WIN32_WINNT=0x0601 # Target windows 7 for ASIO
		-DWIN32
	)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PUBLIC
	-Wall -Werror=return-type -Wfatal-errors
	-std=c++17
)

if(NOT MACOSX)
	target_compile_options(${PROJECT_NAME} PUBLIC -march=x86-64)
endif()
