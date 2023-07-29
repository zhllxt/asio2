#
# COPYRIGHT (C) 2017-2021, zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
# (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
#

add_subdirectory (asio2_rpc_qps_client)
add_subdirectory (asio2_rpc_qps_server)

if (EXISTS "${ASIO2_ROOT_DIR}/test/bench/rpc/rest_rpc-master")
  add_subdirectory (rest_rpc_qps_client)
  add_subdirectory (rest_rpc_qps_server)
endif()
