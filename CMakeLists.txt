cmake_minimum_required(VERSION 3.13)
project(YourAppNameHere)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(MACOSX TRUE)
endif()

if(WIN32)
	message("Detected platform: Windows")
	set(CMAKE_CXX_GLAGS "${CMAKE_CXX_FLAGS} -D__WIN32__ -mthreads")
elseif(MACOSX)
	message("Detected platform: MacOSX (Darwin)")
	link_directories("/usr/local/lib")
	link_directories("/opt/homebrew/opt/libpq/lib")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__MACOSX__")
else()
	message("Detected platform: Unix-like")
	link_directories("/usr/local/pgsql/lib")
	execute_process(COMMAND "pg_config" "--includedir" OUTPUT_VARIABLE PG_INCLUDE_DIR)
	include_directories("${PG_INCLUDE_DIR}")
endif()

# add src and deps include path
include_directories(deps/include)
include_directories(src)

# deps library paths
link_directories(deps/lib)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	# WARNING: BUILDING WITH THIS FLAG DISABLES MULTI-THREADING
	# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDISABLE_THREAD_LOCAL")

	# DISABLE_THREAD_LOCAL avoids a crash caused by MinGW64 at startup when thread_local storage is used on some types
	# this crash can be mitigated by copying the libstdc++-6.dll from mingw/bin into the build directory
endif()

# add sources
file(GLOB_RECURSE headers src/*.h)
file(GLOB_RECURSE sources src/*.c src/*.cpp)

add_executable(${PROJECT_NAME} ${headers} ${sources})

# uncomment next line to enable perf profiling
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_PERF_PROFILING")

# add required libraries here
target_link_libraries(${PROJECT_NAME} PUBLIC amqpcpp pq pgcommon pgport)

if(MACOSX)
	target_compile_options(${PROJECT_NAME} PUBLIC -Duint=unsigned)
endif()

# add platform specific libraries here
if(WIN32)
	target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32 wsock32)
	target_compile_options(${PROJECT_NAME} PUBLIC -Duint=unsigned
		-DWIN32_LEAN_AND_MEAN # Don't include all the unused junk from the windows headers
		-D_WIN32_WINNT=0x0601 # Target windows 7 for ASIO
		-DWIN32
	)
elseif(MACOSX)
	target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl)
else()
	target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl stdc++fs)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PUBLIC
	-Wall -Werror=return-type -Wfatal-errors
	-std=c++17
)

if(NOT MACOSX)
	target_compile_options(${PROJECT_NAME} PUBLIC -march=x86-64)
endif()
