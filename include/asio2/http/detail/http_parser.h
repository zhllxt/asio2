/*
 * this file has modifyed by 37792738@qq.com,so that it can be used in C++ 
 * programs,and add a param "void * user_data" into the http callback,and 
 * add a param "void * user_data" into the http_parser_execute function.
 */

/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __ASIO2_HTTP_PARSER_H__
#define __ASIO2_HTTP_PARSER_H__

#include <asio2/base/detail/push_options.hpp>

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::http::parses
#else
namespace boost::beast::http::parses
#endif
{
//#ifdef __cplusplus
//extern "C" {
//#endif

/* Also update SONAME in the Makefile whenever you change these. */
#define HTTP_PARSER_VERSION_MAJOR 2
#define HTTP_PARSER_VERSION_MINOR 9
#define HTTP_PARSER_VERSION_PATCH 4

#include <stddef.h>
#if defined(_WIN32) && !defined(__MINGW32__) && \
  (!defined(_MSC_VER) || _MSC_VER<1600) && !defined(__WINE__)
#include <BaseTsd.h>
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#elif (defined(__sun) || defined(__sun__)) && defined(__SunOS_5_9)
#include <sys/inttypes.h>
#else
#include <stdint.h>
#endif

/* Compile with -DHTTP_PARSER_STRICT=0 to make less checks, but run
 * faster
 */
#ifndef HTTP_PARSER_STRICT
# define HTTP_PARSER_STRICT 1
#endif

/* Maximium header size allowed. If the macro is not defined
 * before including this header then the default is used. To
 * change the maximum header size, define the macro in the build
 * environment (e.g. -DHTTP_MAX_HEADER_SIZE=<value>). To remove
 * the effective limit on the size of the header, define the macro
 * to a very large number (e.g. -DHTTP_MAX_HEADER_SIZE=0x7fffffff)
 */
#ifndef HTTP_MAX_HEADER_SIZE
# define HTTP_MAX_HEADER_SIZE (80*1024)
#endif

typedef struct http_parser http_parser;
typedef struct http_parser_settings http_parser_settings;


/* Callbacks should return non-zero to indicate an error. The parser will
 * then halt execution.
 *
 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
 * returning '1' from on_headers_complete will tell the parser that it
 * should not expect a body. This is used when receiving a response to a
 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * chunked' headers that indicate the presence of a body.
 *
 * Returning `2` from on_headers_complete will tell parser that it should not
 * expect neither a body nor any futher responses on this connection. This is
 * useful for handling responses to a CONNECT request which may not contain
 * `Upgrade` or `Connection: upgrade` headers.
 *
 * http_data_cb does not return data chunks. It will be called arbitrarily
 * many times for each string. E.G. you might get 10 callbacks for "on_url"
 * each providing just a few characters more data.
 */
typedef int (*http_data_cb) (http_parser*, const char *at, size_t length, void * user_data);
typedef int (*http_cb) (http_parser*, void * user_data);


/* Status Codes */
#define HTTP_STATUS_MAP(HTTP_XX)                                                 \
  HTTP_XX(100, continuation,                    Continue)                        \
  HTTP_XX(101, switching_protocols,             Switching Protocols)             \
  HTTP_XX(102, processing,                      Processing)                      \
  HTTP_XX(200, ok,                              OK)                              \
  HTTP_XX(201, created,                         Created)                         \
  HTTP_XX(202, accepted,                        Accepted)                        \
  HTTP_XX(203, non_authoritative_information,   Non-Authoritative Information)   \
  HTTP_XX(204, no_content,                      No Content)                      \
  HTTP_XX(205, reset_content,                   Reset Content)                   \
  HTTP_XX(206, partial_content,                 Partial Content)                 \
  HTTP_XX(207, multi_status,                    Multi-Status)                    \
  HTTP_XX(208, already_reported,                Already Reported)                \
  HTTP_XX(226, im_used,                         IM Used)                         \
  HTTP_XX(300, multiple_choices,                Multiple Choices)                \
  HTTP_XX(301, moved_permanently,               Moved Permanently)               \
  HTTP_XX(302, found,                           Found)                           \
  HTTP_XX(303, see_other,                       See Other)                       \
  HTTP_XX(304, not_modified,                    Not Modified)                    \
  HTTP_XX(305, use_proxy,                       Use Proxy)                       \
  HTTP_XX(307, temporary_redirect,              Temporary Redirect)              \
  HTTP_XX(308, permanent_redirect,              Permanent Redirect)              \
  HTTP_XX(400, bad_request,                     Bad Request)                     \
  HTTP_XX(401, unauthorized,                    Unauthorized)                    \
  HTTP_XX(402, payment_required,                Payment Required)                \
  HTTP_XX(403, forbidden,                       Forbidden)                       \
  HTTP_XX(404, not_found,                       Not Found)                       \
  HTTP_XX(405, method_not_allowed,              Method Not Allowed)              \
  HTTP_XX(406, not_acceptable,                  Not Acceptable)                  \
  HTTP_XX(407, proxy_authentication_required,   Proxy Authentication Required)   \
  HTTP_XX(408, request_timeout,                 Request Timeout)                 \
  HTTP_XX(409, conflict,                        Conflict)                        \
  HTTP_XX(410, gone,                            Gone)                            \
  HTTP_XX(411, length_required,                 Length Required)                 \
  HTTP_XX(412, precondition_failed,             Precondition Failed)             \
  HTTP_XX(413, payload_too_large,               Payload Too Large)               \
  HTTP_XX(414, uri_too_long,                    URI Too Long)                    \
  HTTP_XX(415, unsupported_media_type,          Unsupported Media Type)          \
  HTTP_XX(416, range_not_satisfiable,           Range Not Satisfiable)           \
  HTTP_XX(417, expectation_failed,              Expectation Failed)              \
  HTTP_XX(421, misdirected_request,             Misdirected Request)             \
  HTTP_XX(422, unprocessable_entity,            Unprocessable Entity)            \
  HTTP_XX(423, locked,                          Locked)                          \
  HTTP_XX(424, failed_dependency,               Failed Dependency)               \
  HTTP_XX(426, upgrade_required,                Upgrade Required)                \
  HTTP_XX(428, precondition_required,           Precondition Required)           \
  HTTP_XX(429, too_many_requests,               Too Many Requests)               \
  HTTP_XX(431, request_header_fields_too_large, Request Header Fields Too Large) \
  HTTP_XX(451, unavailable_for_legal_reasons,   Unavailable For Legal Reasons)   \
  HTTP_XX(500, internal_server_error,           Internal Server Error)           \
  HTTP_XX(501, not_implemented,                 Not Implemented)                 \
  HTTP_XX(502, bad_gateway,                     Bad Gateway)                     \
  HTTP_XX(503, service_unavailable,             Service Unavailable)             \
  HTTP_XX(504, gateway_timeout,                 Gateway Timeout)                 \
  HTTP_XX(505, http_version_not_supported,      HTTP Version Not Supported)      \
  HTTP_XX(506, variant_also_negotiates,         Variant Also Negotiates)         \
  HTTP_XX(507, insufficient_storage,            Insufficient Storage)            \
  HTTP_XX(508, loop_detected,                   Loop Detected)                   \
  HTTP_XX(510, not_extended,                    Not Extended)                    \
  HTTP_XX(511, network_authentication_required, Network Authentication Required) \

enum class http_status
  {
// 20220214 Fix the conflict with the macro definition in <wininet.h>
// HTTP_STATUS_CONTINUE is also defined in <wininet.h>
#define HTTP_XX(num, name, string) name = num,
  HTTP_STATUS_MAP(HTTP_XX)
#undef HTTP_XX
  };


/* Request Methods */
#define HTTP_METHOD_MAP(HTTP_XX)         \
  HTTP_XX(0,  DELETE,      DELETE)       \
  HTTP_XX(1,  GET,         GET)          \
  HTTP_XX(2,  HEAD,        HEAD)         \
  HTTP_XX(3,  POST,        POST)         \
  HTTP_XX(4,  PUT,         PUT)          \
  /* pathological */                     \
  HTTP_XX(5,  CONNECT,     CONNECT)      \
  HTTP_XX(6,  OPTIONS,     OPTIONS)      \
  HTTP_XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                           \
  HTTP_XX(8,  COPY,        COPY)         \
  HTTP_XX(9,  LOCK,        LOCK)         \
  HTTP_XX(10, MKCOL,       MKCOL)        \
  HTTP_XX(11, MOVE,        MOVE)         \
  HTTP_XX(12, PROPFIND,    PROPFIND)     \
  HTTP_XX(13, PROPPATCH,   PROPPATCH)    \
  HTTP_XX(14, SEARCH,      SEARCH)       \
  HTTP_XX(15, UNLOCK,      UNLOCK)       \
  HTTP_XX(16, BIND,        BIND)         \
  HTTP_XX(17, REBIND,      REBIND)       \
  HTTP_XX(18, UNBIND,      UNBIND)       \
  HTTP_XX(19, ACL,         ACL)          \
  /* subversion */                       \
  HTTP_XX(20, REPORT,      REPORT)       \
  HTTP_XX(21, MKACTIVITY,  MKACTIVITY)   \
  HTTP_XX(22, CHECKOUT,    CHECKOUT)     \
  HTTP_XX(23, MERGE,       MERGE)        \
  /* upnp */                             \
  HTTP_XX(24, MSEARCH,     M-SEARCH)     \
  HTTP_XX(25, NOTIFY,      NOTIFY)       \
  HTTP_XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  HTTP_XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                         \
  HTTP_XX(28, PATCH,       PATCH)        \
  HTTP_XX(29, PURGE,       PURGE)        \
  /* CalDAV */                           \
  HTTP_XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */       \
  HTTP_XX(31, LINK,        LINK)         \
  HTTP_XX(32, UNLINK,      UNLINK)       \
  /* icecast */                          \
  HTTP_XX(33, SOURCE,      SOURCE)       \

enum class http_method
  {
#define HTTP_XX(num, name, string) HTTP_##name = num,
  HTTP_METHOD_MAP(HTTP_XX)
#undef HTTP_XX
  };


enum class http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH };


/* Flag values for http_parser.flags field */
enum class flags
  { F_CHUNKED               = 1 << 0
  , F_CONNECTION_KEEP_ALIVE = 1 << 1
  , F_CONNECTION_CLOSE      = 1 << 2
  , F_CONNECTION_UPGRADE    = 1 << 3
  , F_TRAILING              = 1 << 4
  , F_UPGRADE               = 1 << 5
  , F_SKIPBODY              = 1 << 6
  , F_CONTENTLENGTH         = 1 << 7
  };


/* Map for errno-related constants
 *
 * The provided argument should be a macro that takes 2 arguments.
 */
#define HTTP_ERRNO_MAP(HTTP_XX)                                           \
  /* No error */                                                          \
  HTTP_XX(OK, "success")                                                  \
                                                                          \
  /* Callback-related errors */                                           \
  HTTP_XX(CB_message_begin, "the on_message_begin callback failed")       \
  HTTP_XX(CB_url, "the on_url callback failed")                           \
  HTTP_XX(CB_header_field, "the on_header_field callback failed")         \
  HTTP_XX(CB_header_value, "the on_header_value callback failed")         \
  HTTP_XX(CB_headers_complete, "the on_headers_complete callback failed") \
  HTTP_XX(CB_body, "the on_body callback failed")                         \
  HTTP_XX(CB_message_complete, "the on_message_complete callback failed") \
  HTTP_XX(CB_status, "the on_status callback failed")                     \
  HTTP_XX(CB_chunk_header, "the on_chunk_header callback failed")         \
  HTTP_XX(CB_chunk_complete, "the on_chunk_complete callback failed")     \
                                                                          \
  /* Parsing-related errors */                                            \
  HTTP_XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  HTTP_XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                     \
  HTTP_XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")           \
  HTTP_XX(INVALID_VERSION, "invalid HTTP version")                        \
  HTTP_XX(INVALID_STATUS, "invalid HTTP status code")                     \
  HTTP_XX(INVALID_METHOD, "invalid HTTP method")                          \
  HTTP_XX(INVALID_URL, "invalid URL")                                     \
  HTTP_XX(INVALID_HOST, "invalid host")                                   \
  HTTP_XX(INVALID_PORT, "invalid port")                                   \
  HTTP_XX(INVALID_PATH, "invalid path")                                   \
  HTTP_XX(INVALID_QUERY_STRING, "invalid query string")                   \
  HTTP_XX(INVALID_FRAGMENT, "invalid fragment")                           \
  HTTP_XX(LF_EXPECTED, "LF character expected")                           \
  HTTP_XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  HTTP_XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                        \
  HTTP_XX(UNEXPECTED_CONTENT_LENGTH,                                      \
     "unexpected content-length header")                                  \
  HTTP_XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                            \
  HTTP_XX(INVALID_CONSTANT, "invalid constant string")                    \
  HTTP_XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  HTTP_XX(STRICT, "strict mode assertion failed")                         \
  HTTP_XX(PAUSED, "parser is paused")                                     \
  HTTP_XX(UNKNOWN, "an unknown error occurred")                           \
  HTTP_XX(INVALID_TRANSFER_ENCODING,                                      \
     "request has invalid transfer-encoding")                             \


