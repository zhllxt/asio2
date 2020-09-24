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

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef BEAST_HEADER_ONLY
namespace beast::http::nginx_parser {
#else
namespace boost::beast::http::nginx_parser {
#endif
//#ifdef __cplusplus
//extern "C" {
//#endif

/* Also update SONAME in the Makefile whenever you change these. */
#define HTTP_PARSER_VERSION_MAJOR 2
#define HTTP_PARSER_VERSION_MINOR 8
#define HTTP_PARSER_VERSION_PATCH 1

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
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

enum class http_status
  {
#define XX(num, name, string) HTTP_STATUS_##name = num,
  HTTP_STATUS_MAP(XX)
#undef XX
  };


/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

enum class http_method
  {
#define XX(num, name, string) HTTP_##name = num,
  HTTP_METHOD_MAP(XX)
#undef XX
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
#define HTTP_ERRNO_MAP(XX)                                           \
  /* No error */                                                     \
  XX(OK, "success")                                                  \
                                                                     \
  /* Callback-related errors */                                      \
  XX(CB_message_begin, "the on_message_begin callback failed")       \
  XX(CB_url, "the on_url callback failed")                           \
  XX(CB_header_field, "the on_header_field callback failed")         \
  XX(CB_header_value, "the on_header_value callback failed")         \
  XX(CB_headers_complete, "the on_headers_complete callback failed") \
  XX(CB_body, "the on_body callback failed")                         \
  XX(CB_message_complete, "the on_message_complete callback failed") \
  XX(CB_status, "the on_status callback failed")                     \
  XX(CB_chunk_header, "the on_chunk_header callback failed")         \
  XX(CB_chunk_complete, "the on_chunk_complete callback failed")     \
                                                                     \
  /* Parsing-related errors */                                       \
  XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                \
  XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")      \
  XX(INVALID_VERSION, "invalid HTTP version")                        \
  XX(INVALID_STATUS, "invalid HTTP status code")                     \
  XX(INVALID_METHOD, "invalid HTTP method")                          \
  XX(INVALID_URL, "invalid URL")                                     \
  XX(INVALID_HOST, "invalid host")                                   \
  XX(INVALID_PORT, "invalid port")                                   \
  XX(INVALID_PATH, "invalid path")                                   \
  XX(INVALID_QUERY_STRING, "invalid query string")                   \
  XX(INVALID_FRAGMENT, "invalid fragment")                           \
  XX(LF_EXPECTED, "LF character expected")                           \
  XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                   \
  XX(UNEXPECTED_CONTENT_LENGTH,                                      \
     "unexpected content-length header")                             \
  XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                       \
  XX(INVALID_CONSTANT, "invalid constant string")                    \
  XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  XX(STRICT, "strict mode assertion failed")                         \
  XX(PAUSED, "parser is paused")                                     \
  XX(UNKNOWN, "an unknown error occurred")


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
  unsigned int flags : 8;        /* F_* values from 'flags' enum; semi-public */
  unsigned int state : 7;        /* enum state from http_parser.c */
  unsigned int header_state : 7; /* enum header_state from http_parser.c */
  unsigned int index : 7;        /* index into current matcher */
  unsigned int lenient_http_headers : 1;

