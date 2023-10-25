#ifndef ASIO2_ENABLE_LOG
#define ASIO2_ENABLE_LOG
#endif

#ifndef ASIO2_ENABLE_SSL
#define ASIO2_ENABLE_SSL
#endif

#include "unit_test.hpp"

#include <asio2/asio2.hpp>

#include <asio2/bho/mysql.hpp>
#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_with_diagnostics.hpp>
#include <asio2/bho/mysql/handshake_params.hpp>
#include <asio2/bho/mysql/row_view.hpp>
#include <asio2/bho/mysql/tcp_ssl.hpp>
#include <asio2/bho/mysql/throw_on_error.hpp>

#include <asio/as_tuple.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ssl/context.hpp>
#include <asio/use_awaitable.hpp>

#include <exception>
#include <iostream>

#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_code.hpp>
#include <asio2/bho/mysql/handshake_params.hpp>
#include <asio2/bho/mysql/row_view.hpp>
#include <asio2/bho/mysql/tcp_ssl.hpp>
#include <asio2/bho/mysql/throw_on_error.hpp>

#include <asio/as_tuple.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ssl/context.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace mysql = bho::mysql;

using mysql::error_code;

void print_employee(mysql::row_view employee)
{
    std::cout << "Employee '" << employee.at(0) << " "   // first_name (string)
        << employee.at(1) << "' earns "            // last_name  (string)
        << employee.at(2) << " dollars yearly\n";  // salary     (double)
}

#ifdef ASIO_HAS_CO_AWAIT

#include <asio/experimental/awaitable_operators.hpp>

using namespace asio::experimental::awaitable_operators;
using asio::use_awaitable;
using mysql::error_code;

constexpr std::chrono::milliseconds TIMEOUT(8000);

/**
 * Helper functions to check whether an async operation, launched in parallel with
 * a timer, was successful, resulted in an error or timed out. The timer is always the first operation.
 * If the variant holds the first alternative, the timer fired before
 * the async operation completed, which means a timeout. We'll be using as_tuple with use_awaitable to be able
 * to use mysql::throw_on_error and include server diagnostics in the thrown exceptions.
 */
template <class T>
T check_error(
    std::variant<std::monostate, std::tuple<error_code, T>>&& op_result,
    const mysql::diagnostics& diag = {}
)
{
    if (op_result.index() == 0)
    {
        throw std::runtime_error("Operation timed out");
    }
    auto [ec, res] = std::get<1>(std::move(op_result));
    mysql::throw_on_error(ec, diag);
    return res;
}

void check_error(
    const std::variant<std::monostate, std::tuple<error_code>>& op_result,
    const mysql::diagnostics& diag
)
{
    if (op_result.index() == 0)
    {
        throw std::runtime_error("Operation timed out");
    }
    auto [ec] = std::get<1>(op_result);
    mysql::throw_on_error(ec, diag);
}

// Using this completion token instead of plain use_awaitable prevents
// co_await from throwing exceptions. Instead, co_await will return a std::tuple<error_code>
// with a non-zero code on error. We will then use mysql::throw_on_error
// to throw exceptions with embedded diagnostics, if available. If you
// employ plain use_awaitable, you will get boost::system::system_error exceptions
// instead of mysql::error_with_diagnostics exceptions. This is a limitation of use_awaitable.
constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);

/**
 * We use Boost.Asio's cancellation capabilities to implement timeouts for our
 * asynchronous operations. This is not something specific to Boost.MySQL, and
 * can be used with any other asynchronous operation that follows Asio's model.
 *
 * Each time we invoke an asynchronous operation, we also call timer_type::async_wait.
 * We then use Asio's overload for operator || to run the timer wait and the async operation
 * in parallel. Once the first of them finishes, the other operation is cancelled
 * (the behavior is similar to JavaScripts's Promise.race).
 * If we co_await the awaitable returned by operator ||, we get a std::variant<std::monostate, T>,
 * where T is the async operation's result type. If the timer wait finishes first (we have a
 * timeout), the variant will hold the std::monostate at index 0; otherwise, it will have the async
 * operation's result at index 1. The function check_error throws an exception in the case of
 * timeout and extracts the operation's result otherwise.
 *
 * If any of the MySQL specific operations result in a timeout, the connection is left
 * in an unspecified state. You should close it and re-open it to get it working again.
 */