/* Define HPE_* values for each errno value above */
#define HTTP_ERRNO_GEN(n, s) HPE_##n,
enum class http_errno {
  HTTP_ERRNO_MAP(HTTP_ERRNO_GEN)
};
#undef HTTP_ERRNO_GEN


/* Get an http_errno value from an http_parser */
#define HTTP_PARSER_ERRNO(p)            ((http_errno) (p)->http_errno)


struct http_parser {
  /** PRIVATE **/
  unsigned int type : 2;         /* enum http_parser_type */
  unsigned int flags : 8;       /* F_* values from 'flags' enum; semi-public */
  unsigned int state : 7;        /* enum state from http_parser.c */
  unsigned int header_state : 7; /* enum header_state from http_parser.c */
  unsigned int index : 5;        /* index into current matcher */
  unsigned int uses_transfer_encoding : 1; /* Transfer-Encoding header is present */
  unsigned int allow_chunked_length : 1; /* Allow headers with both
                                          * `Content-Length` and
                                          * `Transfer-Encoding: chunked` set */
  unsigned int lenient_http_headers : 1;

  uint32_t nread;          /* # bytes read in various scenarios */
  uint64_t content_length; /* # bytes in body. `(uint64_t) -1` (all bits one)
                            * if no Content-Length header.
                            */

  /** READ-ONLY **/
  unsigned short http_major;
  unsigned short http_minor;
  unsigned int status_code : 16; /* responses only */
  unsigned int method : 8;       /* requests only */
  unsigned int http_errno : 7;

  /* 1 = Upgrade header was present and the parser has exited because of that.
   * 0 = No upgrade header present.
   * Should be checked when http_parser_execute() returns in addition to
   * error checking.
   */
  unsigned int upgrade : 1;

  /** PUBLIC **/
  void *data; /* A pointer to get hook to the "connection" or "socket" object */
};


struct http_parser_settings {
  http_cb      on_message_begin;
  http_data_cb on_url;
  http_data_cb on_status;
  http_data_cb on_header_field;
  http_data_cb on_header_value;
  http_cb      on_headers_complete;
  http_data_cb on_body;
  http_cb      on_message_complete;
  /* When on_chunk_header is called, the current chunk length is stored
   * in parser->content_length.
   */
  http_cb      on_chunk_header;
  http_cb      on_chunk_complete;
};


enum class url_fields
  { UF_SCHEMA           = 0
  , UF_HOST             = 1
  , UF_PORT             = 2
  , UF_PATH             = 3
  , UF_QUERY            = 4
  , UF_FRAGMENT         = 5
  , UF_USERINFO         = 6
  , UF_MAX              = 7
  };


/* Result structure for http_parser_parse_url().
 *
 * Callers should index into field_data[] with UF_* values iff field_set
 * has the relevant (1 << UF_*) bit set. As a courtesy to clients (and
 * because we probably have padding left over), we convert any port to
 * a uint16_t.
 */
struct http_parser_url {
  uint16_t field_set;           /* Bitmask of (1 << UF_*) values */
  uint16_t port;                /* Converted UF_PORT string */

  struct {
    uint16_t off;               /* Offset into buffer in which field starts */
    uint16_t len;               /* Length of run in buffer */
  } field_data[(int)(url_fields::UF_MAX)];
};


/* Returns the library version. Bits 16-23 contain the major version number,
 * bits 8-15 the minor version number and bits 0-7 the patch level.
 * Usage example:
 *
 *   unsigned long version = http_parser_version();
 *   unsigned major = (version >> 16) & 255;
 *   unsigned minor = (version >> 8) & 255;
 *   unsigned patch = version & 255;
 *   printf("http_parser v%u.%u.%u\n", major, minor, patch);
 */
//unsigned long http_parser_version(void);

//void http_parser_init(http_parser *parser, http_parser_type type);


/* Initialize http_parser_settings members to 0
 */
//void http_parser_settings_init(http_parser_settings *settings);


/* Executes the parser. Returns number of parsed bytes. Sets
 * `parser->http_errno` on error. */
//size_t http_parser_execute(http_parser *parser,
//                           const http_parser_settings *settings,
//                           const char *data,
//                           size_t len);


/* If http_should_keep_alive() in the on_headers_complete or
 * on_message_complete callback returns 0, then this should be
 * the last message on the connection.
 * If you are the server, respond with the "Connection: close" header.
 * If you are the client, close the connection.
 */
//int http_should_keep_alive(const http_parser *parser);

/* Returns a string version of the HTTP method. */
//const char *http_method_str(http_method m);

/* Return a string name of the given error */
//const char *http_errno_name(http_errno err);

/* Return a string description of the given error */
//const char *http_errno_description(http_errno err);

/* Initialize all http_parser_url members to 0 */
//void http_parser_url_init(struct http_parser_url *u);

/* Parse a URL; return nonzero on failure */
//int http_parser_parse_url(const char *buf, size_t buflen,
//                          int is_connect,
//                          struct http_parser_url *u);

/* Pause or un-pause the parser; a nonzero value pauses */
//void http_parser_pause(http_parser *parser, int paused);

/* Checks if this is the final chunk of the body. */
//int http_body_is_final(const http_parser *parser);

// ---------------------------------------------------------------------------- http_parser.c ---------------------------------------------------------------------------- //
/* Based on src/http/ngx_http_parse.c from NGINX copyright Igor Sysoev
 *
 * Additional changes are licensed under the same terms as NGINX and
 * copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
//#include "http_parser.h"
#include <assert.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static uint32_t max_header_size = HTTP_MAX_HEADER_SIZE;

#ifndef HTTP_ULLONG_MAX
# define HTTP_ULLONG_MAX ((uint64_t) -1) /* 2^64-1 */
#endif

#ifndef HTTP_MIN
# define HTTP_MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef HTTP_ARRAY_SIZE
# define HTTP_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef HTTP_BIT_AT
# define HTTP_BIT_AT(a, i)                                           \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
   (1 << ((unsigned int) (i) & 7))))
#endif

#ifndef HTTP_ELEM_AT
# define HTTP_ELEM_AT(a, i, v) ((unsigned int) (i) < HTTP_ARRAY_SIZE(a) ? (a)[(unsigned int)(i)] : (v))
#endif

#define HTTP_SET_ERRNO(e)                                            \
do {                                                                 \
  parser->nread = nread;                                             \
  parser->http_errno = (unsigned int)(e);                            \
} while(0)

#define HTTP_CURRENT_STATE() p_state
#define HTTP_UPDATE_STATE(V) p_state = (state) (V);
#define HTTP_RETURN(V)                                               \
do {                                                                 \
  parser->nread = nread;                                             \
  parser->state = (unsigned int)(HTTP_CURRENT_STATE());              \
  return (V);                                                        \
} while (0);
#define HTTP_REEXECUTE()                                             \
  goto reexecute;                                                    \


#ifdef __GNUC__
# define HTTP_LIKELY(X) __builtin_expect(!!(X), 1)
# define HTTP_UNLIKELY(X) __builtin_expect(!!(X), 0)
#else
# define HTTP_LIKELY(X) (X)
# define HTTP_UNLIKELY(X) (X)
#endif


