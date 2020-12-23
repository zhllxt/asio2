# 2020-12-23 version 2.7:

  * Add remote data call ability.
  * Other general enhancements.

# 2020-09-24 version 2.6:

  * Fixed a bug in the connect_timeout_cp class that caused the program to crash.
  * Fixed a bug in the ws_stream_cp class that causes program assertions to fail under debug mode.
  * Fixed a bug in the rpc module that caused the program to crash after receiving illegal data.
  * Rewrote the rpc_call_cp class code to support chained calls.
  * Rewrote the api of http and websocket class.
  * Rewrote the example code.
  * Fixed and optimized the code for auto reconnection.
  * A standalone version of Beast was added to support headonly http and websocket capabilities,now all functionality is no longer dependent on the Boost library.
  * Rename the timeout function of rpc_client and rpc_session class to default_timeout function.
  * Fixed compile warnings and errors under clang. Compiled under msvc(vs2017 vs2019) gcc8.2.0 clang10.
  * Add event_guard class to enhance the reliability of event handle.
  * Remove some class, the files is : httpws_server.hpp,httpws_session.hpp,httpwss_server.hpphttpwss_session.hpp;
  * Various code fixes and improvements.
  * Upgrade the asio library to version 1.16.1
  * Upgrade the beast library to version 290 (boost-1.73.0)
  * Upgrade the cereal library to version 1.3.0
  * Upgrade the fmt library to version 7.0.3

# 2020-06-28 version 2.5:

  * Improved SSL and certificate loading capabilities.
  * Fix url_decode bug.
  * Enhanced user_data() function.
  * Other general enhancements.

# 2020-04-28 version 2.4:

  * Add call and async_call for rpc_server.
  * Add async call version without callback function.
  * Other general enhancements.
  * Bug fix : Fix the rpc bug of calling parameters is raw pointer or object refrence.

# 2020-01-10 version 2.3:

  * Add automatic reconnection mechanism in the client when disconnection occurs.
  * Bug fix : Fix the problem that data cannot be sent when the client is restarted in some cases(By adding event queue mechanism).

# 2019-11-02 version 2.2:

  * Change internal send mode from synchronous to asynchronous.

# 2019-06-28 version 2.1:

  * Fixed some bugs.

# 2019-06-04 version 2.0:

  * ##### [BIG Update!!!]
  * Rewrite all code with template metaprogramming mode.
  * Version 1.x code will no longer be maintained.

# 2018-04-12 version 1.4:

  * Fixed some bugs in session_mgr_t class.
  * Adjust some other code.

# 2018-01-24 version 1.3:

  * Replace boost::asio with asio standalone to avoid conflicts when use boost and asio2 at the same time.
  * Adjust and optimize some other code.

# 2018-01-06 version 1.2:

  * fixed bug : sender and session has't post close notify to listener when closed.
  * Redesign the sender code.
  
# 2018-01-02 version 1.1:

  * Upgrade the boost (asio) version to 1.66.0
  * Changed the parameters of the listening interface to reference
  * Modified the SSL certificate setting function, many functions need to be invoked before, now only one function needs to be called
  * Optimizes a lot of internal code

# 2017-10-27 version 1.0:

  * Initial release.