asio::awaitable<void> coro_main2(
    mysql::tcp_ssl_connection& conn,
    asio::ip::tcp::resolver& resolver,
    asio::steady_timer& timer,
    const mysql::handshake_params& params,
    const char* hostname,
    const char* company_id
)
{
    mysql::diagnostics diag;

    // Resolve hostname
    timer.expires_after(TIMEOUT);
    auto endpoints = check_error(co_await(
        timer.async_wait(use_awaitable) ||
        resolver.async_resolve(hostname, mysql::default_port_string, tuple_awaitable)
        ));

    // Connect to server. Note that we need to reset the timer before using it again.
    timer.expires_after(TIMEOUT);
    auto op_result = co_await(
        timer.async_wait(use_awaitable) ||
        conn.async_connect(*endpoints.begin(), params, diag, tuple_awaitable)
        );
    check_error(op_result, diag);

    // We will be using company_id, which is untrusted user input, so we will use a prepared
    // statement.
    auto stmt_op_result = co_await(
        timer.async_wait(use_awaitable) ||
        conn.async_prepare_statement(
            "SELECT first_name, last_name, salary FROM employee WHERE company_id = ?",
            diag,
            tuple_awaitable
        )
        );
    mysql::statement stmt = check_error(std::move(stmt_op_result), diag);

    // Execute the statement
    mysql::results result;
    timer.expires_after(TIMEOUT);
    op_result = co_await(
        timer.async_wait(use_awaitable) ||
        conn.async_execute(stmt.bind(company_id), result, diag, tuple_awaitable)
        );
    check_error(op_result, diag);

    // Print all the obtained rows
    for (mysql::row_view employee : result.rows())
    {
        print_employee(employee);
    }

    // Notify the MySQL server we want to quit, then close the underlying connection.
    op_result = co_await(timer.async_wait(use_awaitable) || conn.async_close(diag, tuple_awaitable));
    check_error(op_result, diag);
}

void main_impl2(int argc, char** argv)
{
    if (argc != 4 && argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " <username> <password> <server-hostname> [company-id]\n";
        exit(1);
    }

    const char* hostname = argv[3];

    // The company_id whose employees we will be listing. This
    // is user-supplied input, and should be treated as untrusted.
    const char* company_id = argc == 5 ? argv[4] : "HGS";

    // I/O context and connection. We use SSL because MySQL 8+ default settings require it.
    asio::io_context ctx;
    asio::ssl::context ssl_ctx(asio::ssl::context::tls_client);
    mysql::tcp_ssl_connection conn(ctx, ssl_ctx);
    asio::steady_timer timer(ctx.get_executor());

    // Connection parameters
    mysql::handshake_params params(
        argv[1],                // username
        argv[2],                // password
        "boost_mysql_examples"  // database to use; leave empty or omit for no database
    );

    // Resolver for hostname resolution
    asio::ip::tcp::resolver resolver(ctx.get_executor());

    // The entry point. We pass in a function returning a asio::awaitable<void>, as required.
    asio::co_spawn(
        ctx.get_executor(),
        [&conn, &resolver, &timer, params, hostname, company_id] {
            return coro_main2(conn, resolver, timer, params, hostname, company_id);
        },
        // If any exception is thrown in the coroutine body, rethrow it.
        [](std::exception_ptr ptr) {
            if (ptr)
            {
                std::rethrow_exception(ptr);
            }
        }
    );

    // Calling run will actually start the requested operations.
    ctx.run();
}

#else

void main_impl2(int, char**)
{
    std::cout << "Sorry, your compiler does not support C++20 coroutines" << std::endl;
}

#endif

#ifdef ASIO_HAS_CO_AWAIT

// Using this completion token instead of plain use_awaitable prevents
// co_await from throwing exceptions. Instead, co_await will return a std::tuple<error_code>
// with a non-zero code on error. We will then use mysql::throw_on_error
// to throw exceptions with embedded diagnostics, if available. If you
// employ plain use_awaitable, you will get boost::system::system_error exceptions
// instead of mysql::error_with_diagnostics exceptions. This is a limitation of use_awaitable.

/**
 * Our coroutine. It must have a return type of asio::awaitable<T>.
 * Our coroutine does not communicate any result back, so T=void.
 * Remember that you do not have to explicitly create any awaitable<void> in
 * your function. Instead, the return type is fed to std::coroutine_traits
 * to determine the semantics of the coroutine, like the promise type.
 * Asio already takes care of all this for us.
 *
 * The coroutine will suspend every time we call one of the asynchronous functions, saving
 * all information it needs for resuming. When the asynchronous operation completes,
 * the coroutine will resume in the point it was left.
 *
 * The return type of an asynchronous operation that uses use_awaitable
 * as completion token is a asio::awaitable<T>, where T
 * is the second argument to the handler signature for the asynchronous operation.
 * If any of the asynchronous operations fail, an exception will be raised
 * within the coroutine.
 */
asio::awaitable<void> coro_main(
    mysql::tcp_ssl_connection& conn,
    asio::ip::tcp::resolver& resolver,
    const mysql::handshake_params& params,
    const char* hostname,
    const char* company_id
)
{
    error_code ec;
    mysql::diagnostics diag;

    // Resolve hostname. We may use use_awaitable here, as hostname resolution
    // never produces any diagnostics.
    auto endpoints = co_await resolver.async_resolve(
        hostname,
        mysql::default_port_string,
        asio::use_awaitable
    );

    // Connect to server
    std::tie(ec) = co_await conn.async_connect(*endpoints.begin(), params, diag, tuple_awaitable);
    mysql::throw_on_error(ec, diag);

    // We will be using company_id, which is untrusted user input, so we will use a prepared
    // statement.
    mysql::statement stmt;
    std::tie(ec, stmt) = co_await conn.async_prepare_statement(
        "SELECT first_name, last_name, salary FROM employee WHERE company_id = ?",
        diag,
        tuple_awaitable
    );
    mysql::throw_on_error(ec, diag);

    // Execute the statement
    mysql::results result;
    std::tie(ec) = co_await conn.async_execute(stmt.bind(company_id), result, diag, tuple_awaitable);
    mysql::throw_on_error(ec, diag);

    // Print all employees
    for (mysql::row_view employee : result.rows())
    {
        print_employee(employee);
    }

    // Notify the MySQL server we want to quit, then close the underlying connection.
    std::tie(ec) = co_await conn.async_close(diag, tuple_awaitable);
    mysql::throw_on_error(ec, diag);
}