/* Run the notify callback FOR, returning ER if it fails */
#define HTTP_CALLBACK_NOTIFY_(FOR, ER)                               \
do {                                                                 \
  assert(HTTP_PARSER_ERRNO(parser) == http_errno::HPE_OK);           \
                                                                     \
  if (HTTP_LIKELY(settings->on_##FOR)) {                             \
    parser->state = (unsigned int)(HTTP_CURRENT_STATE());            \
    if (HTTP_UNLIKELY(0 != settings->on_##FOR(parser, user_data))) { \
      HTTP_SET_ERRNO(http_errno::HPE_CB_##FOR);                      \
    }                                                                \
    HTTP_UPDATE_STATE(parser->state);                                \
                                                                     \
    /* We either errored above or got paused; get out */             \
    if (HTTP_UNLIKELY(HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK)) {             \
      return (ER);                                                   \
    }                                                                \
  }                                                                  \
} while (0)

/* Run the notify callback FOR and consume the current byte */
#define HTTP_CALLBACK_NOTIFY(FOR)            HTTP_CALLBACK_NOTIFY_(FOR, p - data + 1)

/* Run the notify callback FOR and don't consume the current byte */
#define HTTP_CALLBACK_NOTIFY_NOADVANCE(FOR)  HTTP_CALLBACK_NOTIFY_(FOR, p - data)

/* Run data callback FOR with LEN bytes, returning ER if it fails */
#define HTTP_CALLBACK_DATA_(FOR, LEN, ER)                            \
do {                                                                 \
  assert(HTTP_PARSER_ERRNO(parser) == http_errno::HPE_OK);           \
                                                                     \
  if (FOR##_mark) {                                                  \
    if (HTTP_LIKELY(settings->on_##FOR)) {                           \
      parser->state = (unsigned int)(HTTP_CURRENT_STATE());          \
      if (HTTP_UNLIKELY(0 !=                                         \
        settings->on_##FOR(parser, FOR##_mark, (LEN), user_data))) { \
        HTTP_SET_ERRNO(http_errno::HPE_CB_##FOR);                    \
      }                                                              \
      HTTP_UPDATE_STATE(parser->state);                              \
                                                                     \
      /* We either errored above or got paused; get out */           \
      if (HTTP_UNLIKELY(HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK)) {           \
        return (ER);                                                 \
      }                                                              \
    }                                                                \
    FOR##_mark = NULL;                                               \
  }                                                                  \
} while (0)

/* Run the data callback FOR and consume the current byte */
#define HTTP_CALLBACK_DATA(FOR)                                      \
    HTTP_CALLBACK_DATA_(FOR, p - FOR##_mark, p - data + 1)

/* Run the data callback FOR and don't consume the current byte */
#define HTTP_CALLBACK_DATA_NOADVANCE(FOR)                            \
    HTTP_CALLBACK_DATA_(FOR, p - FOR##_mark, p - data)

/* Set the mark FOR; non-destructive if mark is already set */
#define HTTP_MARK(FOR)                                               \
do {                                                                 \
  if (!FOR##_mark) {                                                 \
    FOR##_mark = p;                                                  \
  }                                                                  \
} while (0)

/* Don't allow the total size of the HTTP headers (including the status
 * line) to exceed max_header_size.  This check is here to protect
 * embedders against denial-of-service attacks where the attacker feeds
 * us a never-ending header that the embedder keeps buffering.
 *
 * This check is arguably the responsibility of embedders but we're doing
 * it on the embedder's behalf because most won't bother and this way we
 * make the web a little safer.  max_header_size is still far bigger
 * than any reasonable request or response so this should never affect
 * day-to-day operation.
 */
#define HTTP_COUNT_HEADER_SIZE(V)                                    \
do {                                                                 \
  nread += (uint32_t)(V);                                            \
  if (HTTP_UNLIKELY(nread > max_header_size)) {                      \
    HTTP_SET_ERRNO(http_errno::HPE_HEADER_OVERFLOW);                 \
    goto error;                                                      \
  }                                                                  \
} while (0)


#define HTTP_PROXY_CONNECTION "proxy-connection"
#define HTTP_CONNECTION "connection"
#define HTTP_CONTENT_LENGTH "content-length"
#define HTTP_TRANSFER_ENCODING "transfer-encoding"
#define HTTP_UPGRADE "upgrade"
#define HTTP_CHUNKED "chunked"
#define HTTP_KEEP_ALIVE "keep-alive"
#define HTTP_CLOSE "close"


static const char *method_strings[] =
  {
#define HTTP_XX(num, name, string) #string,
  HTTP_METHOD_MAP(HTTP_XX)
#undef HTTP_XX
  };


/* Tokens as defined by rfc 2616. Also lowercases them.
 *        token       = 1*<any CHAR except CTLs or separators>
 *     separators     = "(" | ")" | "<" | ">" | "@"
 *                    | "," | ";" | ":" | "\" | <">
 *                    | "/" | "[" | "]" | "?" | "="
 *                    | "{" | "}" | SP | HT
 */
static const char tokens[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
       ' ',     '!',      0,      '#',     '$',     '%',     '&',    '\'',
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        0,       0,      '*',     '+',      0,      '-',     '.',      0,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
       '0',     '1',     '2',     '3',     '4',     '5',     '6',     '7',
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
       '8',     '9',      0,       0,       0,       0,       0,       0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        0,      'a',     'b',     'c',     'd',     'e',     'f',     'g',
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
       'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
       'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
       'x',     'y',     'z',      0,       0,       0,      '^',     '_',
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
       '`',     'a',     'b',     'c',     'd',     'e',     'f',     'g',
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
       'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
       'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
       'x',     'y',     'z',      0,      '|',      0,      '~',       0 };


static const int8_t unhex[256] =
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  , 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1
  ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  };


#if HTTP_PARSER_STRICT
# define T(v) 0
#else
# define T(v) v
#endif


static const uint8_t normal_url_char[32] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
        0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
        0    | T(2)   |   0    |   0    | T(16)  |   0    |   0    |   0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
        0    |   2    |   4    |   0    |   16   |   32   |   64   |  128,
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |   0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |   0, };

#undef T

enum class state
  { s_dead = 1 /* important that this is > 0 */

  , s_start_req_or_res
  , s_res_or_resp_H
  , s_start_res
  , s_res_H
  , s_res_HT
  , s_res_HTT
  , s_res_HTTP
  , s_res_http_major
  , s_res_http_dot
  , s_res_http_minor
  , s_res_http_end
  , s_res_first_status_code
  , s_res_status_code
  , s_res_status_start
  , s_res_status
  , s_res_line_almost_done

  , s_start_req

  , s_req_method
  , s_req_spaces_before_url
  , s_req_schema
  , s_req_schema_slash
  , s_req_schema_slash_slash
  , s_req_server_start
  , s_req_server
  , s_req_server_with_at
  , s_req_path
  , s_req_query_string_start
  , s_req_query_string
  , s_req_fragment_start
  , s_req_fragment
  , s_req_http_start
  , s_req_http_H
  , s_req_http_HT
  , s_req_http_HTT
  , s_req_http_HTTP
  , s_req_http_I
  , s_req_http_IC
  , s_req_http_major
  , s_req_http_dot
  , s_req_http_minor
  , s_req_http_end
  , s_req_line_almost_done

  , s_header_field_start
  , s_header_field
  , s_header_value_discard_ws
  , s_header_value_discard_ws_almost_done
  , s_header_value_discard_lws
  , s_header_value_start
  , s_header_value
  , s_header_value_lws

  , s_header_almost_done

  , s_chunk_size_start
  , s_chunk_size
  , s_chunk_parameters
  , s_chunk_size_almost_done

  , s_headers_almost_done
  , s_headers_done

  /* Important: 's_headers_done' must be the last 'header' state. All
   * states beyond this must be 'body' states. It is used for overflow
   * checking. See the HTTP_PARSING_HEADER() macro.
   */

  , s_chunk_data
  , s_chunk_data_almost_done
  , s_chunk_data_done

  , s_body_identity
  , s_body_identity_eof

  , s_message_done
  };


#define HTTP_PARSING_HEADER(_state) (_state <= state::s_headers_done)


enum class header_states
  { h_general = 0
  , h_C
  , h_CO
  , h_CON

  , h_matching_connection
  , h_matching_proxy_connection
  , h_matching_content_length
  , h_matching_transfer_encoding
  , h_matching_upgrade

  , h_connection
  , h_content_length
  , h_content_length_num
  , h_content_length_ws
  , h_transfer_encoding
  , h_upgrade

  , h_matching_transfer_encoding_token_start
  , h_matching_transfer_encoding_chunked
  , h_matching_transfer_encoding_token

  , h_matching_connection_token_start
  , h_matching_connection_keep_alive
  , h_matching_connection_close
  , h_matching_connection_upgrade
  , h_matching_connection_token

  , h_transfer_encoding_chunked
  , h_connection_keep_alive
  , h_connection_close
  , h_connection_upgrade
  };

enum class http_host_state
  {
    s_http_host_dead = 1
  , s_http_userinfo_start
  , s_http_userinfo
  , s_http_host_start
  , s_http_host_v6_start
  , s_http_host
  , s_http_host_v6
  , s_http_host_v6_end
  , s_http_host_v6_zone_start
  , s_http_host_v6_zone
  , s_http_host_port_start
  , s_http_host_port
};

/* Macros for character classes; depends on strict-mode  */
#define HTTP_CR                  '\r'
#define HTTP_LF                  '\n'
#define HTTP_LOWER(c)            (unsigned char)(c | 0x20)
#define HTTP_IS_ALPHA(c)         (HTTP_LOWER(c) >= 'a' && HTTP_LOWER(c) <= 'z')
#define HTTP_IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#define HTTP_IS_ALPHANUM(c)      (HTTP_IS_ALPHA(c) || HTTP_IS_NUM(c))
#define HTTP_IS_HEX(c)           (HTTP_IS_NUM(c) || (HTTP_LOWER(c) >= 'a' && HTTP_LOWER(c) <= 'f'))
#define HTTP_IS_MARK(c)          ((c) == '-' || (c) == '_' || (c) == '.' || \
  (c) == '!' || (c) == '~' || (c) == '*' || (c) == '\'' || (c) == '(' || \
  (c) == ')')
#define HTTP_IS_USERINFO_CHAR(c) (HTTP_IS_ALPHANUM(c) || HTTP_IS_MARK(c) || (c) == '%' || \
  (c) == ';' || (c) == ':' || (c) == '&' || (c) == '=' || (c) == '+' || \
  (c) == '$' || (c) == ',')

#define HTTP_STRICT_TOKEN(c)     ((c == ' ') ? 0 : tokens[(unsigned char)c])

#if HTTP_PARSER_STRICT
#define HTTP_TOKEN(c)            HTTP_STRICT_TOKEN(c)
#define HTTP_IS_URL_CHAR(c)      (HTTP_BIT_AT(normal_url_char, (unsigned char)c))
#define HTTP_IS_HOST_CHAR(c)     (HTTP_IS_ALPHANUM(c) || (c) == '.' || (c) == '-')
#else
#define HTTP_TOKEN(c)            tokens[(unsigned char)c]
#define HTTP_IS_URL_CHAR(c)                                                         \
  (HTTP_BIT_AT(normal_url_char, (unsigned char)c) || ((c) & 0x80))
#define HTTP_IS_HOST_CHAR(c)                                                        \
  (HTTP_IS_ALPHANUM(c) || (c) == '.' || (c) == '-' || (c) == '_')
#endif

/**
 * Verify that a char is a valid visible (printable) US-ASCII
 * character or %x80-FF
 **/
#define HTTP_IS_HEADER_CHAR(ch)                                                     \
  (ch == HTTP_CR || ch == HTTP_LF || ch == 9 || ((unsigned char)ch > 31 && ch != 127))

#define start_state (parser->type == (unsigned int)(http_parser_type::HTTP_REQUEST) ? state::s_start_req : state::s_start_res)


#if HTTP_PARSER_STRICT
# define HTTP_STRICT_CHECK(cond)                                     \
do {                                                                 \
  if (cond) {                                                        \
    HTTP_SET_ERRNO(http_errno::HPE_STRICT);                          \
    goto error;                                                      \
  }                                                                  \
} while (0)
# define HTTP_NEW_MESSAGE() (http_should_keep_alive(parser) ? start_state : state::s_dead)
#else
# define HTTP_STRICT_CHECK(cond)
# define HTTP_NEW_MESSAGE() start_state
#endif


/* Map errno values to strings for human-readable output */
#define HTTP_STRERROR_GEN(n, s) { "HPE_" #n, s },
static struct {
  const char *name;
  const char *description;
} http_strerror_tab[] = {
  HTTP_ERRNO_MAP(HTTP_STRERROR_GEN)
};
#undef HTTP_STRERROR_GEN

//int http_message_needs_eof(const http_parser *parser);

/* Does the parser need to see an EOF to find the end of the message? */
template<typename = void>
int
http_message_needs_eof (const http_parser *parser)
{
  if (parser->type == (unsigned int)(http_parser_type::HTTP_REQUEST)) {
    return 0;
  }

  /* See RFC 2616 section 4.4 */
  if (parser->status_code / 100 == 1 || /* 1xx e.g. Continue */
      parser->status_code == 204 ||     /* No Content */
      parser->status_code == 304 ||     /* Not Modified */
      parser->flags & (unsigned int)(flags::F_SKIPBODY)) {     /* response to a HEAD request */
    return 0;
  }

  /* RFC 7230 3.3.3, see `s_headers_almost_done` */
  if ((parser->uses_transfer_encoding == 1) &&
      (parser->flags & (unsigned int)(flags::F_CHUNKED)) == 0) {
    return 1;
  }

  if ((parser->flags & (unsigned int)(flags::F_CHUNKED)) || parser->content_length != HTTP_ULLONG_MAX) {
    return 0;
  }

  return 1;
}

template<typename = void>
int
http_should_keep_alive (const http_parser *parser)
{
  if (parser->http_major > 0 && parser->http_minor > 0) {
    /* HTTP/1.1 */
    if (parser->flags & (unsigned int)(flags::F_CONNECTION_CLOSE)) {
      return 0;
    }
  } else {
    /* HTTP/1.0 or earlier */
    if (!(parser->flags & (unsigned int)(flags::F_CONNECTION_KEEP_ALIVE))) {
      return 0;
    }
  }

  return !http_message_needs_eof(parser);
}


/* Our URL parser.
 *
 * This is designed to be shared by http_parser_execute() for URL validation,
 * hence it has a state transition + byte-for-byte interface. In addition, it
 * is meant to be embedded in http_parser_parse_url(), which does the dirty
 * work of turning state transitions URL components for its API.
 *
 * This function should only be invoked with non-space characters. It is
 * assumed that the caller cares about (and can detect) the transition between
 * URL and non-URL states by looking for these.
 */
template<typename = void>
static state
parse_url_char(state s, const char ch)
{
//  if (ch == ' ' || ch == '\r' || ch == '\n') {
//    return state::s_dead;
//  }
//
//#if HTTP_PARSER_STRICT
//  if (ch == '\t' || ch == '\f') {
//    return state::s_dead;
//  }
//#endif

  switch (s) {
    case state::s_req_spaces_before_url:
      /* Proxied requests are followed by scheme of an absolute URI (alpha).
       * All methods except CONNECT are followed by '/' or '*'.
       */

      if (ch == '/' || ch == '*') {
        return state::s_req_path;
      }

      if (HTTP_IS_ALPHA(ch)) {
        return state::s_req_schema;
      }

      break;

    case state::s_req_schema:
      if (HTTP_IS_ALPHA(ch)) {
        return s;
      }

      if (ch == ':') {
        return state::s_req_schema_slash;
      }

      break;

    case state::s_req_schema_slash:
      if (ch == '/') {
        return state::s_req_schema_slash_slash;
      }

      break;

    case state::s_req_schema_slash_slash:
      if (ch == '/') {
        return state::s_req_server_start;
      }

      break;

    case state::s_req_server_with_at:
      if (ch == '@') {
        return state::s_dead;
      }

    /* fall through */
    case state::s_req_server_start:
    case state::s_req_server:
      if (ch == '/') {
        return state::s_req_path;
      }

      if (ch == '?') {
        return state::s_req_query_string_start;
      }

      if (ch == '@') {
        return state::s_req_server_with_at;
      }

      if (HTTP_IS_USERINFO_CHAR(ch) || ch == '[' || ch == ']') {
        return state::s_req_server;
      }

      break;

    case state::s_req_path:
      if (HTTP_IS_URL_CHAR(ch)) {
        return s;
      }

      switch (ch) {
        case '?':
          return state::s_req_query_string_start;

        case '#':
          return state::s_req_fragment_start;

		default:
          return s;
      }

      break;

    case state::s_req_query_string_start:
    case state::s_req_query_string:
      if (HTTP_IS_URL_CHAR(ch)) {
        return state::s_req_query_string;
      }

      switch (ch) {
        case '?':
          /* allow extra '?' in query string */
          return state::s_req_query_string;

        case '#':
          return state::s_req_fragment_start;

		default:
          return state::s_req_query_string;
      }

      break;

    case state::s_req_fragment_start:
      if (HTTP_IS_URL_CHAR(ch)) {
        return state::s_req_fragment;
      }

      switch (ch) {
        case '?':
          return state::s_req_fragment;

        case '#':
          return s;

		default:
          return state::s_req_fragment;
      }

      break;

    case state::s_req_fragment:
      if (HTTP_IS_URL_CHAR(ch)) {
        return s;
      }

      switch (ch) {
        case '?':
        case '#':
          return s;
		default:
          return s;
      }

      break;

    default:
      break;
  }

  /* We should never fall out of the switch above unless there's an error */
  return state::s_dead;
}

template<typename = void>
size_t http_parser_execute (http_parser *parser,
                            const http_parser_settings *settings,
                            const char *data,
                            size_t len,
	                        void * user_data = nullptr
	                        )
{
  char c, ch;
  int8_t unhex_val;
  const char *p = data;
  const char *header_field_mark = 0;
  const char *header_value_mark = 0;
  const char *url_mark = 0;
  const char *body_mark = 0;
  const char *status_mark = 0;
  state p_state = (state) parser->state;
  const unsigned int lenient = parser->lenient_http_headers;
  const unsigned int allow_chunked_length = parser->allow_chunked_length;

  uint32_t nread = parser->nread;

  /* We're in an error state. Don't bother doing anything. */
  if (HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK) {
    return 0;
  }

  if (len == 0) {
    switch (HTTP_CURRENT_STATE()) {
      case state::s_body_identity_eof:
        /* Use of HTTP_CALLBACK_NOTIFY() here would erroneously return 1 byte read if
         * we got paused.
         */
        HTTP_CALLBACK_NOTIFY_NOADVANCE(message_complete);
        return 0;

      case state::s_dead:
      case state::s_start_req_or_res:
      case state::s_start_res:
      case state::s_start_req:
        return 0;

      default:
        HTTP_SET_ERRNO(http_errno::HPE_INVALID_EOF_STATE);
        return 1;
    }
  }


  if (HTTP_CURRENT_STATE() == state::s_header_field)
    header_field_mark = data;
  if (HTTP_CURRENT_STATE() == state::s_header_value)
    header_value_mark = data;
  switch (HTTP_CURRENT_STATE()) {
  case state::s_req_path:
  case state::s_req_schema:
  case state::s_req_schema_slash:
  case state::s_req_schema_slash_slash:
  case state::s_req_server_start:
  case state::s_req_server:
  case state::s_req_server_with_at:
  case state::s_req_query_string_start:
  case state::s_req_query_string:
  case state::s_req_fragment_start:
  case state::s_req_fragment:
    url_mark = data;
    break;
  case state::s_res_status:
    status_mark = data;
    break;
  default:
    break;
  }

  for (p=data; p != data + len; ++p) {
    ch = *p;

    if (HTTP_PARSING_HEADER(HTTP_CURRENT_STATE()))
      HTTP_COUNT_HEADER_SIZE(1);

reexecute:
    switch (HTTP_CURRENT_STATE()) {

      case state::s_dead:
        /* this state is used after a 'Connection: close' message
         * the parser will error out if it reads another message
         */
        if (HTTP_LIKELY(ch == HTTP_CR || ch == HTTP_LF))
          break;

        HTTP_SET_ERRNO(http_errno::HPE_CLOSED_CONNECTION);
        goto error;

      case state::s_start_req_or_res:
      {
        if (ch == HTTP_CR || ch == HTTP_LF)
          break;
        parser->flags = (unsigned int)(0);
        parser->uses_transfer_encoding = 0;
        parser->content_length = HTTP_ULLONG_MAX;

        if (ch == 'H') {
          HTTP_UPDATE_STATE(state::s_res_or_resp_H);

          HTTP_CALLBACK_NOTIFY(message_begin);
        } else {
          parser->type = (unsigned int)(http_parser_type::HTTP_REQUEST);
          HTTP_UPDATE_STATE(state::s_start_req);
          HTTP_REEXECUTE();
        }

        break;
      }

      case state::s_res_or_resp_H:
        if (ch == 'T') {
          parser->type = (unsigned int)(http_parser_type::HTTP_RESPONSE);
          HTTP_UPDATE_STATE(state::s_res_HT);
        } else {
          if (HTTP_UNLIKELY(ch != 'E')) {
            HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONSTANT);
            goto error;
          }

          parser->type = (unsigned int)(http_parser_type::HTTP_REQUEST);
          parser->method = (unsigned int)(http_method::HTTP_HEAD);
          parser->index = 2;
          HTTP_UPDATE_STATE(state::s_req_method);
        }
        break;

      case state::s_start_res:
      {
        if (ch == HTTP_CR || ch == HTTP_LF)
          break;
        parser->flags = (unsigned int)(0);
        parser->uses_transfer_encoding = 0;
        parser->content_length = HTTP_ULLONG_MAX;

        if (ch == 'H') {
          HTTP_UPDATE_STATE(state::s_res_H);
        } else {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONSTANT);
          goto error;
        }

        HTTP_CALLBACK_NOTIFY(message_begin);
        break;
      }

      case state::s_res_H:
        HTTP_STRICT_CHECK(ch != 'T');
        HTTP_UPDATE_STATE(state::s_res_HT);
        break;

      case state::s_res_HT:
        HTTP_STRICT_CHECK(ch != 'T');
        HTTP_UPDATE_STATE(state::s_res_HTT);
        break;

      case state::s_res_HTT:
        HTTP_STRICT_CHECK(ch != 'P');
        HTTP_UPDATE_STATE(state::s_res_HTTP);
        break;

      case state::s_res_HTTP:
        HTTP_STRICT_CHECK(ch != '/');
        HTTP_UPDATE_STATE(state::s_res_http_major);
        break;

      case state::s_res_http_major:
        if (HTTP_UNLIKELY(!HTTP_IS_NUM(ch))) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_major = ch - '0';
        HTTP_UPDATE_STATE(state::s_res_http_dot);
        break;

      case state::s_res_http_dot:
      {
        if (HTTP_UNLIKELY(ch != '.')) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        HTTP_UPDATE_STATE(state::s_res_http_minor);
        break;
      }

      case state::s_res_http_minor:
        if (HTTP_UNLIKELY(!HTTP_IS_NUM(ch))) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_minor = ch - '0';
        HTTP_UPDATE_STATE(state::s_res_http_end);
        break;

      case state::s_res_http_end:
      {
        if (HTTP_UNLIKELY(ch != ' ')) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        HTTP_UPDATE_STATE(state::s_res_first_status_code);
        break;
      }

      case state::s_res_first_status_code:
      {
        if (!HTTP_IS_NUM(ch)) {
          if (ch == ' ') {
            break;
          }

          HTTP_SET_ERRNO(http_errno::HPE_INVALID_STATUS);
          goto error;
        }
        parser->status_code = ch - '0';
        HTTP_UPDATE_STATE(state::s_res_status_code);
        break;
      }

      case state::s_res_status_code:
      {
        if (!HTTP_IS_NUM(ch)) {
          switch (ch) {
            case ' ':
              HTTP_UPDATE_STATE(state::s_res_status_start);
              break;
            case HTTP_CR:
            case HTTP_LF:
              HTTP_UPDATE_STATE(state::s_res_status_start);
              HTTP_REEXECUTE();
              break;
            default:
              HTTP_SET_ERRNO(http_errno::HPE_INVALID_STATUS);
              goto error;
          }
          break;
        }

        parser->status_code *= 10;
        parser->status_code += ch - '0';

        if (HTTP_UNLIKELY(parser->status_code > 999)) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_STATUS);
          goto error;
        }

        break;
      }

      case state::s_res_status_start:
      {
        HTTP_MARK(status);
        HTTP_UPDATE_STATE(state::s_res_status);
        parser->index = 0;

        if (ch == HTTP_CR || ch == HTTP_LF)
          HTTP_REEXECUTE();

        break;
      }

      case state::s_res_status:
        if (ch == HTTP_CR) {
          HTTP_UPDATE_STATE(state::s_res_line_almost_done);
          HTTP_CALLBACK_DATA(status);
          break;
        }

        if (ch == HTTP_LF) {
          HTTP_UPDATE_STATE(state::s_header_field_start);
          HTTP_CALLBACK_DATA(status);
          break;
        }

        break;

      case state::s_res_line_almost_done:
        HTTP_STRICT_CHECK(ch != HTTP_LF);
        HTTP_UPDATE_STATE(state::s_header_field_start);
        break;

      case state::s_start_req:
      {
        if (ch == HTTP_CR || ch == HTTP_LF)
          break;
        parser->flags = (unsigned int)(0);
        parser->uses_transfer_encoding = 0;
        parser->content_length = HTTP_ULLONG_MAX;

        if (HTTP_UNLIKELY(!HTTP_IS_ALPHA(ch))) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_METHOD);
          goto error;
        }

        parser->method = (unsigned int)(0);
        parser->index = 1;
        switch (ch) {
          case 'A': parser->method = (unsigned int)(http_method::HTTP_ACL    ); break;
          case 'B': parser->method = (unsigned int)(http_method::HTTP_BIND   ); break;
          case 'C': parser->method = (unsigned int)(http_method::HTTP_CONNECT); /* or COPY, CHECKOUT */ break;
          case 'D': parser->method = (unsigned int)(http_method::HTTP_DELETE ); break;
          case 'G': parser->method = (unsigned int)(http_method::HTTP_GET    ); break;
          case 'H': parser->method = (unsigned int)(http_method::HTTP_HEAD   ); break;
          case 'L': parser->method = (unsigned int)(http_method::HTTP_LOCK   ); /* or LINK */ break;
          case 'M': parser->method = (unsigned int)(http_method::HTTP_MKCOL  ); /* or MOVE, MKACTIVITY, MERGE, M-SEARCH, MKCALENDAR */ break;
          case 'N': parser->method = (unsigned int)(http_method::HTTP_NOTIFY ); break;
          case 'O': parser->method = (unsigned int)(http_method::HTTP_OPTIONS); break;
          case 'P': parser->method = (unsigned int)(http_method::HTTP_POST   );
            /* or PROPFIND|PROPPATCH|PUT|PATCH|PURGE */
            break;
          case 'R': parser->method = (unsigned int)(http_method::HTTP_REPORT   ); /* or REBIND */ break;
          case 'S': parser->method = (unsigned int)(http_method::HTTP_SUBSCRIBE); /* or SEARCH, SOURCE */ break;
          case 'T': parser->method = (unsigned int)(http_method::HTTP_TRACE    ); break;
          case 'U': parser->method = (unsigned int)(http_method::HTTP_UNLOCK   ); /* or UNSUBSCRIBE, UNBIND, UNLINK */ break;
          default:
            HTTP_SET_ERRNO(http_errno::HPE_INVALID_METHOD);
            goto error;
        }
        HTTP_UPDATE_STATE(state::s_req_method);

        HTTP_CALLBACK_NOTIFY(message_begin);

        break;
      }

      case state::s_req_method:
      {
        const char *matcher;
        if (HTTP_UNLIKELY(ch == '\0')) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_METHOD);
          goto error;
        }

        matcher = method_strings[parser->method];
        if (ch == ' ' && matcher[parser->index] == '\0') {
          HTTP_UPDATE_STATE(state::s_req_spaces_before_url);
        } else if (ch == matcher[parser->index]) {
          ; /* nada */
        } else if ((ch >= 'A' && ch <= 'Z') || ch == '-') {

          switch (parser->method << 16 | parser->index << 8 | ch) {
#define HTTP_XX(meth, pos, ch, new_meth) \
            case ((size_t)http_method::HTTP_##meth << 16 | pos << 8 | ch): \
              parser->method = (unsigned int)(http_method::HTTP_##new_meth); break;

            HTTP_XX(POST,      1, 'U', PUT)
            HTTP_XX(POST,      1, 'A', PATCH)
            HTTP_XX(POST,      1, 'R', PROPFIND)
            HTTP_XX(PUT,       2, 'R', PURGE)
            HTTP_XX(CONNECT,   1, 'H', CHECKOUT)
            HTTP_XX(CONNECT,   2, 'P', COPY)
            HTTP_XX(MKCOL,     1, 'O', MOVE)
            HTTP_XX(MKCOL,     1, 'E', MERGE)
            HTTP_XX(MKCOL,     1, '-', MSEARCH)
            HTTP_XX(MKCOL,     2, 'A', MKACTIVITY)
            HTTP_XX(MKCOL,     3, 'A', MKCALENDAR)
            HTTP_XX(SUBSCRIBE, 1, 'E', SEARCH)
            HTTP_XX(SUBSCRIBE, 1, 'O', SOURCE)
            HTTP_XX(REPORT,    2, 'B', REBIND)
            HTTP_XX(PROPFIND,  4, 'P', PROPPATCH)
            HTTP_XX(LOCK,      1, 'I', LINK)
            HTTP_XX(UNLOCK,    2, 'S', UNSUBSCRIBE)
            HTTP_XX(UNLOCK,    2, 'B', UNBIND)
            HTTP_XX(UNLOCK,    3, 'I', UNLINK)
#undef HTTP_XX
            default:
              HTTP_SET_ERRNO(http_errno::HPE_INVALID_METHOD);
              goto error;
          }
        } else {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_METHOD);
          goto error;
        }

        ++parser->index;
        break;
      }

      case state::s_req_spaces_before_url:
      {
        if (ch == ' ') break;

        HTTP_MARK(url);
        if (parser->method == (unsigned int)(http_method::HTTP_CONNECT)) {
          HTTP_UPDATE_STATE(state::s_req_server_start);
        }

        HTTP_UPDATE_STATE(parse_url_char(HTTP_CURRENT_STATE(), ch));
        if (HTTP_UNLIKELY(HTTP_CURRENT_STATE() == state::s_dead)) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_URL);
          goto error;
        }

        break;
      }

      case state::s_req_schema:
      case state::s_req_schema_slash:
      case state::s_req_schema_slash_slash:
      case state::s_req_server_start:
      {
        switch (ch) {
          /* No whitespace allowed here */
          case ' ':
          case HTTP_CR:
          case HTTP_LF:
            HTTP_SET_ERRNO(http_errno::HPE_INVALID_URL);
            goto error;
          default:
            HTTP_UPDATE_STATE(parse_url_char(HTTP_CURRENT_STATE(), ch));
            if (HTTP_UNLIKELY(HTTP_CURRENT_STATE() == state::s_dead)) {
              HTTP_SET_ERRNO(http_errno::HPE_INVALID_URL);
              goto error;
            }
        }

        break;
      }

      case state::s_req_server:
      case state::s_req_server_with_at:
      case state::s_req_path:
      case state::s_req_query_string_start:
      case state::s_req_query_string:
      case state::s_req_fragment_start:
      case state::s_req_fragment:
      {
        switch (ch) {
          case ' ':
            HTTP_UPDATE_STATE(state::s_req_http_start);
            HTTP_CALLBACK_DATA(url);
            break;
          case HTTP_CR:
          case HTTP_LF:
            parser->http_major = 0;
            parser->http_minor = 9;
            HTTP_UPDATE_STATE((ch == HTTP_CR) ?
              state::s_req_line_almost_done :
              state::s_header_field_start);
            HTTP_CALLBACK_DATA(url);
            break;
          default:
            HTTP_UPDATE_STATE(parse_url_char(HTTP_CURRENT_STATE(), ch));
            if (HTTP_UNLIKELY(HTTP_CURRENT_STATE() == state::s_dead)) {
              HTTP_SET_ERRNO(http_errno::HPE_INVALID_URL);
              goto error;
            }
        }
        break;
      }

      case state::s_req_http_start:
        switch (ch) {
          case ' ':
            break;
          case 'H':
            HTTP_UPDATE_STATE(state::s_req_http_H);
            break;
          case 'I':
            if (parser->method == (unsigned int)(http_method::HTTP_SOURCE)) {
              HTTP_UPDATE_STATE(state::s_req_http_I);
              break;
            }
            /* fall through */
          default:
            HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONSTANT);
            goto error;
        }
        break;

      case state::s_req_http_H:
        HTTP_STRICT_CHECK(ch != 'T');
        HTTP_UPDATE_STATE(state::s_req_http_HT);
        break;

      case state::s_req_http_HT:
        HTTP_STRICT_CHECK(ch != 'T');
        HTTP_UPDATE_STATE(state::s_req_http_HTT);
        break;

      case state::s_req_http_HTT:
        HTTP_STRICT_CHECK(ch != 'P');
		HTTP_UPDATE_STATE(state::s_req_http_HTTP);
        break;

      case state::s_req_http_I:
        HTTP_STRICT_CHECK(ch != 'C');
		HTTP_UPDATE_STATE(state::s_req_http_IC);
        break;

      case state::s_req_http_IC:
        HTTP_STRICT_CHECK(ch != 'E');
        HTTP_UPDATE_STATE(state::s_req_http_HTTP);  /* Treat "ICE" as "HTTP". */
        break;

      case state::s_req_http_HTTP:
        HTTP_STRICT_CHECK(ch != '/');
        HTTP_UPDATE_STATE(state::s_req_http_major);
        break;

      case state::s_req_http_major:
        if (HTTP_UNLIKELY(!HTTP_IS_NUM(ch))) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_major = ch - '0';
        HTTP_UPDATE_STATE(state::s_req_http_dot);
        break;

      case state::s_req_http_dot:
      {
        if (HTTP_UNLIKELY(ch != '.')) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        HTTP_UPDATE_STATE(state::s_req_http_minor);
        break;
      }

      case state::s_req_http_minor:
        if (HTTP_UNLIKELY(!HTTP_IS_NUM(ch))) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_minor = ch - '0';
        HTTP_UPDATE_STATE(state::s_req_http_end);
        break;

      case state::s_req_http_end:
      {
        if (ch == HTTP_CR) {
          HTTP_UPDATE_STATE(state::s_req_line_almost_done);
          break;
        }

        if (ch == HTTP_LF) {
          HTTP_UPDATE_STATE(state::s_header_field_start);
          break;
        }

        HTTP_SET_ERRNO(http_errno::HPE_INVALID_VERSION);
        goto error;
        break;
      }

      /* end of request line */
      case state::s_req_line_almost_done:
      {
        if (HTTP_UNLIKELY(ch != HTTP_LF)) {
          HTTP_SET_ERRNO(http_errno::HPE_LF_EXPECTED);
          goto error;
        }

        HTTP_UPDATE_STATE(state::s_header_field_start);
        break;
      }

      case state::s_header_field_start:
      {
        if (ch == HTTP_CR) {
          HTTP_UPDATE_STATE(state::s_headers_almost_done);
          break;
        }

        if (ch == HTTP_LF) {
          /* they might be just sending \n instead of \r\n so this would be
           * the second \n to denote the end of headers*/
          HTTP_UPDATE_STATE(state::s_headers_almost_done);
          HTTP_REEXECUTE();
        }

        c = HTTP_TOKEN(ch);

        if (HTTP_UNLIKELY(!c)) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_HEADER_TOKEN);
          goto error;
        }

        HTTP_MARK(header_field);

        parser->index = 0;
        HTTP_UPDATE_STATE(state::s_header_field);

        switch (c) {
          case 'c':
            parser->header_state = (unsigned int)(header_states::h_C);
            break;

          case 'p':
            parser->header_state = (unsigned int)(header_states::h_matching_proxy_connection);
            break;

          case 't':
            parser->header_state = (unsigned int)(header_states::h_matching_transfer_encoding);
            break;

          case 'u':
            parser->header_state = (unsigned int)(header_states::h_matching_upgrade);
            break;

          default:
            parser->header_state = (unsigned int)(header_states::h_general);
            break;
        }
        break;
      }

      case state::s_header_field:
      {
        const char* start = p;
        for (; p != data + len; ++p) {
          ch = *p;
          c = HTTP_TOKEN(ch);

          if (!c)
            break;

          switch (header_states(parser->header_state)) {
            case header_states::h_general: {
              size_t left = size_t(data + len - p);
              const char* pe = p + HTTP_MIN(left, max_header_size);
              while (p+1 < pe && HTTP_TOKEN(p[1])) {
                p++;
              }
              break;
            }

            case header_states::h_C:
              parser->index++;
              parser->header_state = (unsigned int)(c == 'o' ? header_states::h_CO : header_states::h_general);
              break;

            case header_states::h_CO:
              parser->index++;
              parser->header_state = (unsigned int)(c == 'n' ? header_states::h_CON : header_states::h_general);
              break;

            case header_states::h_CON:
              parser->index++;
              switch (c) {
                case 'n':
                  parser->header_state = (unsigned int)(header_states::h_matching_connection);
                  break;
                case 't':
                  parser->header_state = (unsigned int)(header_states::h_matching_content_length);
                  break;
                default:
                  parser->header_state = (unsigned int)(header_states::h_general);
                  break;
              }
              break;

            /* connection */

            case header_states::h_matching_connection:
              parser->index++;
              if (parser->index > sizeof(HTTP_CONNECTION)-1
                  || c != HTTP_CONNECTION[parser->index]) {
                parser->header_state = (unsigned int)(header_states::h_general);
              } else if (parser->index == sizeof(HTTP_CONNECTION)-2) {
                parser->header_state = (unsigned int)(header_states::h_connection);
              }
              break;

            /* proxy-connection */

            case header_states::h_matching_proxy_connection:
              parser->index++;
              if (parser->index > sizeof(HTTP_PROXY_CONNECTION)-1
                  || c != HTTP_PROXY_CONNECTION[parser->index]) {
                parser->header_state = (unsigned int)(header_states::h_general);
              } else if (parser->index == sizeof(HTTP_PROXY_CONNECTION)-2) {
                parser->header_state = (unsigned int)(header_states::h_connection);
              }
              break;

            /* content-length */

            case header_states::h_matching_content_length:
              parser->index++;
              if (parser->index > sizeof(HTTP_CONTENT_LENGTH)-1
                  || c != HTTP_CONTENT_LENGTH[parser->index]) {
                parser->header_state = (unsigned int)(header_states::h_general);
              } else if (parser->index == sizeof(HTTP_CONTENT_LENGTH)-2) {
                parser->header_state = (unsigned int)(header_states::h_content_length);
              }
              break;

            /* transfer-encoding */

            case header_states::h_matching_transfer_encoding:
              parser->index++;
              if (parser->index > sizeof(HTTP_TRANSFER_ENCODING)-1
                  || c != HTTP_TRANSFER_ENCODING[parser->index]) {
                parser->header_state = (unsigned int)(header_states::h_general);
              } else if (parser->index == sizeof(HTTP_TRANSFER_ENCODING)-2) {
                parser->header_state = (unsigned int)(header_states::h_transfer_encoding);
                parser->uses_transfer_encoding = 1;
              }
              break;

            /* upgrade */

            case header_states::h_matching_upgrade:
              parser->index++;
              if (parser->index > sizeof(HTTP_UPGRADE)-1
                  || c != HTTP_UPGRADE[parser->index]) {
                parser->header_state = (unsigned int)(header_states::h_general);
              } else if (parser->index == sizeof(HTTP_UPGRADE)-2) {
                parser->header_state = (unsigned int)(header_states::h_upgrade);
              }
              break;

            case header_states::h_connection:
            case header_states::h_content_length:
            case header_states::h_transfer_encoding:
            case header_states::h_upgrade:
              if (ch != ' ') parser->header_state = (unsigned int)(header_states::h_general);
              break;

            default:
              assert(0 && "Unknown header_state");
              break;
          }
        }

        if (p == data + len) {
          --p;
          HTTP_COUNT_HEADER_SIZE(p - start);
          break;
        }

        HTTP_COUNT_HEADER_SIZE(p - start);

        if (ch == ':') {
          HTTP_UPDATE_STATE(state::s_header_value_discard_ws);
          HTTP_CALLBACK_DATA(header_field);
          break;
        }

        HTTP_SET_ERRNO(http_errno::HPE_INVALID_HEADER_TOKEN);
        goto error;
      }

      case state::s_header_value_discard_ws:
        if (ch == ' ' || ch == '\t') break;

        if (ch == HTTP_CR) {
          HTTP_UPDATE_STATE(state::s_header_value_discard_ws_almost_done);
          break;
        }

        if (ch == HTTP_LF) {
          HTTP_UPDATE_STATE(state::s_header_value_discard_lws);
          break;
        }

        /* fall through */

      case state::s_header_value_start:
      {
        HTTP_MARK(header_value);

        HTTP_UPDATE_STATE(state::s_header_value);
        parser->index = 0;

        c = HTTP_LOWER(ch);

        switch (header_states(parser->header_state)) {
          case header_states::h_upgrade:
            parser->flags |= (unsigned int)(flags::F_UPGRADE);
            parser->header_state = (unsigned int)(header_states::h_general);
            break;

          case header_states::h_transfer_encoding:
            /* looking for 'Transfer-Encoding: chunked' */
            if ('c' == c) {
              parser->header_state = (unsigned int)(header_states::h_matching_transfer_encoding_chunked);
            } else {
              parser->header_state = (unsigned int)(header_states::h_matching_transfer_encoding_token);
            }
            break;

          /* Multi-value `Transfer-Encoding` header */
          case header_states::h_matching_transfer_encoding_token_start:
            break;

          case header_states::h_content_length:
            if (HTTP_UNLIKELY(!HTTP_IS_NUM(ch))) {
              HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
              goto error;
            }

            if (parser->flags & (unsigned int)(flags::F_CONTENTLENGTH)) {
              HTTP_SET_ERRNO(http_errno::HPE_UNEXPECTED_CONTENT_LENGTH);
              goto error;
            }

            parser->flags |= (unsigned int)(flags::F_CONTENTLENGTH);
            parser->content_length = ch - '0';
            parser->header_state = (unsigned int)(header_states::h_content_length_num);
            break;

          /* when obsolete line folding is encountered for content length
           * continue to the s_header_value state */
          case header_states::h_content_length_ws:
            break;

          case header_states::h_connection:
            /* looking for 'Connection: keep-alive' */
            if (c == 'k') {
              parser->header_state = (unsigned int)(header_states::h_matching_connection_keep_alive);
            /* looking for 'Connection: close' */
            } else if (c == 'c') {
              parser->header_state = (unsigned int)(header_states::h_matching_connection_close);
            } else if (c == 'u') {
              parser->header_state = (unsigned int)(header_states::h_matching_connection_upgrade);
            } else {
              parser->header_state = (unsigned int)(header_states::h_matching_connection_token);
            }
            break;

          /* Multi-value `Connection` header */
          case header_states::h_matching_connection_token_start:
            break;

          default:
            parser->header_state = (unsigned int)(header_states::h_general);
            break;
        }
        break;
      }

      case state::s_header_value:
      {
        const char* start = p;
        header_states h_state = (header_states) parser->header_state;
        for (; p != data + len; ++p) {
          ch = *p;
          if (ch == HTTP_CR) {
            HTTP_UPDATE_STATE(state::s_header_almost_done);
            parser->header_state = (unsigned int)(h_state);
            HTTP_CALLBACK_DATA(header_value);
            break;
          }

          if (ch == HTTP_LF) {
            HTTP_UPDATE_STATE(state::s_header_almost_done);
            HTTP_COUNT_HEADER_SIZE(p - start);
            parser->header_state = (unsigned int)(h_state);
            HTTP_CALLBACK_DATA_NOADVANCE(header_value);
            HTTP_REEXECUTE();
          }

          if (!lenient && !HTTP_IS_HEADER_CHAR(ch)) {
            HTTP_SET_ERRNO(http_errno::HPE_INVALID_HEADER_TOKEN);
            goto error;
          }

          c = HTTP_LOWER(ch);

          switch (h_state) {
            case header_states::h_general:
              {
                size_t left = size_t(data + len - p);
                const char* pe = p + HTTP_MIN(left, max_header_size);

                for (; p != pe; p++) {
                  ch = *p;
                  if (ch == HTTP_CR || ch == HTTP_LF) {
                    --p;
                    break;
                  }
                  if (!lenient && !HTTP_IS_HEADER_CHAR(ch)) {
                    HTTP_SET_ERRNO(http_errno::HPE_INVALID_HEADER_TOKEN);
                    goto error;
                  }
                }
                if (p == data + len)
                  --p;
                break;
              }

            case header_states::h_connection:
            case header_states::h_transfer_encoding:
              assert(0 && "Shouldn't get here.");
              break;

            case header_states::h_content_length:
              if (ch == ' ') break;
              h_state = header_states::h_content_length_num;
              /* fall through */

            case header_states::h_content_length_num:
            {
              uint64_t t;

              if (ch == ' ') {
                h_state = header_states::h_content_length_ws;
                break;
              }

              if (HTTP_UNLIKELY(!HTTP_IS_NUM(ch))) {
                HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
                parser->header_state = (unsigned int)(h_state);
                goto error;
              }

              t = parser->content_length;
              t *= 10;
              t += ch - '0';

              /* Overflow? Test against a conservative limit for simplicity. */
              if (HTTP_UNLIKELY((HTTP_ULLONG_MAX - 10) / 10 < parser->content_length)) {
                HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
                parser->header_state = (unsigned int)(h_state);
                goto error;
              }

              parser->content_length = t;
              break;
            }

            case header_states::h_content_length_ws:
              if (ch == ' ') break;
              HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
              parser->header_state = (unsigned int)(h_state);
              goto error;

            /* Transfer-Encoding: chunked */
            case header_states::h_matching_transfer_encoding_token_start:
              /* looking for 'Transfer-Encoding: chunked' */
              if ('c' == c) {
                h_state = header_states::h_matching_transfer_encoding_chunked;
              } else if (HTTP_STRICT_TOKEN(c)) {
                /* TODO(indutny): similar code below does this, but why?
                 * At the very least it seems to be inconsistent given that
                 * h_matching_transfer_encoding_token does not check for
                 * `STRICT_TOKEN`
                 */
                h_state = header_states::h_matching_transfer_encoding_token;
              } else if (c == ' ' || c == '\t') {
                /* Skip lws */
              } else {
                h_state = header_states::h_general;
              }
              break;

            case header_states::h_matching_transfer_encoding_chunked:
              parser->index++;
              if (parser->index > sizeof(HTTP_CHUNKED)-1
                  || c != HTTP_CHUNKED[parser->index]) {
                h_state = header_states::h_matching_transfer_encoding_token;
              } else if (parser->index == sizeof(HTTP_CHUNKED)-2) {
                h_state = header_states::h_transfer_encoding_chunked;
              }
              break;

            case header_states::h_matching_transfer_encoding_token:
              if (ch == ',') {
                h_state = header_states::h_matching_transfer_encoding_token_start;
                parser->index = 0;
              }
              break;

            case header_states::h_matching_connection_token_start:
              /* looking for 'Connection: keep-alive' */
              if (c == 'k') {
                h_state = header_states::h_matching_connection_keep_alive;
              /* looking for 'Connection: close' */
              } else if (c == 'c') {
                h_state = header_states::h_matching_connection_close;
              } else if (c == 'u') {
                h_state = header_states::h_matching_connection_upgrade;
              } else if (HTTP_STRICT_TOKEN(c)) {
                h_state = header_states::h_matching_connection_token;
              } else if (c == ' ' || c == '\t') {
                /* Skip lws */
              } else {
                h_state = header_states::h_general;
              }
              break;

            /* looking for 'Connection: keep-alive' */
            case header_states::h_matching_connection_keep_alive:
              parser->index++;
              if (parser->index > sizeof(HTTP_KEEP_ALIVE)-1
                  || c != HTTP_KEEP_ALIVE[parser->index]) {
                h_state = header_states::h_matching_connection_token;
              } else if (parser->index == sizeof(HTTP_KEEP_ALIVE)-2) {
                h_state = header_states::h_connection_keep_alive;
              }
              break;

            /* looking for 'Connection: close' */
            case header_states::h_matching_connection_close:
              parser->index++;
              if (parser->index > sizeof(HTTP_CLOSE)-1 || c != HTTP_CLOSE[parser->index]) {
                h_state = header_states::h_matching_connection_token;
              } else if (parser->index == sizeof(HTTP_CLOSE)-2) {
                h_state = header_states::h_connection_close;
              }
              break;

            /* looking for 'Connection: upgrade' */
            case header_states::h_matching_connection_upgrade:
              parser->index++;
              if (parser->index > sizeof(HTTP_UPGRADE) - 1 ||
                  c != HTTP_UPGRADE[parser->index]) {
                h_state = header_states::h_matching_connection_token;
              } else if (parser->index == sizeof(HTTP_UPGRADE)-2) {
                h_state = header_states::h_connection_upgrade;
              }
              break;

            case header_states::h_matching_connection_token:
              if (ch == ',') {
                h_state = header_states::h_matching_connection_token_start;
                parser->index = 0;
              }
              break;

            case header_states::h_transfer_encoding_chunked:
              if (ch != ' ') h_state = header_states::h_matching_transfer_encoding_token;
              break;

            case header_states::h_connection_keep_alive:
            case header_states::h_connection_close:
            case header_states::h_connection_upgrade:
              if (ch == ',') {
                if (h_state == header_states::h_connection_keep_alive) {
                  parser->flags |= (unsigned int)(flags::F_CONNECTION_KEEP_ALIVE);
                } else if (h_state == header_states::h_connection_close) {
                  parser->flags |= (unsigned int)(flags::F_CONNECTION_CLOSE);
                } else if (h_state == header_states::h_connection_upgrade) {
                  parser->flags |= (unsigned int)(flags::F_CONNECTION_UPGRADE);
                }
                h_state = header_states::h_matching_connection_token_start;
                parser->index = 0;
              } else if (ch != ' ') {
                h_state = header_states::h_matching_connection_token;
              }
              break;

            default:
              HTTP_UPDATE_STATE(state::s_header_value);
              h_state = header_states::h_general;
              break;
          }
        }
        parser->header_state = (unsigned int)(h_state);

        if (p == data + len)
          --p;

        HTTP_COUNT_HEADER_SIZE(p - start);
        break;
      }

      case state::s_header_almost_done:
      {
        if (HTTP_UNLIKELY(ch != HTTP_LF)) {
          HTTP_SET_ERRNO(http_errno::HPE_LF_EXPECTED);
          goto error;
        }

        HTTP_UPDATE_STATE(state::s_header_value_lws);
        break;
      }

      case state::s_header_value_lws:
      {
        if (ch == ' ' || ch == '\t') {
          if (parser->header_state == (unsigned int)(header_states::h_content_length_num)) {
              /* treat obsolete line folding as space */
              parser->header_state = (unsigned int)(header_states::h_content_length_ws);
          }
          HTTP_UPDATE_STATE(state::s_header_value_start);
          HTTP_REEXECUTE();
        }

        /* finished the header */
        switch (header_states(parser->header_state)) {
          case header_states::h_connection_keep_alive:
            parser->flags |= (unsigned int)(flags::F_CONNECTION_KEEP_ALIVE);
            break;
          case header_states::h_connection_close:
            parser->flags |= (unsigned int)(flags::F_CONNECTION_CLOSE);
            break;
          case header_states::h_transfer_encoding_chunked:
            parser->flags |= (unsigned int)(flags::F_CHUNKED);
            break;
          case header_states::h_connection_upgrade:
            parser->flags |= (unsigned int)(flags::F_CONNECTION_UPGRADE);
            break;
          default:
            break;
        }

        HTTP_UPDATE_STATE(state::s_header_field_start);
        HTTP_REEXECUTE();
      }

      case state::s_header_value_discard_ws_almost_done:
      {
        HTTP_STRICT_CHECK(ch != HTTP_LF);
        HTTP_UPDATE_STATE(state::s_header_value_discard_lws);
        break;
      }

      case state::s_header_value_discard_lws:
      {
        if (ch == ' ' || ch == '\t') {
          HTTP_UPDATE_STATE(state::s_header_value_discard_ws);
          break;
        } else {
          switch (header_states(parser->header_state)) {
            case header_states::h_connection_keep_alive:
              parser->flags |= (unsigned int)(flags::F_CONNECTION_KEEP_ALIVE);
              break;
            case header_states::h_connection_close:
              parser->flags |= (unsigned int)(flags::F_CONNECTION_CLOSE);
              break;
            case header_states::h_connection_upgrade:
              parser->flags |= (unsigned int)(flags::F_CONNECTION_UPGRADE);
              break;
            case header_states::h_transfer_encoding_chunked:
              parser->flags |= (unsigned int)(flags::F_CHUNKED);
              break;
            case header_states::h_content_length:
              /* do not allow empty content length */
              HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
              goto error;
              break;
            default:
              break;
          }

          /* header value was empty */
          HTTP_MARK(header_value);
          HTTP_UPDATE_STATE(state::s_header_field_start);
          HTTP_CALLBACK_DATA_NOADVANCE(header_value);
          HTTP_REEXECUTE();
        }
      }

      case state::s_headers_almost_done:
      {
        HTTP_STRICT_CHECK(ch != HTTP_LF);

        if (parser->flags & (unsigned int)(flags::F_TRAILING)) {
          /* End of a chunked request */
          HTTP_UPDATE_STATE(state::s_message_done);
          HTTP_CALLBACK_NOTIFY_NOADVANCE(chunk_complete);
          HTTP_REEXECUTE();
        }

        /* Cannot use transfer-encoding and a content-length header together
           per the HTTP specification. (RFC 7230 Section 3.3.3) */
        if ((parser->uses_transfer_encoding == 1) &&
            (parser->flags & (unsigned int)(flags::F_CONTENTLENGTH))) {
          /* Allow it for lenient parsing as long as `Transfer-Encoding` is
           * not `chunked` or allow_length_with_encoding is set
           */
          if (parser->flags & (unsigned int)(flags::F_CHUNKED)) {
            if (!allow_chunked_length) {
              HTTP_SET_ERRNO(http_errno::HPE_UNEXPECTED_CONTENT_LENGTH);
              goto error;
            }
          } else if (!lenient) {
            HTTP_SET_ERRNO(http_errno::HPE_UNEXPECTED_CONTENT_LENGTH);
            goto error;
          }
        }

        HTTP_UPDATE_STATE(state::s_headers_done);

        /* Set this here so that on_headers_complete() callbacks can see it */
        if ((parser->flags & (unsigned int)(flags::F_UPGRADE)) &&
            (parser->flags & (unsigned int)(flags::F_CONNECTION_UPGRADE))) {
          /* For responses, "Upgrade: foo" and "Connection: upgrade" are
           * mandatory only when it is a 101 Switching Protocols response,
           * otherwise it is purely informational, to announce support.
           */
          parser->upgrade =
              (parser->type == (unsigned int)(http_parser_type::HTTP_REQUEST) || parser->status_code == 101);
        } else {
          parser->upgrade = (parser->method == (unsigned int)(http_method::HTTP_CONNECT));
        }

        /* Here we call the headers_complete callback. This is somewhat
         * different than other callbacks because if the user returns 1, we
         * will interpret that as saying that this message has no body. This
         * is needed for the annoying case of recieving a response to a HEAD
         * request.
         *
         * We'd like to use HTTP_CALLBACK_NOTIFY_NOADVANCE() here but we cannot, so
         * we have to simulate it by handling a change in errno below.
         */
        if (settings->on_headers_complete) {
          switch (settings->on_headers_complete(parser, user_data)) {
            case 0:
              break;

            case 2:
              parser->upgrade = 1;

            /* fall through */
            case 1:
              parser->flags |= (unsigned int)(flags::F_SKIPBODY);
              break;

            default:
              HTTP_SET_ERRNO(http_errno::HPE_CB_headers_complete);
              HTTP_RETURN(p - data); /* Error */
          }
        }

        if (HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK) {
          HTTP_RETURN(p - data);
        }

        HTTP_REEXECUTE();
      }

      case state::s_headers_done:
      {
        int hasBody;
        HTTP_STRICT_CHECK(ch != HTTP_LF);

        parser->nread = 0;
        nread = 0;

        hasBody = parser->flags & (unsigned int)(flags::F_CHUNKED) ||
          (parser->content_length > 0 && parser->content_length != HTTP_ULLONG_MAX);
        if (parser->upgrade && (parser->method == (unsigned int)(http_method::HTTP_CONNECT) ||
                                (parser->flags & (unsigned int)(flags::F_SKIPBODY)) || !hasBody)) {
          /* Exit, the rest of the message is in a different protocol. */
          HTTP_UPDATE_STATE(HTTP_NEW_MESSAGE());
          HTTP_CALLBACK_NOTIFY(message_complete);
          HTTP_RETURN((p - data) + 1);
        }

        if (parser->flags & (unsigned int)(flags::F_SKIPBODY)) {
          HTTP_UPDATE_STATE(HTTP_NEW_MESSAGE());
          HTTP_CALLBACK_NOTIFY(message_complete);
        } else if (parser->flags & (unsigned int)(flags::F_CHUNKED)) {
          /* chunked encoding - ignore Content-Length header,
           * prepare for a chunk */
          HTTP_UPDATE_STATE(state::s_chunk_size_start);
        } else if (parser->uses_transfer_encoding == 1) {
          if (parser->type == (unsigned int)(http_parser_type::HTTP_REQUEST) && !lenient) {
            /* RFC 7230 3.3.3 */

            /* If a Transfer-Encoding header field
             * is present in a request and the chunked transfer coding is not
             * the final encoding, the message body length cannot be determined
             * reliably; the server MUST respond with the 400 (Bad Request)
             * status code and then close the connection.
             */
            HTTP_SET_ERRNO(http_errno::HPE_INVALID_TRANSFER_ENCODING);
            HTTP_RETURN(p - data); /* Error */
          } else {
            /* RFC 7230 3.3.3 */

            /* If a Transfer-Encoding header field is present in a response and
             * the chunked transfer coding is not the final encoding, the
             * message body length is determined by reading the connection until
             * it is closed by the server.
             */
            HTTP_UPDATE_STATE(state::s_body_identity_eof);
          }
        } else {
          if (parser->content_length == 0) {
            /* Content-Length header given but zero: Content-Length: 0\r\n */
            HTTP_UPDATE_STATE(HTTP_NEW_MESSAGE());
            HTTP_CALLBACK_NOTIFY(message_complete);
          } else if (parser->content_length != HTTP_ULLONG_MAX) {
            /* Content-Length header given and non-zero */
            HTTP_UPDATE_STATE(state::s_body_identity);
          } else {
            if (!http_message_needs_eof(parser)) {
              /* Assume content-length 0 - read the next */
              HTTP_UPDATE_STATE(HTTP_NEW_MESSAGE());
              HTTP_CALLBACK_NOTIFY(message_complete);
            } else {
              /* Read body until EOF */
              HTTP_UPDATE_STATE(state::s_body_identity_eof);
            }
          }
        }

        break;
      }

      case state::s_body_identity:
      {
        uint64_t to_read = HTTP_MIN(parser->content_length,
                               (uint64_t) ((data + len) - p));

        assert(parser->content_length != 0
            && parser->content_length != HTTP_ULLONG_MAX);

        /* The difference between advancing content_length and p is because
         * the latter will automaticaly advance on the next loop iteration.
         * Further, if content_length ends up at 0, we want to see the last
         * byte again for our message complete callback.
         */
        HTTP_MARK(body);
        parser->content_length -= to_read;
        p += to_read - 1;

        if (parser->content_length == 0) {
          HTTP_UPDATE_STATE(state::s_message_done);

          /* Mimic HTTP_CALLBACK_DATA_NOADVANCE() but with one extra byte.
           *
           * The alternative to doing this is to wait for the next byte to
           * trigger the data callback, just as in every other case. The
           * problem with this is that this makes it difficult for the test
           * harness to distinguish between complete-on-EOF and
           * complete-on-length. It's not clear that this distinction is
           * important for applications, but let's keep it for now.
           */
          HTTP_CALLBACK_DATA_(body, p - body_mark + 1, p - data);
          HTTP_REEXECUTE();
        }

        break;
      }

      /* read until EOF */
      case state::s_body_identity_eof:
        HTTP_MARK(body);
        p = data + len - 1;

        break;

      case state::s_message_done:
        HTTP_UPDATE_STATE(HTTP_NEW_MESSAGE());
        HTTP_CALLBACK_NOTIFY(message_complete);
        if (parser->upgrade) {
          /* Exit, the rest of the message is in a different protocol. */
          HTTP_RETURN((p - data) + 1);
        }
        break;

      case state::s_chunk_size_start:
      {
        assert(nread == 1);
        assert(parser->flags & (unsigned int)(flags::F_CHUNKED));

        unhex_val = unhex[(unsigned char)ch];
        if (HTTP_UNLIKELY(unhex_val == -1)) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_CHUNK_SIZE);
          goto error;
        }

        parser->content_length = unhex_val;
        HTTP_UPDATE_STATE(state::s_chunk_size);
        break;
      }

      case state::s_chunk_size:
      {
        uint64_t t;

        assert(parser->flags & (unsigned int)(flags::F_CHUNKED));

        if (ch == HTTP_CR) {
          HTTP_UPDATE_STATE(state::s_chunk_size_almost_done);
          break;
        }

        unhex_val = unhex[(unsigned char)ch];

        if (unhex_val == -1) {
          if (ch == ';' || ch == ' ') {
            HTTP_UPDATE_STATE(state::s_chunk_parameters);
            break;
          }

          HTTP_SET_ERRNO(http_errno::HPE_INVALID_CHUNK_SIZE);
          goto error;
        }

        t = parser->content_length;
        t *= 16;
        t += unhex_val;

        /* Overflow? Test against a conservative limit for simplicity. */
        if (HTTP_UNLIKELY((HTTP_ULLONG_MAX - 16) / 16 < parser->content_length)) {
          HTTP_SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
          goto error;
        }

        parser->content_length = t;
        break;
      }

      case state::s_chunk_parameters:
      {
        assert(parser->flags & (unsigned int)(flags::F_CHUNKED));
        /* just ignore this shit. TODO check for overflow */
        if (ch == HTTP_CR) {
          HTTP_UPDATE_STATE(state::s_chunk_size_almost_done);
          break;
        }
        break;
      }

      case state::s_chunk_size_almost_done:
      {
        assert(parser->flags & (unsigned int)(flags::F_CHUNKED));
        HTTP_STRICT_CHECK(ch != HTTP_LF);

        parser->nread = 0;
        nread = 0;

        if (parser->content_length == 0) {
          parser->flags |= (unsigned int)(flags::F_TRAILING);
          HTTP_UPDATE_STATE(state::s_header_field_start);
        } else {
          HTTP_UPDATE_STATE(state::s_chunk_data);
        }
        HTTP_CALLBACK_NOTIFY(chunk_header);
        break;
      }

      case state::s_chunk_data:
      {
        uint64_t to_read = HTTP_MIN(parser->content_length,
                               (uint64_t) ((data + len) - p));

        assert(parser->flags & (unsigned int)(flags::F_CHUNKED));
        assert(parser->content_length != 0
            && parser->content_length != HTTP_ULLONG_MAX);

        /* See the explanation in state::s_body_identity for why the content
         * length and data pointers are managed this way.
         */
        HTTP_MARK(body);
        parser->content_length -= to_read;
        p += to_read - 1;

        if (parser->content_length == 0) {
          HTTP_UPDATE_STATE(state::s_chunk_data_almost_done);
        }

        break;
      }

      case state::s_chunk_data_almost_done:
        assert(parser->flags & (unsigned int)(flags::F_CHUNKED));
        assert(parser->content_length == 0);
        HTTP_STRICT_CHECK(ch != HTTP_CR);
        HTTP_UPDATE_STATE(state::s_chunk_data_done);
        HTTP_CALLBACK_DATA(body);
        break;

      case state::s_chunk_data_done:
        assert(parser->flags & (unsigned int)(flags::F_CHUNKED));
        HTTP_STRICT_CHECK(ch != HTTP_LF);
        parser->nread = 0;
        nread = 0;
        HTTP_UPDATE_STATE(state::s_chunk_size_start);
        HTTP_CALLBACK_NOTIFY(chunk_complete);
        break;

      default:
        assert(0 && "unhandled state");
        HTTP_SET_ERRNO(http_errno::HPE_INVALID_INTERNAL_STATE);
        goto error;
    }
  }

  /* Run callbacks for any marks that we have leftover after we ran out of
   * bytes. There should be at most one of these set, so it's OK to invoke
   * them in series (unset marks will not result in callbacks).
   *
   * We use the NOADVANCE() variety of callbacks here because 'p' has already
   * overflowed 'data' and this allows us to correct for the off-by-one that
   * we'd otherwise have (since HTTP_CALLBACK_DATA() is meant to be run with a 'p'
   * value that's in-bounds).
   */

  assert(((header_field_mark ? 1 : 0) +
          (header_value_mark ? 1 : 0) +
          (url_mark ? 1 : 0)  +
          (body_mark ? 1 : 0) +
          (status_mark ? 1 : 0)) <= 1);

  HTTP_CALLBACK_DATA_NOADVANCE(header_field);
  HTTP_CALLBACK_DATA_NOADVANCE(header_value);
  HTTP_CALLBACK_DATA_NOADVANCE(url);
  HTTP_CALLBACK_DATA_NOADVANCE(body);
  HTTP_CALLBACK_DATA_NOADVANCE(status);

  HTTP_RETURN(len);

