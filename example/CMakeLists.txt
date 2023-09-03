#
# Copyright (c) 2017-2023 zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
#
# export CC=/usr/local/bin/gcc
# export CXX=/usr/local/bin/g++
#
# cmake -G "Visual Studio 15 Win64" .
#

add_subdirectory (http         )
add_subdirectory (icmp         )
add_subdirectory (inherit      )
add_subdirectory (mqtt         )
add_subdirectory (proxy        )
add_subdirectory (rate_limit   )
add_subdirectory (rpc          )
add_subdirectory (serial_port  )
add_subdirectory (shared_iopool)
add_subdirectory (tcp          )
add_subdirectory (timer        )
add_subdirectory (udp          )
add_subdirectory (websocket    )

add_subdirectory (ssl)