void main_impl(int argc, char** argv)
{
    if (argc != 4 && argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " <username> <password> <server-hostname> [company-id]\n";
        exit(1);
    }

    const char* hostname = argv[3];

    // The company_id whose employees we will be listing. This
    // is user-supplied input, and should be treated as untrusted.
    const char* company_id = argc == 5 ? argv[4] : "HGS";

    // I/O context and connection. We use SSL because MySQL 8+ default settings require it.
    asio::io_context ctx;
    asio::ssl::context ssl_ctx(asio::ssl::context::tls_client);
    mysql::tcp_ssl_connection conn(ctx, ssl_ctx);

    // Connection parameters
    mysql::handshake_params params(
        argv[1],                // username
        argv[2],                // password
        "boost_mysql_examples"  // database to use; leave empty or omit the parameter for no
                                // database
    );

    // Resolver for hostname resolution
    asio::ip::tcp::resolver resolver(ctx.get_executor());

    // The entry point. We pass in a function returning
    // asio::awaitable<void>, as required.
    asio::co_spawn(
        ctx.get_executor(),
        [&conn, &resolver, params, hostname, company_id] {
            return coro_main(conn, resolver, params, hostname, company_id);
        },
        // If any exception is thrown in the coroutine body, rethrow it.
        [](std::exception_ptr ptr) {
            if (ptr)
            {
                std::rethrow_exception(ptr);
            }
        }
    );

    // Calling run will execute the requested operations.
    ctx.run();
}

#else

void main_impl(int, char**)
{
    std::cout << "Sorry, your compiler does not support C++20 coroutines" << std::endl;
}

#endif

class application
{
    asio::ip::tcp::resolver::results_type eps;  // Physical endpoint(s) to connect to
    mysql::handshake_params conn_params;        // MySQL credentials and other connection config
    asio::io_context ctx;                       // asio context
    asio::ip::tcp::resolver resolver;           // To perform hostname resolution
    asio::ssl::context ssl_ctx;                 // MySQL 8+ default settings require SSL
    mysql::tcp_ssl_connection conn;             // Represents the connection to the MySQL server
    mysql::statement stmt;                      // A prepared statement
    mysql::results result;                      // A result from a query
    mysql::diagnostics diag;                    // Will be populated with info about server errors
    const char* company_id;  // The ID of the company whose employees we want to list. Untrusted.
public:
    application(const char* username, const char* password, const char* company_id)
        : conn_params(username, password, "boost_mysql_examples"),
          resolver(ctx.get_executor()),
          ssl_ctx(asio::ssl::context::tls_client),
          conn(ctx, ssl_ctx),
          company_id(company_id)
    {
    }

    void start(const char* hostname) { resolve_hostname(hostname); }

    void resolve_hostname(const char* hostname)
    {
        resolver.async_resolve(
            hostname,
            mysql::default_port_string,
            [this](error_code err, asio::ip::tcp::resolver::results_type results) {
                mysql::throw_on_error(err);
                eps = std::move(results);
                connect();
            }
        );
    }

    void connect()
    {
        conn.async_connect(*eps.begin(), conn_params, diag, [this](error_code err) {
            mysql::throw_on_error(err, diag);
            prepare_statement();
        });
    }

    void prepare_statement()
    {
        // We will be using company_id, which is untrusted user input, so we will use a prepared
        // statement.
        conn.async_prepare_statement(
            "SELECT first_name, last_name, salary FROM employee WHERE company_id = ?",
            diag,
            [this](error_code err, mysql::statement temp_stmt) {
                mysql::throw_on_error(err, diag);
                stmt = temp_stmt;
                query_employees();
            }
        );
    }

    void query_employees()
    {
        conn.async_execute(stmt.bind(company_id), result, diag, [this](error_code err) {
            mysql::throw_on_error(err, diag);
            for (mysql::row_view employee : result.rows())
            {
                print_employee(employee);
            }
            close();
        });
    }

    void close()
    {
        // Notify the MySQL server we want to quit and then close the socket
        conn.async_close(diag, [this](error_code err) { mysql::throw_on_error(err, diag); });
    }

    void run() { ctx.run(); }
};

void mysql_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

    static bool flag = false;

    // just test compile

    if (flag)
    {
        application app("root", "123456", "HGS");
        app.start("127.0.0.1");  // starts the async chain
        app.run();           // run the asio::io_context until the async chain finishes
    }

	ASIO2_TEST_END_LOOP;
}

ASIO2_TEST_SUITE
(
	"mysql",
	ASIO2_TEST_CASE(mysql_test)
)