error:
  if (HTTP_PARSER_ERRNO(parser) == http_errno::HPE_OK) {
    HTTP_SET_ERRNO(http_errno::HPE_UNKNOWN);
  }

  HTTP_RETURN(p - data);
}

/* Returns a string version of the HTTP method. */
template<typename = void>
const char *http_method_str (http_method m)
{
  return HTTP_ELEM_AT(method_strings, m, "<unknown>");
}

/* Returns a string version of the HTTP status code. */
template<typename = void>
const char *http_status_str(http_status s)
{
	switch (s) {
#define HTTP_XX(num, name, string) case http_status::name: return #string;
		HTTP_STATUS_MAP(HTTP_XX)
#undef HTTP_XX
	default: return "<unknown>";
	}
}

// add by zhllxt
template<typename = void>
const char * http_status_str(unsigned int v)
{
	return http_status_str(static_cast<http_status>(v));
}

template<typename = void>
void
http_parser_init (http_parser *parser, http_parser_type t)
{
  void *data = parser->data; /* preserve application data */
  memset(parser, 0, sizeof(*parser));
  parser->data = data;
  parser->type = (unsigned int)(t);
  parser->state = (unsigned int)(t == http_parser_type::HTTP_REQUEST ? state::s_start_req : (t == http_parser_type::HTTP_RESPONSE ? state::s_start_res : state::s_start_req_or_res));
  parser->http_errno = (unsigned int)(http_errno::HPE_OK);
}

