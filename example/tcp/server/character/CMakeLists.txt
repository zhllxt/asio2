#
# Copyright (c) 2017-2023 zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

GroupSources (include/asio2 "/")
GroupSources (3rd/asio "/")

aux_source_directory(. SRC_FILES)

source_group("" FILES ${SRC_FILES})

set(TARGET_NAME tcp_server_character)

add_executable (
    ${TARGET_NAME}
    ${ASIO2_FILES}
    ${TARGET_NAME}.cpp
)

#SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ASIO2_EXES_DIR})

set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "example")

set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ASIO2_EXES_DIR})

target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TARGET_NAME} ${GENERAL_LIBS})
