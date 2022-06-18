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
# cmake -G "Visual Studio 15 Win64" .
#

cmake_minimum_required (VERSION 3.5...3.20)



# remove last end of "/"
string(REGEX REPLACE "/$" "" CMAKELISTS_DIR_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# get current relative dir name and set target name
string(REGEX REPLACE ".*/(.*)" "\\1" CMAKELISTS_DIR_NAME ${CMAKELISTS_DIR_PATH})

# get above dir name and set target group name
get_filename_component(ASIO2_ROOT_DIR ${CMAKELISTS_DIR_PATH} DIRECTORY)
string(REGEX REPLACE "/$" "" ASIO2_ROOT_DIR ${ASIO2_ROOT_DIR})
get_filename_component(ASIO2_ROOT_DIR ${ASIO2_ROOT_DIR} DIRECTORY)
message("ASIO2_ROOT_DIR = ${ASIO2_ROOT_DIR}")



#-------------------------------------------------------------------------------

function (DoGroupSources curdir rootdir foldername folder)
    file (GLOB children RELATIVE ${ASIO2_ROOT_DIR}/${curdir} ${ASIO2_ROOT_DIR}/${curdir}/*)
    foreach (child ${children})
        if (IS_DIRECTORY ${ASIO2_ROOT_DIR}/${curdir}/${child})
            DoGroupSources (${curdir}/${child} ${rootdir} ${foldername} ${folder})
        elseif (${child} STREQUAL "CMakeLists.txt")
            source_group("" FILES ${ASIO2_ROOT_DIR}/${curdir}/${child})
        else()
            string (REGEX REPLACE ^${rootdir} ${folder} groupname ${curdir})
            string (REPLACE "/" "\\" groupname ${groupname})
            source_group (${foldername}${groupname} FILES ${ASIO2_ROOT_DIR}/${curdir}/${child})
        endif()
    endforeach()
endfunction()

function (GroupSources curdir folder)
    string (FIND ${curdir} "/" Separator)
    if(${Separator} EQUAL -1)
        set(Separator 0)
    else ()
        math(EXPR Separator "${Separator} + 1")
    endif()
    string (SUBSTRING ${curdir} ${Separator} -1 foldername)
    DoGroupSources (${curdir} ${curdir} ${foldername} ${folder})
endfunction()

#-------------------------------------------------------------------------------
#
# project
#
#-------------------------------------------------------------------------------

project (asio2_client_example)

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


# ---------------------------------------------------------------------------------------
# Set default build to debug
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose Release or Debug" FORCE)
    message("CMAKE_BUILD_TYPE is not set, Set default build to Debug.")
endif()
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

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
	set(GENERAL_LIBS "ws2_32;mswsock;")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -ldl)
ELSE ()
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${ASIO2_EXES_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${ASIO2_EXES_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ASIO2_EXES_DIR})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)


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

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      add_compile_options(
          /bigobj       # large object file format
          #/permissive-  # strict C++
          #/wd4503      # decorated name length exceeded, name was truncated
          /W4           # enable all warnings
          /JMC
          )
  
      set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MTd")
      set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /Zi /Ob2 /Oi /Ot /MT")
  
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    else()
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
      set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG:incremental")
    endif ()
else()
    set (THREADS_PREFER_PTHREAD_FLAG ON)
    find_package (Threads)

    set( CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter")

    #set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wrange-loop-analysis")
    endif ()
endif()


# https://stackoverflow.com/questions/44705845/file-too-big-compiling-on-cygwin-g
if(MINGW OR CYGWIN)
  add_definitions(-O3)
endif()


#include_directories (/usr/local/include) # for boost header file include
include_directories (${ASIO2_ROOT_DIR}/3rd)
include_directories (${ASIO2_ROOT_DIR}/include)

#link_directories    (/usr/local/lib)     # for boost library file include


file (GLOB_RECURSE  ASIO2_FILES  ${ASIO2_ROOT_DIR}/include/asio2/*.*)
file (GLOB_RECURSE   ASIO_FILES  ${ASIO2_ROOT_DIR}/3rd/asio/*.*)
file (GLOB_RECURSE CEREAL_FILES  ${ASIO2_ROOT_DIR}/3rd/cereal/*.*)

# exclude : asio/impl/src.cpp
list(REMOVE_ITEM ASIO_FILES ${ASIO2_ROOT_DIR}/3rd/asio/impl/src.cpp)


add_subdirectory (${ASIO2_ROOT_DIR}/example/http/client          asio2_client_example_project/http_client)
add_subdirectory (${ASIO2_ROOT_DIR}/example/rpc/client           asio2_client_example_project/rpc_client)
add_subdirectory (${ASIO2_ROOT_DIR}/example/tcp/client           asio2_client_example_project/tcp_client)
add_subdirectory (${ASIO2_ROOT_DIR}/example/udp/client           asio2_client_example_project/udp_client)
add_subdirectory (${ASIO2_ROOT_DIR}/example/websocket/client     asio2_client_example_project/websocket_client)
add_subdirectory (${ASIO2_ROOT_DIR}/example/mqtt/client          asio2_client_example_project/mqtt_client)

if(MINGW OR CYGWIN)
# no mingw compiled openssl
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # no MacOS compiled openssl
    else()
        add_subdirectory (${ASIO2_ROOT_DIR}/example/ssl/http/client      asio2_client_example_project/ssl_http_client     )
        add_subdirectory (${ASIO2_ROOT_DIR}/example/ssl/rpc/client       asio2_client_example_project/ssl_rpc_client      )
        add_subdirectory (${ASIO2_ROOT_DIR}/example/ssl/tcp/client       asio2_client_example_project/ssl_tcp_client      )
        add_subdirectory (${ASIO2_ROOT_DIR}/example/ssl/websocket/client asio2_client_example_project/ssl_websocket_client)
    endif()
endif()