template<typename = void>
void
http_parser_settings_init(http_parser_settings *settings)
{
  memset(settings, 0, sizeof(*settings));
}

template<typename = void>
const char *
http_errno_name(http_errno err) {
  assert(((size_t) err) < HTTP_ARRAY_SIZE(http_strerror_tab));
  return http_strerror_tab[(size_t)err].name;
}

template<typename = void>
const char *
http_errno_description(http_errno err) {
  assert(((size_t) err) < HTTP_ARRAY_SIZE(http_strerror_tab));
  return http_strerror_tab[(size_t)err].description;
}

template<typename = void>
static http_host_state
http_parse_host_char(http_host_state s, const char ch) {
  switch(s) {
    case http_host_state::s_http_userinfo:
    case http_host_state::s_http_userinfo_start:
      if (ch == '@') {
        return http_host_state::s_http_host_start;
      }

      if (HTTP_IS_USERINFO_CHAR(ch)) {
        return http_host_state::s_http_userinfo;
      }
      break;

    case http_host_state::s_http_host_start:
      if (ch == '[') {
        return http_host_state::s_http_host_v6_start;
      }

      if (HTTP_IS_HOST_CHAR(ch)) {
        return http_host_state::s_http_host;
      }

      break;

    case http_host_state::s_http_host:
      if (HTTP_IS_HOST_CHAR(ch)) {
        return http_host_state::s_http_host;
      }

    /* fall through */
    case http_host_state::s_http_host_v6_end:
      if (ch == ':') {
        return http_host_state::s_http_host_port_start;
      }

      break;

    case http_host_state::s_http_host_v6:
      if (ch == ']') {
        return http_host_state::s_http_host_v6_end;
      }

    /* fall through */
    case http_host_state::s_http_host_v6_start:
      if (HTTP_IS_HEX(ch) || ch == ':' || ch == '.') {
        return http_host_state::s_http_host_v6;
      }

      if (s == http_host_state::s_http_host_v6 && ch == '%') {
        return http_host_state::s_http_host_v6_zone_start;
      }
      break;

    case http_host_state::s_http_host_v6_zone:
      if (ch == ']') {
        return http_host_state::s_http_host_v6_end;
      }

    /* fall through */
    case http_host_state::s_http_host_v6_zone_start:
      /* RFC 6874 Zone ID consists of 1*( unreserved / pct-encoded) */
      if (HTTP_IS_ALPHANUM(ch) || ch == '%' || ch == '.' || ch == '-' || ch == '_' ||
          ch == '~') {
        return http_host_state::s_http_host_v6_zone;
      }
      break;

    case http_host_state::s_http_host_port:
    case http_host_state::s_http_host_port_start:
      if (HTTP_IS_NUM(ch)) {
        return http_host_state::s_http_host_port;
      }

      break;

    default:
      break;
  }
  return http_host_state::s_http_host_dead;
}