  uint32_t nread;          /* # bytes read in various scenarios */
  uint64_t content_length; /* # bytes in body (0 if no Content-Length header) */

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

#ifndef ULLONG_MAX
# define ULLONG_MAX ((uint64_t) -1) /* 2^64-1 */
#endif

#ifndef MIN
# define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef BIT_AT
# define BIT_AT(a, i)                                                \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
   (1 << ((unsigned int) (i) & 7))))
#endif

#ifndef ELEM_AT
# define ELEM_AT(a, i, v) ((unsigned int) (i) < ARRAY_SIZE(a) ? (a)[(unsigned int)(i)] : (v))
#endif

#define SET_ERRNO(e)                                                 \
do {                                                                 \
  parser->http_errno = (e);                                          \
} while(0)

#define CURRENT_STATE() p_state
#define UPDATE_STATE(V) p_state = (state) (V);
#define RETURN(V)                                                    \
do {                                                                 \
  parser->state = CURRENT_STATE();                                   \
  return (V);                                                        \
} while (0);
#define REEXECUTE()                                                  \
  goto reexecute;                                                    \


#ifdef __GNUC__
# define LIKELY(X) __builtin_expect(!!(X), 1)
# define UNLIKELY(X) __builtin_expect(!!(X), 0)
#else
# define LIKELY(X) (X)
# define UNLIKELY(X) (X)
#endif


/* Run the notify callback FOR, returning ER if it fails */
#define CALLBACK_NOTIFY_(FOR, ER)                                    \
do {                                                                 \
  assert(HTTP_PARSER_ERRNO(parser) == http_errno::HPE_OK);                       \
                                                                     \
  if (LIKELY(settings->on_##FOR)) {                                  \
    parser->state = CURRENT_STATE();                                 \
    if (UNLIKELY(0 != settings->on_##FOR(parser, user_data))) {      \
      SET_ERRNO(http_errno::HPE_CB_##FOR);                                       \
    }                                                                \
    UPDATE_STATE(parser->state);                                     \
                                                                     \
    /* We either errored above or got paused; get out */             \
    if (UNLIKELY(HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK)) {             \
      return (ER);                                                   \
    }                                                                \
  }                                                                  \
} while (0)

/* Run the notify callback FOR and consume the current byte */
#define CALLBACK_NOTIFY(FOR)            CALLBACK_NOTIFY_(FOR, p - data + 1)

/* Run the notify callback FOR and don't consume the current byte */
#define CALLBACK_NOTIFY_NOADVANCE(FOR)  CALLBACK_NOTIFY_(FOR, p - data)

/* Run data callback FOR with LEN bytes, returning ER if it fails */
#define CALLBACK_DATA_(FOR, LEN, ER)                                 \
do {                                                                 \
  assert(HTTP_PARSER_ERRNO(parser) == http_errno::HPE_OK);                       \
                                                                     \
  if (FOR##_mark) {                                                  \
    if (LIKELY(settings->on_##FOR)) {                                \
      parser->state = CURRENT_STATE();                               \
      if (UNLIKELY(0 !=                                              \
        settings->on_##FOR(parser, FOR##_mark, (LEN), user_data))) { \
        SET_ERRNO(http_errno::HPE_CB_##FOR);                                     \
      }                                                              \
      UPDATE_STATE(parser->state);                                   \
                                                                     \
      /* We either errored above or got paused; get out */           \
      if (UNLIKELY(HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK)) {           \
        return (ER);                                                 \
      }                                                              \
    }                                                                \
    FOR##_mark = NULL;                                               \
  }                                                                  \
} while (0)

/* Run the data callback FOR and consume the current byte */
#define CALLBACK_DATA(FOR)                                           \
    CALLBACK_DATA_(FOR, p - FOR##_mark, p - data + 1)

/* Run the data callback FOR and don't consume the current byte */
#define CALLBACK_DATA_NOADVANCE(FOR)                                 \
    CALLBACK_DATA_(FOR, p - FOR##_mark, p - data)

/* Set the mark FOR; non-destructive if mark is already set */
#define MARK(FOR)                                                    \
do {                                                                 \
  if (!FOR##_mark) {                                                 \
    FOR##_mark = p;                                                  \
  }                                                                  \
} while (0)

/* Don't allow the total size of the HTTP headers (including the status
 * line) to exceed HTTP_MAX_HEADER_SIZE.  This check is here to protect
 * embedders against denial-of-service attacks where the attacker feeds
 * us a never-ending header that the embedder keeps buffering.
 *
 * This check is arguably the responsibility of embedders but we're doing
 * it on the embedder's behalf because most won't bother and this way we
 * make the web a little safer.  HTTP_MAX_HEADER_SIZE is still far bigger
 * than any reasonable request or response so this should never affect
 * day-to-day operation.
 */
#define COUNT_HEADER_SIZE(V)                                         \
do {                                                                 \
  parser->nread += static_cast<uint32_t>(V);                                              \
  if (UNLIKELY(parser->nread > (HTTP_MAX_HEADER_SIZE))) {            \
    SET_ERRNO(http_errno::HPE_HEADER_OVERFLOW);                                  \
    goto error;                                                      \
  }                                                                  \
} while (0)


#define PROXY_CONNECTION "proxy-connection"
#define CONNECTION "connection"
#define CONTENT_LENGTH "content-length"
#define TRANSFER_ENCODING "transfer-encoding"
#define UPGRADE "upgrade"
#define CHUNKED "chunked"
#define KEEP_ALIVE "keep-alive"
#define CLOSE "close"


static const char *method_strings[] =
  {
#define XX(num, name, string) #string,
  HTTP_METHOD_MAP(XX)
#undef XX
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
        0,      '!',      0,      '#',     '$',     '%',     '&',    '\'',
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
   * checking. See the PARSING_HEADER() macro.
   */

  , s_chunk_data
  , s_chunk_data_almost_done
  , s_chunk_data_done

  , s_body_identity
  , s_body_identity_eof

  , s_message_done
  };


#define PARSING_HEADER(_state) (_state <= state::s_headers_done)


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

  , h_matching_transfer_encoding_chunked
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
#define CR                  '\r'
#define LF                  '\n'
#define LOWER(c)            (unsigned char)(c | 0x20)
#define IS_ALPHA(c)         (LOWER(c) >= 'a' && LOWER(c) <= 'z')
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c)      (IS_ALPHA(c) || IS_NUM(c))
#define IS_HEX(c)           (IS_NUM(c) || (LOWER(c) >= 'a' && LOWER(c) <= 'f'))
#define IS_MARK(c)          ((c) == '-' || (c) == '_' || (c) == '.' || \
  (c) == '!' || (c) == '~' || (c) == '*' || (c) == '\'' || (c) == '(' || \
  (c) == ')')
#define IS_USERINFO_CHAR(c) (IS_ALPHANUM(c) || IS_MARK(c) || (c) == '%' || \
  (c) == ';' || (c) == ':' || (c) == '&' || (c) == '=' || (c) == '+' || \
  (c) == '$' || (c) == ',')

#define STRICT_TOKEN(c)     (tokens[(unsigned char)c])

#if HTTP_PARSER_STRICT
#define TOKEN(c)            (tokens[(unsigned char)c])
#define IS_URL_CHAR(c)      (BIT_AT(normal_url_char, (unsigned char)c))
#define IS_HOST_CHAR(c)     (IS_ALPHANUM(c) || (c) == '.' || (c) == '-')
#else
#define TOKEN(c)            ((c == ' ') ? ' ' : tokens[(unsigned char)c])
#define IS_URL_CHAR(c)                                                         \
  (BIT_AT(normal_url_char, (unsigned char)c) || ((c) & 0x80))
#define IS_HOST_CHAR(c)                                                        \
  (IS_ALPHANUM(c) || (c) == '.' || (c) == '-' || (c) == '_')
#endif

/**
 * Verify that a char is a valid visible (printable) US-ASCII
 * character or %x80-FF
 **/
#define IS_HEADER_CHAR(ch)                                                     \
  (ch == CR || ch == LF || ch == 9 || ((unsigned char)ch > 31 && ch != 127))

#define start_state (parser->type == (unsigned int)http_parser_type::HTTP_REQUEST ? state::s_start_req : state::s_start_res)


#if HTTP_PARSER_STRICT
# define STRICT_CHECK(cond)                                          \
do {                                                                 \
  if (cond) {                                                        \
    SET_ERRNO(http_errno::HPE_STRICT);                               \
    goto error;                                                      \
  }                                                                  \
} while (0)
# define NEW_MESSAGE() (http_should_keep_alive(parser) ? start_state : state::s_dead)
#else
# define STRICT_CHECK(cond)
# define NEW_MESSAGE() start_state
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
  if (parser->type == (unsigned int)http_parser_type::HTTP_REQUEST) {
    return 0;
  }

  /* See RFC 2616 section 4.4 */
  if (parser->status_code / 100 == 1 || /* 1xx e.g. Continue */
      parser->status_code == 204 ||     /* No Content */
      parser->status_code == 304 ||     /* Not Modified */
      parser->flags & (int)flags::F_SKIPBODY) {     /* response to a HEAD request */
    return 0;
  }

  if ((parser->flags & (int)flags::F_CHUNKED) || parser->content_length != ULLONG_MAX) {
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
    if (parser->flags & (int)flags::F_CONNECTION_CLOSE) {
      return 0;
    }
  } else {
    /* HTTP/1.0 or earlier */
    if (!(parser->flags & (int)flags::F_CONNECTION_KEEP_ALIVE)) {
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

      if (IS_ALPHA(ch)) {
        return state::s_req_schema;
      }

      break;

    case state::s_req_schema:
      if (IS_ALPHA(ch)) {
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

    /* FALLTHROUGH */
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

      if (IS_USERINFO_CHAR(ch) || ch == '[' || ch == ']') {
        return state::s_req_server;
      }

      break;

    case state::s_req_path:
      if (IS_URL_CHAR(ch)) {
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
      if (IS_URL_CHAR(ch)) {
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
      if (IS_URL_CHAR(ch)) {
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
      if (IS_URL_CHAR(ch)) {
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

  /* We're in an error state. Don't bother doing anything. */
  if (HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK) {
    return 0;
  }

  if (len == 0) {
    switch (CURRENT_STATE()) {
      case state::s_body_identity_eof:
        /* Use of CALLBACK_NOTIFY() here would erroneously return 1 byte read if
         * we got paused.
         */
        CALLBACK_NOTIFY_NOADVANCE(message_complete);
        return 0;

      case state::s_dead:
      case state::s_start_req_or_res:
      case state::s_start_res:
      case state::s_start_req:
        return 0;

      default:
        SET_ERRNO(http_errno::HPE_INVALID_EOF_STATE);
        return 1;
    }
  }


  if (CURRENT_STATE() == state::s_header_field)
    header_field_mark = data;
  if (CURRENT_STATE() == state::s_header_value)
    header_value_mark = data;
  switch (CURRENT_STATE()) {
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

    if (PARSING_HEADER(CURRENT_STATE()))
      COUNT_HEADER_SIZE(1);

reexecute:
    switch (CURRENT_STATE()) {

      case state::s_dead:
        /* this state is used after a 'Connection: close' message
         * the parser will error out if it reads another message
         */
        if (LIKELY(ch == CR || ch == LF))
          break;

        SET_ERRNO(http_errno::HPE_CLOSED_CONNECTION);
        goto error;

      case state::s_start_req_or_res:
      {
        if (ch == CR || ch == LF)
          break;
        parser->flags = 0;
        parser->content_length = ULLONG_MAX;

        if (ch == 'H') {
          UPDATE_STATE(state::s_res_or_resp_H);

          CALLBACK_NOTIFY(message_begin);
        } else {
          parser->type = (unsigned int)http_parser_type::HTTP_REQUEST;
          UPDATE_STATE(state::s_start_req);
          REEXECUTE();
        }

        break;
      }

      case state::s_res_or_resp_H:
        if (ch == 'T') {
          parser->type = (unsigned int)http_parser_type::HTTP_RESPONSE;
          UPDATE_STATE(state::s_res_HT);
        } else {
          if (UNLIKELY(ch != 'E')) {
            SET_ERRNO(http_errno::HPE_INVALID_CONSTANT);
            goto error;
          }

          parser->type = (unsigned int)http_parser_type::HTTP_REQUEST;
          parser->method = http_method::HTTP_HEAD;
          parser->index = 2;
          UPDATE_STATE(state::s_req_method);
        }
        break;

      case state::s_start_res:
      {
        parser->flags = 0;
        parser->content_length = ULLONG_MAX;

        switch (ch) {
          case 'H':
            UPDATE_STATE(state::s_res_H);
            break;

          case CR:
          case LF:
            break;

          default:
            SET_ERRNO(http_errno::HPE_INVALID_CONSTANT);
            goto error;
        }

        CALLBACK_NOTIFY(message_begin);
        break;
      }

      case state::s_res_H:
        STRICT_CHECK(ch != 'T');
        UPDATE_STATE(state::s_res_HT);
        break;

      case state::s_res_HT:
        STRICT_CHECK(ch != 'T');
        UPDATE_STATE(state::s_res_HTT);
        break;

      case state::s_res_HTT:
        STRICT_CHECK(ch != 'P');
        UPDATE_STATE(state::s_res_HTTP);
        break;

      case state::s_res_HTTP:
        STRICT_CHECK(ch != '/');
        UPDATE_STATE(state::s_res_http_major);
        break;

      case state::s_res_http_major:
        if (UNLIKELY(!IS_NUM(ch))) {
          SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_major = ch - '0';
        UPDATE_STATE(state::s_res_http_dot);
        break;

      case state::s_res_http_dot:
      {
        if (UNLIKELY(ch != '.')) {
          SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        UPDATE_STATE(state::s_res_http_minor);
        break;
      }

      case state::s_res_http_minor:
        if (UNLIKELY(!IS_NUM(ch))) {
          SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_minor = ch - '0';
        UPDATE_STATE(state::s_res_http_end);
        break;

      case state::s_res_http_end:
      {
        if (UNLIKELY(ch != ' ')) {
          SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        UPDATE_STATE(state::s_res_first_status_code);
        break;
      }

      case state::s_res_first_status_code:
      {
        if (!IS_NUM(ch)) {
          if (ch == ' ') {
            break;
          }

          SET_ERRNO(http_errno::HPE_INVALID_STATUS);
          goto error;
        }
        parser->status_code = ch - '0';
        UPDATE_STATE(state::s_res_status_code);
        break;
      }

      case state::s_res_status_code:
      {
        if (!IS_NUM(ch)) {
          switch (ch) {
            case ' ':
              UPDATE_STATE(state::s_res_status_start);
              break;
            case CR:
            case LF:
              UPDATE_STATE(state::s_res_status_start);
              REEXECUTE();
              break;
            default:
              SET_ERRNO(http_errno::HPE_INVALID_STATUS);
              goto error;
          }
          break;
        }

        parser->status_code *= 10;
        parser->status_code += ch - '0';

        if (UNLIKELY(parser->status_code > 999)) {
          SET_ERRNO(http_errno::HPE_INVALID_STATUS);
          goto error;
        }

        break;
      }

      case state::s_res_status_start:
      {
        MARK(status);
        UPDATE_STATE(state::s_res_status);
        parser->index = 0;

        if (ch == CR || ch == LF)
          REEXECUTE();

        break;
      }

      case state::s_res_status:
        if (ch == CR) {
          UPDATE_STATE(state::s_res_line_almost_done);
          CALLBACK_DATA(status);
          break;
        }

        if (ch == LF) {
          UPDATE_STATE(state::s_header_field_start);
          CALLBACK_DATA(status);
          break;
        }

        break;

      case state::s_res_line_almost_done:
        STRICT_CHECK(ch != LF);
        UPDATE_STATE(state::s_header_field_start);
        break;

      case state::s_start_req:
      {
        if (ch == CR || ch == LF)
          break;
        parser->flags = 0;
        parser->content_length = ULLONG_MAX;

        if (UNLIKELY(!IS_ALPHA(ch))) {
          SET_ERRNO(http_errno::HPE_INVALID_METHOD);
          goto error;
        }

        parser->method = (http_method) 0;
        parser->index = 1;
        switch (ch) {
          case 'A': parser->method = http_method::HTTP_ACL; break;
          case 'B': parser->method = http_method::HTTP_BIND; break;
          case 'C': parser->method = http_method::HTTP_CONNECT; /* or COPY, CHECKOUT */ break;
          case 'D': parser->method = http_method::HTTP_DELETE; break;
          case 'G': parser->method = http_method::HTTP_GET; break;
          case 'H': parser->method = http_method::HTTP_HEAD; break;
          case 'L': parser->method = http_method::HTTP_LOCK; /* or LINK */ break;
          case 'M': parser->method = http_method::HTTP_MKCOL; /* or MOVE, MKACTIVITY, MERGE, M-SEARCH, MKCALENDAR */ break;
          case 'N': parser->method = http_method::HTTP_NOTIFY; break;
          case 'O': parser->method = http_method::HTTP_OPTIONS; break;
          case 'P': parser->method = http_method::HTTP_POST;
            /* or PROPFIND|PROPPATCH|PUT|PATCH|PURGE */
            break;
          case 'R': parser->method = http_method::HTTP_REPORT; /* or REBIND */ break;
          case 'S': parser->method = http_method::HTTP_SUBSCRIBE; /* or SEARCH, SOURCE */ break;
          case 'T': parser->method = http_method::HTTP_TRACE; break;
          case 'U': parser->method = http_method::HTTP_UNLOCK; /* or UNSUBSCRIBE, UNBIND, UNLINK */ break;
          default:
            SET_ERRNO(http_errno::HPE_INVALID_METHOD);
            goto error;
        }
        UPDATE_STATE(state::s_req_method);

        CALLBACK_NOTIFY(message_begin);

        break;
      }

      case state::s_req_method:
      {
        const char *matcher;
        if (UNLIKELY(ch == '\0')) {
          SET_ERRNO(http_errno::HPE_INVALID_METHOD);
          goto error;
        }

        matcher = method_strings[parser->method];
        if (ch == ' ' && matcher[parser->index] == '\0') {
          UPDATE_STATE(state::s_req_spaces_before_url);
        } else if (ch == matcher[parser->index]) {
          ; /* nada */
        } else if ((ch >= 'A' && ch <= 'Z') || ch == '-') {

          switch (parser->method << 16 | parser->index << 8 | ch) {
#define XX(meth, pos, ch, new_meth) \
            case ((size_t)http_method::HTTP_##meth << 16 | pos << 8 | ch): \
              parser->method = http_method::HTTP_##new_meth; break;

            XX(POST,      1, 'U', PUT)
            XX(POST,      1, 'A', PATCH)
            XX(POST,      1, 'R', PROPFIND)
            XX(PUT,       2, 'R', PURGE)
            XX(CONNECT,   1, 'H', CHECKOUT)
            XX(CONNECT,   2, 'P', COPY)
            XX(MKCOL,     1, 'O', MOVE)
            XX(MKCOL,     1, 'E', MERGE)
            XX(MKCOL,     1, '-', MSEARCH)
            XX(MKCOL,     2, 'A', MKACTIVITY)
            XX(MKCOL,     3, 'A', MKCALENDAR)
            XX(SUBSCRIBE, 1, 'E', SEARCH)
            XX(SUBSCRIBE, 1, 'O', SOURCE)
            XX(REPORT,    2, 'B', REBIND)
            XX(PROPFIND,  4, 'P', PROPPATCH)
            XX(LOCK,      1, 'I', LINK)
            XX(UNLOCK,    2, 'S', UNSUBSCRIBE)
            XX(UNLOCK,    2, 'B', UNBIND)
            XX(UNLOCK,    3, 'I', UNLINK)
#undef XX
            default:
              SET_ERRNO(http_errno::HPE_INVALID_METHOD);
              goto error;
          }
        } else {
          SET_ERRNO(http_errno::HPE_INVALID_METHOD);
          goto error;
        }

        ++parser->index;
        break;
      }

      case state::s_req_spaces_before_url:
      {
        if (ch == ' ') break;

        MARK(url);
        if (parser->method == (unsigned int)http_method::HTTP_CONNECT) {
          UPDATE_STATE(state::s_req_server_start);
        }

        UPDATE_STATE(parse_url_char(CURRENT_STATE(), ch));
        if (UNLIKELY(CURRENT_STATE() == state::s_dead)) {
          SET_ERRNO(http_errno::HPE_INVALID_URL);
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
          case CR:
          case LF:
            SET_ERRNO(http_errno::HPE_INVALID_URL);
            goto error;
          default:
            UPDATE_STATE(parse_url_char(CURRENT_STATE(), ch));
            if (UNLIKELY(CURRENT_STATE() == state::s_dead)) {
              SET_ERRNO(http_errno::HPE_INVALID_URL);
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
            UPDATE_STATE(state::s_req_http_start);
            CALLBACK_DATA(url);
            break;
          case CR:
          case LF:
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE((ch == CR) ?
              state::s_req_line_almost_done :
              state::s_header_field_start);
            CALLBACK_DATA(url);
            break;
          default:
            UPDATE_STATE(parse_url_char(CURRENT_STATE(), ch));
            if (UNLIKELY(CURRENT_STATE() == state::s_dead)) {
              SET_ERRNO(http_errno::HPE_INVALID_URL);
              goto error;
            }
        }
        break;
      }

      case state::s_req_http_start:
        switch (ch) {
          case 'H':
            UPDATE_STATE(state::s_req_http_H);
            break;
          case ' ':
            break;
          default:
            SET_ERRNO(http_errno::HPE_INVALID_CONSTANT);
            goto error;
        }
        break;

      case state::s_req_http_H:
        STRICT_CHECK(ch != 'T');
        UPDATE_STATE(state::s_req_http_HT);
        break;

      case state::s_req_http_HT:
        STRICT_CHECK(ch != 'T');
        UPDATE_STATE(state::s_req_http_HTT);
        break;

      case state::s_req_http_HTT:
        STRICT_CHECK(ch != 'P');
        UPDATE_STATE(state::s_req_http_HTTP);
        break;

      case state::s_req_http_HTTP:
        STRICT_CHECK(ch != '/');
        UPDATE_STATE(state::s_req_http_major);
        break;

      case state::s_req_http_major:
        if (UNLIKELY(!IS_NUM(ch))) {
          SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_major = ch - '0';
        UPDATE_STATE(state::s_req_http_dot);
        break;

      case state::s_req_http_dot:
      {
        if (UNLIKELY(ch != '.')) {
          SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        UPDATE_STATE(state::s_req_http_minor);
        break;
      }

      case state::s_req_http_minor:
        if (UNLIKELY(!IS_NUM(ch))) {
          SET_ERRNO(http_errno::HPE_INVALID_VERSION);
          goto error;
        }

        parser->http_minor = ch - '0';
        UPDATE_STATE(state::s_req_http_end);
        break;

      case state::s_req_http_end:
      {
        if (ch == CR) {
          UPDATE_STATE(state::s_req_line_almost_done);
          break;
        }

        if (ch == LF) {
          UPDATE_STATE(state::s_header_field_start);
          break;
        }

        SET_ERRNO(http_errno::HPE_INVALID_VERSION);
        goto error;
        break;
      }

      /* end of request line */
      case state::s_req_line_almost_done:
      {
        if (UNLIKELY(ch != LF)) {
          SET_ERRNO(http_errno::HPE_LF_EXPECTED);
          goto error;
        }

        UPDATE_STATE(state::s_header_field_start);
        break;
      }

      case state::s_header_field_start:
      {
        if (ch == CR) {
          UPDATE_STATE(state::s_headers_almost_done);
          break;
        }

        if (ch == LF) {
          /* they might be just sending \n instead of \r\n so this would be
           * the second \n to denote the end of headers*/
          UPDATE_STATE(state::s_headers_almost_done);
          REEXECUTE();
        }

        c = TOKEN(ch);

        if (UNLIKELY(!c)) {
          SET_ERRNO(http_errno::HPE_INVALID_HEADER_TOKEN);
          goto error;
        }

        MARK(header_field);

        parser->index = 0;
        UPDATE_STATE(state::s_header_field);

        switch (c) {
          case 'c':
            parser->header_state = header_states::h_C;
            break;

          case 'p':
            parser->header_state = header_states::h_matching_proxy_connection;
            break;

          case 't':
            parser->header_state = header_states::h_matching_transfer_encoding;
            break;

          case 'u':
            parser->header_state = header_states::h_matching_upgrade;
            break;

          default:
            parser->header_state = header_states::h_general;
            break;
        }
        break;
      }

      case state::s_header_field:
      {
        const char* start = p;
        for (; p != data + len; ++p) {
          ch = *p;
          c = TOKEN(ch);

          if (!c)
            break;

          switch (parser->header_state) {
            case header_states::h_general:
              break;

            case header_states::h_C:
              parser->index++;
              parser->header_state = (c == 'o' ? header_states::h_CO : header_states::h_general);
              break;

            case header_states::h_CO:
              parser->index++;
              parser->header_state = (c == 'n' ? header_states::h_CON : header_states::h_general);
              break;

            case header_states::h_CON:
              parser->index++;
              switch (c) {
                case 'n':
                  parser->header_state = header_states::h_matching_connection;
                  break;
                case 't':
                  parser->header_state = header_states::h_matching_content_length;
                  break;
                default:
                  parser->header_state = header_states::h_general;
                  break;
              }
              break;

            /* connection */

            case header_states::h_matching_connection:
              parser->index++;
              if (parser->index > sizeof(CONNECTION)-1
                  || c != CONNECTION[parser->index]) {
                parser->header_state = header_states::h_general;
              } else if (parser->index == sizeof(CONNECTION)-2) {
                parser->header_state = header_states::h_connection;
              }
              break;

            /* proxy-connection */

            case header_states::h_matching_proxy_connection:
              parser->index++;
              if (parser->index > sizeof(PROXY_CONNECTION)-1
                  || c != PROXY_CONNECTION[parser->index]) {
                parser->header_state = header_states::h_general;
              } else if (parser->index == sizeof(PROXY_CONNECTION)-2) {
                parser->header_state = header_states::h_connection;
              }
              break;

            /* content-length */

            case header_states::h_matching_content_length:
              parser->index++;
              if (parser->index > sizeof(CONTENT_LENGTH)-1
                  || c != CONTENT_LENGTH[parser->index]) {
                parser->header_state = header_states::h_general;
              } else if (parser->index == sizeof(CONTENT_LENGTH)-2) {
                parser->header_state = header_states::h_content_length;
              }
              break;

            /* transfer-encoding */

            case header_states::h_matching_transfer_encoding:
              parser->index++;
              if (parser->index > sizeof(TRANSFER_ENCODING)-1
                  || c != TRANSFER_ENCODING[parser->index]) {
                parser->header_state = header_states::h_general;
              } else if (parser->index == sizeof(TRANSFER_ENCODING)-2) {
                parser->header_state = header_states::h_transfer_encoding;
              }
              break;

            /* upgrade */

            case header_states::h_matching_upgrade:
              parser->index++;
              if (parser->index > sizeof(UPGRADE)-1
                  || c != UPGRADE[parser->index]) {
                parser->header_state = header_states::h_general;
              } else if (parser->index == sizeof(UPGRADE)-2) {
                parser->header_state = header_states::h_upgrade;
              }
              break;

            case header_states::h_connection:
            case header_states::h_content_length:
            case header_states::h_transfer_encoding:
            case header_states::h_upgrade:
              if (ch != ' ') parser->header_state = header_states::h_general;
              break;

            default:
              assert(0 && "Unknown header_state");
              break;
          }
        }

        COUNT_HEADER_SIZE(p - start);

        if (p == data + len) {
          --p;
          break;
        }

        if (ch == ':') {
          UPDATE_STATE(state::s_header_value_discard_ws);
          CALLBACK_DATA(header_field);
          break;
        }

        SET_ERRNO(http_errno::HPE_INVALID_HEADER_TOKEN);
        goto error;
      }

      case state::s_header_value_discard_ws:
        if (ch == ' ' || ch == '\t') break;

        if (ch == CR) {
          UPDATE_STATE(state::s_header_value_discard_ws_almost_done);
          break;
        }

        if (ch == LF) {
          UPDATE_STATE(state::s_header_value_discard_lws);
          break;
        }

        /* FALLTHROUGH */

      case state::s_header_value_start:
      {
        MARK(header_value);

        UPDATE_STATE(state::s_header_value);
        parser->index = 0;

        c = LOWER(ch);

        switch (parser->header_state) {
          case header_states::h_upgrade:
            parser->flags |= (int)flags::F_UPGRADE;
            parser->header_state = header_states::h_general;
            break;

          case header_states::h_transfer_encoding:
            /* looking for 'Transfer-Encoding: chunked' */
            if ('c' == c) {
              parser->header_state = header_states::h_matching_transfer_encoding_chunked;
            } else {
              parser->header_state = header_states::h_general;
            }
            break;

          case header_states::h_content_length:
            if (UNLIKELY(!IS_NUM(ch))) {
              SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
              goto error;
            }

            if (parser->flags & (int)flags::F_CONTENTLENGTH) {
              SET_ERRNO(http_errno::HPE_UNEXPECTED_CONTENT_LENGTH);
              goto error;
            }

            parser->flags |= (int)flags::F_CONTENTLENGTH;
            parser->content_length = ch - '0';
            parser->header_state = header_states::h_content_length_num;
            break;

          case header_states::h_connection:
            /* looking for 'Connection: keep-alive' */
            if (c == 'k') {
              parser->header_state = header_states::h_matching_connection_keep_alive;
            /* looking for 'Connection: close' */
            } else if (c == 'c') {
              parser->header_state = header_states::h_matching_connection_close;
            } else if (c == 'u') {
              parser->header_state = header_states::h_matching_connection_upgrade;
            } else {
              parser->header_state = header_states::h_matching_connection_token;
            }
            break;

          /* Multi-value `Connection` header */
          case header_states::h_matching_connection_token_start:
            break;

          default:
            parser->header_state = header_states::h_general;
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
          if (ch == CR) {
            UPDATE_STATE(state::s_header_almost_done);
            parser->header_state = h_state;
            CALLBACK_DATA(header_value);
            break;
          }

          if (ch == LF) {
            UPDATE_STATE(state::s_header_almost_done);
            COUNT_HEADER_SIZE(p - start);
            parser->header_state = h_state;
            CALLBACK_DATA_NOADVANCE(header_value);
            REEXECUTE();
          }

          if (!lenient && !IS_HEADER_CHAR(ch)) {
            SET_ERRNO(http_errno::HPE_INVALID_HEADER_TOKEN);
            goto error;
          }

          c = LOWER(ch);

          switch (h_state) {
            case header_states::h_general:
            {
              const char* p_cr;
              const char* p_lf;
              size_t limit = data + len - p;

              limit = MIN(limit, HTTP_MAX_HEADER_SIZE);

              p_cr = (const char*) memchr(p, CR, limit);
              p_lf = (const char*) memchr(p, LF, limit);
              if (p_cr != NULL) {
                if (p_lf != NULL && p_cr >= p_lf)
                  p = p_lf;
                else
                  p = p_cr;
              } else if (UNLIKELY(p_lf != NULL)) {
                p = p_lf;
              } else {
                p = data + len;
              }
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
              /* FALLTHROUGH */

            case header_states::h_content_length_num:
            {
              uint64_t t;

              if (ch == ' ') {
                h_state = header_states::h_content_length_ws;
                break;
              }

              if (UNLIKELY(!IS_NUM(ch))) {
                SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
                parser->header_state = h_state;
                goto error;
              }

              t = parser->content_length;
              t *= 10;
              t += ch - '0';

              /* Overflow? Test against a conservative limit for simplicity. */
              if (UNLIKELY((ULLONG_MAX - 10) / 10 < parser->content_length)) {
                SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
                parser->header_state = h_state;
                goto error;
              }

              parser->content_length = t;
              break;
            }

            case header_states::h_content_length_ws:
              if (ch == ' ') break;
              SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
              parser->header_state = h_state;
              goto error;

            /* Transfer-Encoding: chunked */
            case header_states::h_matching_transfer_encoding_chunked:
              parser->index++;
              if (parser->index > sizeof(CHUNKED)-1
                  || c != CHUNKED[parser->index]) {
                h_state = header_states::h_general;
              } else if (parser->index == sizeof(CHUNKED)-2) {
                h_state = header_states::h_transfer_encoding_chunked;
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
              } else if (STRICT_TOKEN(c)) {
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
              if (parser->index > sizeof(KEEP_ALIVE)-1
                  || c != KEEP_ALIVE[parser->index]) {
                h_state = header_states::h_matching_connection_token;
              } else if (parser->index == sizeof(KEEP_ALIVE)-2) {
                h_state = header_states::h_connection_keep_alive;
              }
              break;

            /* looking for 'Connection: close' */
            case header_states::h_matching_connection_close:
              parser->index++;
              if (parser->index > sizeof(CLOSE)-1 || c != CLOSE[parser->index]) {
                h_state = header_states::h_matching_connection_token;
              } else if (parser->index == sizeof(CLOSE)-2) {
                h_state = header_states::h_connection_close;
              }
              break;

            /* looking for 'Connection: upgrade' */
            case header_states::h_matching_connection_upgrade:
              parser->index++;
              if (parser->index > sizeof(UPGRADE) - 1 ||
                  c != UPGRADE[parser->index]) {
                h_state = header_states::h_matching_connection_token;
              } else if (parser->index == sizeof(UPGRADE)-2) {
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
              if (ch != ' ') h_state = header_states::h_general;
              break;

            case header_states::h_connection_keep_alive:
            case header_states::h_connection_close:
            case header_states::h_connection_upgrade:
              if (ch == ',') {
                if (h_state == header_states::h_connection_keep_alive) {
                  parser->flags |= (int)flags::F_CONNECTION_KEEP_ALIVE;
                } else if (h_state == header_states::h_connection_close) {
                  parser->flags |= (int)flags::F_CONNECTION_CLOSE;
                } else if (h_state == header_states::h_connection_upgrade) {
                  parser->flags |= (int)flags::F_CONNECTION_UPGRADE;
                }
                h_state = header_states::h_matching_connection_token_start;
                parser->index = 0;
              } else if (ch != ' ') {
                h_state = header_states::h_matching_connection_token;
              }
              break;

            default:
              UPDATE_STATE(state::s_header_value);
              h_state = header_states::h_general;
              break;
          }
        }
        parser->header_state = h_state;

        COUNT_HEADER_SIZE(p - start);

        if (p == data + len)
          --p;
        break;
      }

      case state::s_header_almost_done:
      {
        if (UNLIKELY(ch != LF)) {
          SET_ERRNO(http_errno::HPE_LF_EXPECTED);
          goto error;
        }

        UPDATE_STATE(state::s_header_value_lws);
        break;
      }

      case state::s_header_value_lws:
      {
        if (ch == ' ' || ch == '\t') {
          UPDATE_STATE(state::s_header_value_start);
          REEXECUTE();
        }

        /* finished the header */
        switch (parser->header_state) {
          case header_states::h_connection_keep_alive:
            parser->flags |= (int)flags::F_CONNECTION_KEEP_ALIVE;
            break;
          case header_states::h_connection_close:
            parser->flags |= (int)flags::F_CONNECTION_CLOSE;
            break;
          case header_states::h_transfer_encoding_chunked:
            parser->flags |= (int)flags::F_CHUNKED;
            break;
          case header_states::h_connection_upgrade:
            parser->flags |= (int)flags::F_CONNECTION_UPGRADE;
            break;
          default:
            break;
        }

        UPDATE_STATE(state::s_header_field_start);
        REEXECUTE();
      }

      case state::s_header_value_discard_ws_almost_done:
      {
        STRICT_CHECK(ch != LF);
        UPDATE_STATE(state::s_header_value_discard_lws);
        break;
      }

      case state::s_header_value_discard_lws:
      {
        if (ch == ' ' || ch == '\t') {
          UPDATE_STATE(state::s_header_value_discard_ws);
          break;
        } else {
          switch (parser->header_state) {
            case header_states::h_connection_keep_alive:
              parser->flags |= (int)flags::F_CONNECTION_KEEP_ALIVE;
              break;
            case header_states::h_connection_close:
              parser->flags |= (int)flags::F_CONNECTION_CLOSE;
              break;
            case header_states::h_connection_upgrade:
              parser->flags |= (int)flags::F_CONNECTION_UPGRADE;
              break;
            case header_states::h_transfer_encoding_chunked:
              parser->flags |= (int)flags::F_CHUNKED;
              break;
            default:
              break;
          }

          /* header value was empty */
          MARK(header_value);
          UPDATE_STATE(state::s_header_field_start);
          CALLBACK_DATA_NOADVANCE(header_value);
          REEXECUTE();
        }
      }

      case state::s_headers_almost_done:
      {
        STRICT_CHECK(ch != LF);

        if (parser->flags & (int)flags::F_TRAILING) {
          /* End of a chunked request */
          UPDATE_STATE(state::s_message_done);
          CALLBACK_NOTIFY_NOADVANCE(chunk_complete);
          REEXECUTE();
        }

        /* Cannot use chunked encoding and a content-length header together
           per the HTTP specification. */
        if ((parser->flags & (int)flags::F_CHUNKED) &&
            (parser->flags & (int)flags::F_CONTENTLENGTH)) {
          SET_ERRNO(http_errno::HPE_UNEXPECTED_CONTENT_LENGTH);
          goto error;
        }

        UPDATE_STATE(state::s_headers_done);

        /* Set this here so that on_headers_complete() callbacks can see it */
        if ((parser->flags & (int)flags::F_UPGRADE) &&
            (parser->flags & (int)flags::F_CONNECTION_UPGRADE)) {
          /* For responses, "Upgrade: foo" and "Connection: upgrade" are
           * mandatory only when it is a 101 Switching Protocols response,
           * otherwise it is purely informational, to announce support.
           */
          parser->upgrade =
              (parser->type == (unsigned int)http_parser_type::HTTP_REQUEST || parser->status_code == 101);
        } else {
          parser->upgrade = (parser->method == (unsigned int)http_method::HTTP_CONNECT);
        }

        /* Here we call the headers_complete callback. This is somewhat
         * different than other callbacks because if the user returns 1, we
         * will interpret that as saying that this message has no body. This
         * is needed for the annoying case of recieving a response to a HEAD
         * request.
         *
         * We'd like to use CALLBACK_NOTIFY_NOADVANCE() here but we cannot, so
         * we have to simulate it by handling a change in errno below.
         */
        if (settings->on_headers_complete) {
          switch (settings->on_headers_complete(parser, user_data)) {
            case 0:
              break;

            case 2:
              parser->upgrade = 1;

            /* FALLTHROUGH */
            case 1:
              parser->flags |= (int)flags::F_SKIPBODY;
              break;

            default:
              SET_ERRNO(http_errno::HPE_CB_headers_complete);
              RETURN(p - data); /* Error */
          }
        }

        if (HTTP_PARSER_ERRNO(parser) != http_errno::HPE_OK) {
          RETURN(p - data);
        }

        REEXECUTE();
      }

      case state::s_headers_done:
      {
        int hasBody;
        STRICT_CHECK(ch != LF);

        parser->nread = 0;

        hasBody = parser->flags & (int)flags::F_CHUNKED ||
          (parser->content_length > 0 && parser->content_length != ULLONG_MAX);
        if (parser->upgrade && (parser->method == (unsigned int)http_method::HTTP_CONNECT ||
                                (parser->flags & (int)flags::F_SKIPBODY) || !hasBody)) {
          /* Exit, the rest of the message is in a different protocol. */
          UPDATE_STATE(NEW_MESSAGE());
          CALLBACK_NOTIFY(message_complete);
          RETURN((p - data) + 1);
        }

        if (parser->flags & (int)flags::F_SKIPBODY) {
          UPDATE_STATE(NEW_MESSAGE());
          CALLBACK_NOTIFY(message_complete);
        } else if (parser->flags & (int)flags::F_CHUNKED) {
          /* chunked encoding - ignore Content-Length header */
          UPDATE_STATE(state::s_chunk_size_start);
        } else {
          if (parser->content_length == 0) {
            /* Content-Length header given but zero: Content-Length: 0\r\n */
            UPDATE_STATE(NEW_MESSAGE());
            CALLBACK_NOTIFY(message_complete);
          } else if (parser->content_length != ULLONG_MAX) {
            /* Content-Length header given and non-zero */
            UPDATE_STATE(state::s_body_identity);
          } else {
            if (!http_message_needs_eof(parser)) {
              /* Assume content-length 0 - read the next */
              UPDATE_STATE(NEW_MESSAGE());
              CALLBACK_NOTIFY(message_complete);
            } else {
              /* Read body until EOF */
              UPDATE_STATE(state::s_body_identity_eof);
            }
          }
        }

        break;
      }

      case state::s_body_identity:
      {
        uint64_t to_read = MIN(parser->content_length,
                               (uint64_t) ((data + len) - p));

        assert(parser->content_length != 0
            && parser->content_length != ULLONG_MAX);

        /* The difference between advancing content_length and p is because
         * the latter will automaticaly advance on the next loop iteration.
         * Further, if content_length ends up at 0, we want to see the last
         * byte again for our message complete callback.
         */
        MARK(body);
        parser->content_length -= to_read;
        p += to_read - 1;

        if (parser->content_length == 0) {
          UPDATE_STATE(state::s_message_done);

          /* Mimic CALLBACK_DATA_NOADVANCE() but with one extra byte.
           *
           * The alternative to doing this is to wait for the next byte to
           * trigger the data callback, just as in every other case. The
           * problem with this is that this makes it difficult for the test
           * harness to distinguish between complete-on-EOF and
           * complete-on-length. It's not clear that this distinction is
           * important for applications, but let's keep it for now.
           */
          CALLBACK_DATA_(body, p - body_mark + 1, p - data);
          REEXECUTE();
        }

        break;
      }

      /* read until EOF */
      case state::s_body_identity_eof:
        MARK(body);
        p = data + len - 1;

        break;

      case state::s_message_done:
        UPDATE_STATE(NEW_MESSAGE());
        CALLBACK_NOTIFY(message_complete);
        if (parser->upgrade) {
          /* Exit, the rest of the message is in a different protocol. */
          RETURN((p - data) + 1);
        }
        break;

      case state::s_chunk_size_start:
      {
        assert(parser->nread == 1);
        assert(parser->flags & (int)flags::F_CHUNKED);

        unhex_val = unhex[(unsigned char)ch];
        if (UNLIKELY(unhex_val == -1)) {
          SET_ERRNO(http_errno::HPE_INVALID_CHUNK_SIZE);
          goto error;
        }

        parser->content_length = unhex_val;
        UPDATE_STATE(state::s_chunk_size);
        break;
      }

      case state::s_chunk_size:
      {
        uint64_t t;

        assert(parser->flags & (int)flags::F_CHUNKED);

        if (ch == CR) {
          UPDATE_STATE(state::s_chunk_size_almost_done);
          break;
        }

        unhex_val = unhex[(unsigned char)ch];

        if (unhex_val == -1) {
          if (ch == ';' || ch == ' ') {
            UPDATE_STATE(state::s_chunk_parameters);
            break;
          }

          SET_ERRNO(http_errno::HPE_INVALID_CHUNK_SIZE);
          goto error;
        }

        t = parser->content_length;
        t *= 16;
        t += unhex_val;

        /* Overflow? Test against a conservative limit for simplicity. */
        if (UNLIKELY((ULLONG_MAX - 16) / 16 < parser->content_length)) {
          SET_ERRNO(http_errno::HPE_INVALID_CONTENT_LENGTH);
          goto error;
        }

        parser->content_length = t;
        break;
      }

      case state::s_chunk_parameters:
      {
        assert(parser->flags & (int)flags::F_CHUNKED);
        /* just ignore this shit. TODO check for overflow */
        if (ch == CR) {
          UPDATE_STATE(state::s_chunk_size_almost_done);
          break;
        }
        break;
      }

      case state::s_chunk_size_almost_done:
      {
        assert(parser->flags & (int)flags::F_CHUNKED);
        STRICT_CHECK(ch != LF);

        parser->nread = 0;

        if (parser->content_length == 0) {
          parser->flags |= (int)flags::F_TRAILING;
          UPDATE_STATE(state::s_header_field_start);
        } else {
          UPDATE_STATE(state::s_chunk_data);
        }
        CALLBACK_NOTIFY(chunk_header);
        break;
      }

      case state::s_chunk_data:
      {
        uint64_t to_read = MIN(parser->content_length,
                               (uint64_t) ((data + len) - p));

        assert(parser->flags & (int)flags::F_CHUNKED);
        assert(parser->content_length != 0
            && parser->content_length != ULLONG_MAX);

        /* See the explanation in state::s_body_identity for why the content
         * length and data pointers are managed this way.
         */
        MARK(body);
        parser->content_length -= to_read;
        p += to_read - 1;

        if (parser->content_length == 0) {
          UPDATE_STATE(state::s_chunk_data_almost_done);
        }

        break;
      }

      case state::s_chunk_data_almost_done:
        assert(parser->flags & (int)flags::F_CHUNKED);
        assert(parser->content_length == 0);
        STRICT_CHECK(ch != CR);
        UPDATE_STATE(state::s_chunk_data_done);
        CALLBACK_DATA(body);
        break;

      case state::s_chunk_data_done:
        assert(parser->flags & (int)flags::F_CHUNKED);
        STRICT_CHECK(ch != LF);
        parser->nread = 0;
        UPDATE_STATE(state::s_chunk_size_start);
        CALLBACK_NOTIFY(chunk_complete);
        break;

      default:
        assert(0 && "unhandled state");
        SET_ERRNO(http_errno::HPE_INVALID_INTERNAL_STATE);
        goto error;
    }
  }

  /* Run callbacks for any marks that we have leftover after we ran our of
   * bytes. There should be at most one of these set, so it's OK to invoke
   * them in series (unset marks will not result in callbacks).
   *
   * We use the NOADVANCE() variety of callbacks here because 'p' has already
   * overflowed 'data' and this allows us to correct for the off-by-one that
   * we'd otherwise have (since CALLBACK_DATA() is meant to be run with a 'p'
   * value that's in-bounds).
   */

  assert(((header_field_mark ? 1 : 0) +
          (header_value_mark ? 1 : 0) +
          (url_mark ? 1 : 0)  +
          (body_mark ? 1 : 0) +
          (status_mark ? 1 : 0)) <= 1);

  CALLBACK_DATA_NOADVANCE(header_field);
  CALLBACK_DATA_NOADVANCE(header_value);
  CALLBACK_DATA_NOADVANCE(url);
  CALLBACK_DATA_NOADVANCE(body);
  CALLBACK_DATA_NOADVANCE(status);

  RETURN(len);

error:
  if (HTTP_PARSER_ERRNO(parser) == http_errno::HPE_OK) {
    SET_ERRNO(http_errno::HPE_UNKNOWN);
  }

  RETURN(p - data);
}

template<typename = void>
const char *
http_method_str (http_method m)
{
  return ELEM_AT(method_strings, m, "<unknown>");
}

template<typename = void>
const char *
http_status_str(http_status s)
{
	switch (s) {
#define XX(num, name, string) case http_status::HTTP_STATUS_##name: return #string;
		HTTP_STATUS_MAP(XX)
#undef XX
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
  parser->type = t;
  parser->state = (t == http_parser_type::HTTP_REQUEST ? state::s_start_req : (t == http_parser_type::HTTP_RESPONSE ? state::s_start_res : state::s_start_req_or_res));
  parser->http_errno = http_errno::HPE_OK;
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
  assert(((size_t) err) < ARRAY_SIZE(http_strerror_tab));
  return http_strerror_tab[(size_t)err].name;
}

template<typename = void>
const char *
http_errno_description(http_errno err) {
  assert(((size_t) err) < ARRAY_SIZE(http_strerror_tab));
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

      if (IS_USERINFO_CHAR(ch)) {
        return http_host_state::s_http_userinfo;
      }
      break;

    case http_host_state::s_http_host_start:
      if (ch == '[') {
        return http_host_state::s_http_host_v6_start;
      }

      if (IS_HOST_CHAR(ch)) {
        return http_host_state::s_http_host;
      }

      break;

    case http_host_state::s_http_host:
      if (IS_HOST_CHAR(ch)) {
        return http_host_state::s_http_host;
      }

    /* FALLTHROUGH */
    case http_host_state::s_http_host_v6_end:
      if (ch == ':') {
        return http_host_state::s_http_host_port_start;
      }

      break;

    case http_host_state::s_http_host_v6:
      if (ch == ']') {
        return http_host_state::s_http_host_v6_end;
      }

    /* FALLTHROUGH */
    case http_host_state::s_http_host_v6_start:
      if (IS_HEX(ch) || ch == ':' || ch == '.') {
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

    /* FALLTHROUGH */
    case http_host_state::s_http_host_v6_zone_start:
      /* RFC 6874 Zone ID consists of 1*( unreserved / pct-encoded) */
      if (IS_ALPHANUM(ch) || ch == '%' || ch == '.' || ch == '-' || ch == '_' ||
          ch == '~') {
        return http_host_state::s_http_host_v6_zone;
      }
      break;

    case http_host_state::s_http_host_port:
    case http_host_state::s_http_host_port_start:
      if (IS_NUM(ch)) {
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

      /* FALLTHROUGH */
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
    const char* p;
    const char* end;
    unsigned long v;

    off = u->field_data[(int)url_fields::UF_PORT].off;
    len = u->field_data[(int)url_fields::UF_PORT].len;
    end = buf + off + len;

    /* NOTE: The characters are already validated and are in the [0-9] range */
    assert(size_t(off + len) <= buflen && "Port number overflow");
    v = 0;
    for (p = buf + off; p < end; ++p) {
      v *= 10;
      v += *p - '0';

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
    SET_ERRNO((paused) ? http_errno::HPE_PAUSED : http_errno::HPE_OK);
  } else {
    assert(0 && "Attempting to pause parser in error state");
  }
}

template<typename = void>
int
http_body_is_final(const struct http_parser *parser) {
    return parser->state == (unsigned int)state::s_message_done;
}

template<typename = void>
unsigned long
http_parser_version(void) {
  return HTTP_PARSER_VERSION_MAJOR * 0x10000 |
         HTTP_PARSER_VERSION_MINOR * 0x00100 |
         HTTP_PARSER_VERSION_PATCH * 0x00001;
}

//#ifdef __cplusplus
//}
//#endif
}

#if defined(__GNUC__) || defined(__GNUG__)
#	pragma GCC diagnostic pop
#endif

#endif
