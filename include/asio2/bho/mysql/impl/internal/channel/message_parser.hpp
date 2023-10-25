//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_CHANNEL_MESSAGE_PARSER_HPP
#define BHO_MYSQL_IMPL_INTERNAL_CHANNEL_MESSAGE_PARSER_HPP

#include <asio2/bho/mysql/detail/config.hpp>

#include <asio2/bho/mysql/impl/internal/channel/read_buffer.hpp>
#include <asio2/bho/mysql/impl/internal/protocol/constants.hpp>

#include <cstddef>
#include <cstdint>

namespace bho {
namespace mysql {
namespace detail {

class message_parser
{
    struct state_t
    {
        bool is_first_frame{true};
        std::uint8_t seqnum_first{};
        std::uint8_t seqnum_last{};
        bool reading_header{true};
        std::size_t remaining_bytes{0};
        bool more_frames_follow{false};
        bool has_seqnum_mismatch{false};
    };

    std::size_t max_frame_size_;
    state_t state_;

public:
    struct result
    {
        // whether it has a message or not
        bool has_message{false};

        // if !has_message, number of bytes required to parse the current message
        std::size_t required_size{0};

        // if has_message, the actual parsed message
        struct message_t
        {
            std::uint8_t seqnum_first;
            std::uint8_t seqnum_last;
            std::size_t size;
            bool has_seqnum_mismatch;  // for multi-frame messages, set to true if an error mismatch
                                       // happened
        } message{};

        void set_required_size(std::size_t size) noexcept
        {
            has_message = false;
            required_size = size;
        }

        void set_message(const message_t& msg) noexcept
        {
            has_message = true;
            message = msg;
        }
    };

    // max_frame_size is configurable so tests run faster
    message_parser(std::size_t max_frame_size = MAX_PACKET_SIZE) noexcept : max_frame_size_(max_frame_size){};

    // Attempts to process a message from buff and puts it into msg.
    // If a message is read, res.has_message == true, and res.message will be populated.
    // Otherwise, res.required_size will contain
    // the number of bytes needed to complete the message part we're parsing.
    // Doesn't cause buffer reallocations, and doesn't change the contents
    // of buff's reserved area.
    BHO_MYSQL_DECL
    void parse_message(read_buffer& buff, result& res) noexcept;
};

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#ifdef BHO_MYSQL_HEADER_ONLY
#include <asio2/bho/mysql/impl/internal/channel/message_parser.ipp>
#endif

#endif
