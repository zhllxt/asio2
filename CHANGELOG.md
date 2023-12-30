# 2023-12-30 version 2.8:

  * Upgrade the asio library to version 1.29.0
  * Upgrade the beast library to version 351 (boost-1.84.0)
  * Upgrade the fmt library to version 9.1.0
  * Add CMake to build the example projects
  * Add mqtt, support mqtt v3.1 v3.1.1 v5.0
  * Change the "_fire_init, _fire_start, _fire_stop" triggered thread from thread main to thread 0.
  * Change the rpc's "call" interface function, before if it is called in the communication thread, it will do nothing, now it will degenerates into async_call and the return value is empty.
  * Change the file "asio2/base/selector.hpp" to "asio2/external/asio.hpp and asio2/external/beast.hpp", used to separate asio and beast header file, previously "included tcp_lient.hpp" will contain both asio and beast, but now "included tcp_lient.hpp" will only contain asio.
  * Change the serial port component which named "scp" to "serial_port".
  * Modify "push_event, next_event" function, to optimized the problem where session_ptr would be copied multiple times.
  * Modify "post, dispatch" function in post_cp.hpp, add "derive.selfptr()" to ensure that the asio::post callback function must hold the session_ptr.
  * Split the original asynchronous function "send" into "send" and "async_send" two functions. Now "send" function means synchronous sending, "async_send" means asynchronous sending (you can find the "send" in your code and replace it with "async_send" directly). Now the return value of the "send" function is the number of bytes of data sent (formerly bool), The return value of "async_send" is void. If the synchronous function "send" is called in the communication thread, it will degenerate into an asynchronous function "async_send";
  * Modify the behavior of the async_send function with the callback parameter, whenever async_send function called fails, such as an exception, the callback will be called. In previous versions, the callback was called only when write data failed.
  * Modify the http::request and http::response in the http callback function to http::web_request and http::web_response, This purpose is to be compatible with boost (you can find http::request in your code and directly replace it with http::web_request. The same is for response. For details, please refer to example/http code example).
  * Remove the "asio::error_code ec" parameter in all callback functions, for example: it used to be bind_start([](asio::error_code ec){}); now it's bind_start([](){}); Now you need to use asio2::get_last_error(); function to determine whether there is an error; Modified interfaces include: bind_start,bind_stop,bind_connect,bind_disconnect and async_call of rpc callback functions;
  * Remove the error_code parameters in several functions, mainly include http_client::execute,ping::execute,http::make_request,http::make_response, and so on..., now you need to use asio2::get_last_error() to determine whether an error has occurred;
  * Remove "thread_local static error_code ec_ignore;" in asio2/base/error.hpp.
  * Resolve compiling errors under "Visual Studio 2017 - Windows XP (v141_xp)", changed the std::shared_mutex to asio2_shared_mutex marco.
  * Fixed bug : after call server.stop(); in some cases, the server still accept session and the session can "recv send data" normaly. This will cause the server to never exit.
  * Fixed bug : in asio2/base/error.hpp : thread_local static error_code ec_last; In vs2017 and some cases, this maybe cause crash before the "main" function.
  * fixed bug : The reconnect_timer will be invalid when "client.start(...);client.stop();client.start(...);".
  * fixed bug : the _fire_init() notify is not called always in thread 0.
  * fixed bug : Incorrect parsing of filename and content_type in asio2/http/multipart.hpp.
  * fixed bug : asio2/util/ini.hpp, When the exe file name does not contain dot "." , The automatically generated ini file name is incorrect.
  * fixed bug : when call "start" with some exception, the "state" will be "starting", this will cause "start" failed next time forever.
  * fixed bug : it will cause crash when call "client.start" multi times, all "ssl client" and "websocket client" will be affected by this bug, This is because multithreading reads and writes "ws_stream_".
  * fixed bug : The code error traversing "http_router's wildcard_routers_" causes crash.
  * fixed bug : When the timer is stopped immediately after it is started in the io_context thread, the timer will stop fails.
  * fixed bug : If user has called "http::response.defer()" in the recv callback, it maybe cause crash if the session has disconnected before the "defer" has executed, beacuse the "rep_" was destroyed already at this time.
  * fixed bug : If user has called "http::response.defer()" in the recv callback, and the client has not a "keep_alive" option, the response will can not be send to the client correctly.
  * fixed bug : If user has called "http::response.defer()" in the recv callback, the variable "req_" maybe read write in two threads at the same time, and this maybe cause crash.
  * fixed bug : it might find a callback function that does not match the id of rdc component.
  * fixed bug : in some cases, after client.stop(), the client.is_stopped() maybe false.
  * Fixed the problem where kcp server and client did not send fin frame correctly.
  * Fixed the problem where the endian of the KCP handshake data header was not handled correctly.
  * Fixed problem : when the client.stop is called, and at this time the async_send is called in another thread, the async_send maybe has unexpected behavior (e.g. async_send's callback is not called).
  * Fixed unnecessary memory allocation problems in allocator to reduce the number of memory allocation. Now all timers do not use customized allocator to allocate memory, but use asio to allocate memory automatically.
  * Many other general enhancements, code fixes and improvements. See the commits records between 2020-12-23 and 2023-12-30.

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