template<typename = void>
static int
http_parse_host(const char * buf, struct http_parser_url *u, int found_at) {
  http_host_state s;

  const char *p;
  size_t buflen = u->field_data[(int)url_fields::UF_HOST].off + u->field_data[(int)url_fields::UF_HOST].len;

  assert(u->field_set & (1 << (int)url_fields::UF_HOST));

  u->field_data[(int)url_fields::UF_HOST].len = 0;

  s = found_at ? http_host_state::s_http_userinfo_start : http_host_state::s_http_host_start;

  for (p = buf + u->field_data[(int)url_fields::UF_HOST].off; p < buf + buflen; ++p) {
    http_host_state new_s = http_parse_host_char(s, *p);

    if (new_s == http_host_state::s_http_host_dead) {
      return 1;
    }

    switch(new_s) {
      case http_host_state::s_http_host:
        if (s != http_host_state::s_http_host) {
          u->field_data[(int)url_fields::UF_HOST].off = static_cast<uint16_t>(p - buf);
        }
        u->field_data[(int)url_fields::UF_HOST].len++;
        break;

      case http_host_state::s_http_host_v6:
        if (s != http_host_state::s_http_host_v6) {
          u->field_data[(int)url_fields::UF_HOST].off = static_cast<uint16_t>(p - buf);
        }
        u->field_data[(int)url_fields::UF_HOST].len++;
        break;

      case http_host_state::s_http_host_v6_zone_start:
      case http_host_state::s_http_host_v6_zone:
        u->field_data[(int)url_fields::UF_HOST].len++;
        break;

      case http_host_state::s_http_host_port:
        if (s != http_host_state::s_http_host_port) {
          u->field_data[(int)url_fields::UF_PORT].off = static_cast<uint16_t>(p - buf);
          u->field_data[(int)url_fields::UF_PORT].len = 0;
          u->field_set |= (1 << (int)url_fields::UF_PORT);
        }
        u->field_data[(int)url_fields::UF_PORT].len++;
        break;

      case http_host_state::s_http_userinfo:
        if (s != http_host_state::s_http_userinfo) {
          u->field_data[(int)url_fields::UF_USERINFO].off = static_cast<uint16_t>(p - buf);
          u->field_data[(int)url_fields::UF_USERINFO].len = 0;
          u->field_set |= (1 << (int)url_fields::UF_USERINFO);
        }
        u->field_data[(int)url_fields::UF_USERINFO].len++;
        break;

      default:
        break;
    }
    s = new_s;
  }

  /* Make sure we don't end somewhere unexpected */
  switch (s) {
    case http_host_state::s_http_host_start:
    case http_host_state::s_http_host_v6_start:
    case http_host_state::s_http_host_v6:
    case http_host_state::s_http_host_v6_zone_start:
    case http_host_state::s_http_host_v6_zone:
    case http_host_state::s_http_host_port_start:
    case http_host_state::s_http_userinfo:
    case http_host_state::s_http_userinfo_start:
      return 1;
    default:
      break;
  }

  return 0;
}

