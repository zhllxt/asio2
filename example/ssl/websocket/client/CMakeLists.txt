#
# COPYRIGHT (C) 2017-2021, zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
# (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
#

GroupSources (include/asio2 "/")
GroupSources (3rd/asio "/")

aux_source_directory(. SRC_FILES)

source_group("" FILES ${SRC_FILES})

set(TARGET_NAME ssl_websocket_client)

link_directories    (${ASIO2_LIBS_DIR})

add_executable (
    ${TARGET_NAME}
    ${ASIO2_FILES} ${SRC_FILES}
    ${TARGET_NAME}.cpp
)

target_link_libraries(${TARGET_NAME} ${OPENSSL_LIBS})

set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "ssl")

#SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ASIO2_EXES_DIR})

set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ASIO2_EXES_DIR})

if (MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TARGET_NAME} ${GENERAL_LIBS})
