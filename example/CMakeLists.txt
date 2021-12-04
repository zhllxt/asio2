#
# COPYRIGHT (C) 2017-2021, zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
# (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
#
#
# export CC=/usr/local/bin/gcc
# export CXX=/usr/local/bin/g++
#

cmake_minimum_required (VERSION 2.8)



# remove last end of "/"
string(REGEX REPLACE "/$" "" CMAKELISTS_DIR_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# get current relative dir name and set target name
string(REGEX REPLACE ".*/(.*)" "\\1" CMAKELISTS_DIR_NAME ${CMAKELISTS_DIR_PATH})

# get above dir name and set target group name
get_filename_component(ASIO2_ROOT_DIR ${CMAKELISTS_DIR_PATH} DIRECTORY)
message("ASIO2_ROOT_DIR = ${ASIO2_ROOT_DIR}")



#-------------------------------------------------------------------------------

function (DoGroupSources curdir rootdir folder)
    file (GLOB children RELATIVE ${ASIO2_ROOT_DIR}/${curdir} ${ASIO2_ROOT_DIR}/${curdir}/*)
    foreach (child ${children})
        if (IS_DIRECTORY ${ASIO2_ROOT_DIR}/${curdir}/${child})
            DoGroupSources (${curdir}/${child} ${rootdir} ${folder})
        elseif (${child} STREQUAL "CMakeLists.txt")
            source_group("" FILES ${ASIO2_ROOT_DIR}/${curdir}/${child})
        else()
            string (REGEX REPLACE ^${rootdir} ${folder} groupname ${curdir})
            string (REPLACE "/" "\\" groupname ${groupname})
            source_group (${rootdir}${groupname} FILES ${ASIO2_ROOT_DIR}/${curdir}/${child})
        endif()
    endforeach()
endfunction()

function (GroupSources curdir folder)
    DoGroupSources (${curdir} ${curdir} ${folder})
endfunction()

#-------------------------------------------------------------------------------
#
# project
#
#-------------------------------------------------------------------------------

project (asio2_example)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

# how to detect current is 32 bit or 64 bit ?
#if(NOT "${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
#endif()
#
#if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
#endif()
#
#if(NOT CMAKE_CL_64)
#endif()
#
#if(NOT ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win64"))
#endif()
#
#if ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
#endif ()

#if(CMAKE_CL_64)
#    set(ASIO2_LIBS_DIR ${ASIO2_ROOT_DIR}/lib/x64)
#else(CMAKE_CL_64)
#    set(ASIO2_LIBS_DIR ${ASIO2_ROOT_DIR}/lib/x86)
#endif(CMAKE_CL_64)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ASIO2_LIBS_DIR ${ASIO2_ROOT_DIR}/lib/x64)
    set(ASIO2_EXES_DIR ${ASIO2_ROOT_DIR}/example/bin/x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ASIO2_LIBS_DIR ${ASIO2_ROOT_DIR}/lib/x86)
    set(ASIO2_EXES_DIR ${ASIO2_ROOT_DIR}/example/bin/x86)
endif()

message("ASIO2_LIBS_DIR = ${ASIO2_LIBS_DIR}")
message("ASIO2_EXES_DIR = ${ASIO2_EXES_DIR}")
 
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message("if the [unrecognized command line option '-std=c++17'] error occurs, attempt to execute this command: export CC=/usr/local/bin/gcc; export CXX=/usr/local/bin/g++;")
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(OPENSSL_LIBS "libssl.lib;libcrypto.lib;Crypt32.lib;")
	set(GENERAL_LIBS "")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
ELSE ()
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${ASIO2_EXES_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${ASIO2_EXES_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ASIO2_EXES_DIR})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)



# ---------------------------------------------------------------------------------------
# Set default build to release
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

# ---------------------------------------------------------------------------------------
# Compiler config
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME MATCHES "CYGWIN" OR CMAKE_SYSTEM_NAME MATCHES "MSYS")
    set(CMAKE_CXX_EXTENSIONS ON)
endif()



if (MSVC)
    set (CMAKE_VERBOSE_MAKEFILE FALSE)

    #add_definitions (
    #    -D_WIN32_WINNT=0x0601
    #    -D_SCL_SECURE_NO_WARNINGS=1
    #    -D_CRT_SECURE_NO_WARNINGS=1
    #    -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    #    -D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
    #)

    add_compile_options(
        /bigobj       # large object file format
        #/permissive-  # strict C++
        #/wd4503      # decorated name length exceeded, name was truncated
        /W4           # enable all warnings
        /MP           # Multi-processor compilation
        /JMC
        )

    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MTd /ZI")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /Zi /Ob2 /Oi /Ot /GL /MT")

    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")

else()
    set (THREADS_PREFER_PTHREAD_FLAG ON)
    find_package (Threads)

    set( CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wrange-loop-analysis")
    endif ()
endif()


include_directories (${ASIO2_ROOT_DIR})


file (GLOB_RECURSE ASIO2_FILES ${ASIO2_ROOT_DIR}/asio2/*.*)
file (GLOB_RECURSE ASIO_FILES  ${ASIO2_ROOT_DIR}/asio/*.*)
file (GLOB_RECURSE BEAST_FILES ${ASIO2_ROOT_DIR}/beast/*.*)

# exclude : asio/impl/src.cpp
list(REMOVE_ITEM ASIO_FILES ${ASIO2_ROOT_DIR}/asio/impl/src.cpp)


add_subdirectory (${ASIO2_ROOT_DIR}/example/http             asio2_example_project/http             )
add_subdirectory (${ASIO2_ROOT_DIR}/example/icmp             asio2_example_project/icmp             )
add_subdirectory (${ASIO2_ROOT_DIR}/example/rpc              asio2_example_project/rpc              )
add_subdirectory (${ASIO2_ROOT_DIR}/example/serial_port      asio2_example_project/serial_port      )
add_subdirectory (${ASIO2_ROOT_DIR}/example/tcp              asio2_example_project/tcp              )
add_subdirectory (${ASIO2_ROOT_DIR}/example/udp              asio2_example_project/udp              )
add_subdirectory (${ASIO2_ROOT_DIR}/example/websocket        asio2_example_project/websocket        )
add_subdirectory (${ASIO2_ROOT_DIR}/example/mqtt             asio2_example_project/mqtt             )
add_subdirectory (${ASIO2_ROOT_DIR}/example/custom_scheduler asio2_example_project/custom_scheduler )

add_subdirectory (${ASIO2_ROOT_DIR}/example/ssl              asio2_example_project/ssl              )