template<typename = void>
void
http_parser_url_init(struct http_parser_url *u) {
  memset(u, 0, sizeof(*u));
}

template<typename = void>
int
http_parser_parse_url(const char *buf, size_t buflen, int is_connect,
                      struct http_parser_url *u)
{
  state s;
  const char *p;
  url_fields uf, old_uf;
  int found_at = 0;

  if (buflen == 0) {
    return 1;
  }

  u->port = u->field_set = 0;
  s = is_connect ? state::s_req_server_start : state::s_req_spaces_before_url;
  old_uf = url_fields::UF_MAX;

  for (p = buf; p < buf + buflen; ++p) {
    s = parse_url_char(s, *p);

    /* Figure out the next field that we're operating on */
    switch (s) {
      case state::s_dead:
        return 1;

      /* Skip delimeters */
      case state::s_req_schema_slash:
      case state::s_req_schema_slash_slash:
      case state::s_req_server_start:
      case state::s_req_query_string_start:
      case state::s_req_fragment_start:
        continue;

      case state::s_req_schema:
        uf = url_fields::UF_SCHEMA;
        break;

      case state::s_req_server_with_at:
        found_at = 1;

      /* fall through */
      case state::s_req_server:
        uf = url_fields::UF_HOST;
        break;

      case state::s_req_path:
        uf = url_fields::UF_PATH;
        break;

      case state::s_req_query_string:
        uf = url_fields::UF_QUERY;
        break;

      case state::s_req_fragment:
        uf = url_fields::UF_FRAGMENT;
        break;

      default:
        assert(!"Unexpected state");
        return 1;
    }

    /* Nothing's changed; soldier on */
    if (uf == old_uf) {
      u->field_data[(int)uf].len++;
      continue;
    }

    u->field_data[(int)uf].off = static_cast<uint16_t>(p - buf);
    u->field_data[(int)uf].len = 1;

    u->field_set |= uint16_t(1 << (int)uf);
    old_uf = uf;
  }

