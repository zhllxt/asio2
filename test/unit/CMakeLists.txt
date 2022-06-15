#
# COPYRIGHT (C) 2017-2021, zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
# (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
#

#GroupSources (include/asio2 "/")
#GroupSources (3rd/asio "/")

aux_source_directory(. SRC_FILES)

source_group("" FILES ${SRC_FILES})

function (AddExecutableTarget InputTargetName)
    set(TARGET_NAME ${InputTargetName})
	link_directories(${ASIO2_LIBS_DIR})
    add_executable (${TARGET_NAME} ${TARGET_NAME}.cpp)
    set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "unit")
    set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ASIO2_EXES_DIR})
    target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${TARGET_NAME} ${GENERAL_LIBS})
	if (MSVC)
      set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()
endfunction()

function (AddSslExecutableTarget InputTargetName)
    set(TARGET_NAME ${InputTargetName})
	link_directories(${ASIO2_LIBS_DIR})
    add_executable (${TARGET_NAME} ${TARGET_NAME}.cpp)
    set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "unit")
    set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ASIO2_EXES_DIR})
	target_link_libraries(${TARGET_NAME} ${OPENSSL_LIBS})
    target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${TARGET_NAME} ${GENERAL_LIBS})
	if (MSVC)
      set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()
endfunction()

AddExecutableTarget(aes)
AddExecutableTarget(base64)
AddExecutableTarget(des)
AddExecutableTarget(ini)
AddExecutableTarget(md5)
AddExecutableTarget(sha1)
AddExecutableTarget(uuid)
AddExecutableTarget(rpc)
AddExecutableTarget(thread_pool)
AddExecutableTarget(timer)
AddExecutableTarget(timer_enable_error)
AddExecutableTarget(http)
AddExecutableTarget(shared_iopool)
AddExecutableTarget(tcp_general)
AddExecutableTarget(websocket)
AddExecutableTarget(zlib)
AddExecutableTarget(event_dispatcher)
AddExecutableTarget(reflection)

if(MINGW OR CYGWIN)
# no mingw compiled openssl
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # no MacOS compiled openssl
    else()
        AddSslExecutableTarget(https)
    endif()
endif()
