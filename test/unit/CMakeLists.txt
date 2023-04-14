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

function (AddUtilExecutableTarget InputTargetName)
    set(TARGET_NAME ${InputTargetName})
	link_directories(${ASIO2_LIBS_DIR})
    add_executable (${TARGET_NAME} ${TARGET_NAME}.cpp)
    set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "unit/utils")
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
    set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "unit/ssl")
    set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ASIO2_EXES_DIR})
	target_link_libraries(${TARGET_NAME} ${OPENSSL_LIBS})
    target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${TARGET_NAME} ${GENERAL_LIBS})
	if (MSVC)
      set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()
endfunction()

AddExecutableTarget(mqtt)
AddExecutableTarget(rpc)
AddExecutableTarget(rpc_kcp)
AddExecutableTarget(timer)
AddExecutableTarget(timer_enable_error)
AddExecutableTarget(http)
AddExecutableTarget(shared_iopool)
AddExecutableTarget(tcp_general)
AddExecutableTarget(tcp_dgram)
AddExecutableTarget(tcp_custom)
AddExecutableTarget(websocket)
AddExecutableTarget(udp)

AddUtilExecutableTarget(aes)
AddUtilExecutableTarget(base64)
AddUtilExecutableTarget(codecvt)
AddUtilExecutableTarget(des)
AddUtilExecutableTarget(ini)
AddUtilExecutableTarget(md5)
AddUtilExecutableTarget(sha1)
AddUtilExecutableTarget(uuid)
AddUtilExecutableTarget(thread_pool)
AddUtilExecutableTarget(zlib)
AddUtilExecutableTarget(event_dispatcher)
AddUtilExecutableTarget(reflection)
AddUtilExecutableTarget(strutil)

AddSslExecutableTarget(https)
AddSslExecutableTarget(rdc)
AddSslExecutableTarget(rate_limit)
AddSslExecutableTarget(start_stop)