  /* host must be present if there is a schema */
  /* parsing http:///toto will fail */
  if ((u->field_set & (1 << (int)url_fields::UF_SCHEMA)) &&
      (u->field_set & (1 << (int)url_fields::UF_HOST)) == 0) {
    return 1;
  }

  if (u->field_set & (1 << (int)url_fields::UF_HOST)) {
    if (http_parse_host(buf, u, found_at) != 0) {
      return 1;
    }
  }

  /* CONNECT requests can only contain "hostname:port" */
  if (is_connect && u->field_set != ((1 << (int)url_fields::UF_HOST)|(1 << (int)url_fields::UF_PORT))) {
    return 1;
  }

  if (u->field_set & (1 << (int)url_fields::UF_PORT)) {
    uint16_t off;
    uint16_t len;
    const char* pn;
    const char* end;
    unsigned long v;

    off = u->field_data[(int)url_fields::UF_PORT].off;
    len = u->field_data[(int)url_fields::UF_PORT].len;
    end = buf + off + len;

    /* NOTE: The characters are already validated and are in the [0-9] range */
    assert(size_t(off + len) <= buflen && "Port number overflow");
    v = 0;
    for (pn = buf + off; pn < end; ++pn) {
      v *= 10;
      v += *pn - '0';

      /* Ports have a max value of 2^16 */
      if (v > 0xffff) {
        return 1;
      }
    }

    u->port = (uint16_t) v;
  }

  return 0;
}

template<typename = void>
void
http_parser_pause(http_parser *parser, int paused) {
  /* Users should only be pausing/unpausing a parser that is not in an error
   * state. In non-debug builds, there's not much that we can do about this
   * other than ignore it.
   */
  if (HTTP_PARSER_ERRNO(parser) == http_errno::HPE_OK ||
      HTTP_PARSER_ERRNO(parser) == http_errno::HPE_PAUSED) {
    uint32_t nread = parser->nread; /* used by the HTTP_SET_ERRNO macro */
    HTTP_SET_ERRNO((paused) ? http_errno::HPE_PAUSED : http_errno::HPE_OK);
  } else {
    assert(0 && "Attempting to pause parser in error state");
  }
}

/* Checks if this is the final chunk of the body. */
template<typename = void>
int http_body_is_final(const struct http_parser *parser) {
    return parser->state == (unsigned int)(state::s_message_done);
}

/* Change the maximum header size provided at compile time. */
template<typename = void>
void http_parser_set_max_header_size(uint32_t size)
{
  max_header_size = size;
}

template<typename = void>
unsigned long
http_parser_version(void) {
  return HTTP_PARSER_VERSION_MAJOR * 0x10000 |
         HTTP_PARSER_VERSION_MINOR * 0x00100 |
         HTTP_PARSER_VERSION_PATCH * 0x00001;
}

#undef HTTP_CR
#undef HTTP_LF
#undef HTTP_LOWER
#undef HTTP_IS_ALPHA
#undef HTTP_IS_NUM
#undef HTTP_IS_ALPHANUM
#undef HTTP_IS_HEX
#undef HTTP_IS_MARK
#undef HTTP_IS_USERINFO_CHAR
#undef HTTP_STRICT_TOKEN
#undef HTTP_TOKEN
#undef HTTP_IS_URL_CHAR
#undef HTTP_IS_HOST_CHAR
#undef HTTP_IS_HEADER_CHAR
#undef start_state
#undef HTTP_STRICT_CHECK
#undef HTTP_NEW_MESSAGE
#undef HTTP_STRERROR_GEN
#undef HTTP_PARSING_HEADER
#undef T
#undef HTTP_PROXY_CONNECTION
#undef HTTP_CONNECTION
#undef HTTP_CONTENT_LENGTH
#undef HTTP_TRANSFER_ENCODING
#undef HTTP_UPGRADE
#undef HTTP_CHUNKED
#undef HTTP_KEEP_ALIVE
#undef HTTP_CLOSE
#undef HTTP_COUNT_HEADER_SIZE
#undef HTTP_MARK
#undef HTTP_CALLBACK_DATA_NOADVANCE
#undef HTTP_CALLBACK_DATA
#undef HTTP_CALLBACK_DATA_
#undef HTTP_CALLBACK_NOTIFY_NOADVANCE
#undef HTTP_CALLBACK_NOTIFY
#undef HTTP_CALLBACK_NOTIFY_
#undef HTTP_UNLIKELY
#undef HTTP_LIKELY
#undef HTTP_REEXECUTE
#undef HTTP_RETURN
#undef HTTP_UPDATE_STATE
#undef HTTP_CURRENT_STATE
#undef HTTP_SET_ERRNO
#undef HTTP_ELEM_AT
#undef HTTP_BIT_AT
#undef HTTP_ARRAY_SIZE
#undef HTTP_MIN
#undef HTTP_ULLONG_MAX
#undef HTTP_PARSER_ERRNO
#undef HTTP_ERRNO_GEN
#undef HTTP_ERRNO_MAP
#undef HTTP_XX
#undef HTTP_STATUS_MAP
#undef HTTP_MAX_HEADER_SIZE
#undef HTTP_PARSER_STRICT

//#ifdef __cplusplus
//}
//#endif
}

#include <asio2/base/detail/pop_options.hpp>

#endif
