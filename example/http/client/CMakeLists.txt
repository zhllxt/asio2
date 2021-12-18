#
# COPYRIGHT (C) 2017-2021, zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
# (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
#

GroupSources (asio2 "/")
GroupSources (asio "/")
GroupSources (bho "/")

aux_source_directory(. SRC_FILES)

source_group("" FILES ${SRC_FILES})

set(TARGET_NAME http_client)

add_executable (
    ${TARGET_NAME}
    ${ASIO2_FILES} ${SRC_FILES}
    ${TARGET_NAME}.cpp
)

#SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ASIO2_EXES_DIR})

set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ASIO2_EXES_DIR})

target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TARGET_NAME} ${GENERAL_LIBS})



#####################################################################################################


#GroupSources (asio2 "/")
#GroupSources (asio "/")
#GroupSources (bho "/")
#
#aux_source_directory(. SRC_FILES)
#
#file (GLOB_RECURSE CEREAL_FILES ${ASIO2_ROOT_DIR}/cereal/*.*)
#file (GLOB_RECURSE FMT_FILES ${ASIO2_ROOT_DIR}/fmt/*.*)
#file (GLOB_RECURSE JSON_FILES ${ASIO2_ROOT_DIR}/nlohmann/*.*)
#file (GLOB_RECURSE PFR_FILES ${ASIO2_ROOT_DIR}/pfr/*.*)
#file (GLOB_RECURSE SPDLOG_FILES ${ASIO2_ROOT_DIR}/spdlog/*.*)
#
#
#GroupSources (cereal "/")
#GroupSources (fmt "/")
#GroupSources (nlohmann "/")
#GroupSources (pfr "/")
#GroupSources (spdlog "/")
#
#
#source_group("" FILES ${SRC_FILES})
#
#set(TARGET_NAME http_client)
#
#add_executable (
#    ${TARGET_NAME}
#    ${ASIO2_FILES} ${FMT_FILES} ${JSON_FILES} ${PFR_FILES} ${SPDLOG_FILES} ${CEREAL_FILES} ${ASIO_FILES} ${BHO_FILES} ${SRC_FILES}
#    ${TARGET_NAME}.cpp
#)
#
##SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ASIO2_EXES_DIR})
#
#set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ASIO2_EXES_DIR})
#
#target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(${TARGET_NAME} ${GENERAL_LIBS})
#