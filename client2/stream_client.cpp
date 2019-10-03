//
// stream_client.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <msgpack.hpp>

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

using boost::asio::local::stream_protocol;

constexpr std::size_t max_length = 1024;

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2) {
            std::cerr << "Usage: stream_client <file>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        // 接続しに行く
        stream_protocol::socket s(io_context);
        s.connect(stream_protocol::endpoint(argv[1]));

//    std::cout << "Enter message: ";
//    char request[max_length];
//    std::cin.getline(request, max_length);
//    size_t request_length = std::strlen(request);
//    msgpack::type::tuple<std::string, bool, int> src(std::string(request, request_length), false, 100);
        
        msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> pk2(&buffer);
        pk2.pack_map(3);
        pk2.pack(std::string("action"));
        pk2.pack(std::string("broadcast"));
        pk2.pack(std::string("topic"));
        pk2.pack(std::string("mute"));
        pk2.pack(std::string("data"));
        pk2.pack(std::string("unmuted"));

        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(buffer.data(), buffer.size()), ec);
        if (!ec) {
            std::cout << "sent" << std::endl;
        }
        else {
            std::cout << "error: " << ec << std::endl;
        }

//        msgpack::unpacker unp;
//        size_t reply_length = boost::asio::read(s, boost::asio::buffer(unp.buffer(), buffer.size()));
//        unp.buffer_consumed(reply_length);
//        std::cout << "Reply is: ";
//        msgpack::object_handle oh;
//        while (unp.next(oh)) {
//            std::cout << oh.get();
//        }
        std::cout << "\n";
        std::cout << "done" << std::endl;
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

#else // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#error Local sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
