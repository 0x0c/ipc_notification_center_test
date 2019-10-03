#include <array>
#include <boost/asio.hpp>
#include <cstdio>
#include <iostream>
#include <memory>
#include <msgpack.hpp>
#include <functional>

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

using boost::asio::local::stream_protocol;

class session : public std::enable_shared_from_this<session>
{
public:
    std::function<void(msgpack::object, std::shared_ptr<session> ses)> handler_;
	session(stream_protocol::socket sock)
	    : socket_(std::move(sock))
	{
	}
    
    ~session()
    {
        std::cout << "deinit" << std::endl;
    }
    
	void start(std::function<void(msgpack::object, std::shared_ptr<session> ses)> handler) {
        handler_ = std::move(handler);
        wait();
    }
    

    void send(msgpack::object data) {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, data);
        boost::system::error_code ec;
        auto self(shared_from_this());
        boost::asio::write(socket_, boost::asio::buffer(buffer.data(), buffer.size()), ec);
        if (!ec) {
            std::cout << "send success" << std::endl;
        }
        else {
            std::cout << "send error: " << ec << std::endl;
        }
    }

private:
	std::size_t const window_size = 1024;
	msgpack::unpacker unp;

	void wait()
	{
		// clientからデータが送られてくるのを待つ
        unp.reserve_buffer(window_size);
        
		auto self(shared_from_this());
        boost::system::error_code ec;
        size_t length = socket_.read_some(boost::asio::buffer(unp.buffer(), window_size), ec);
        if (!ec) {
            unp.buffer_consumed(length);
            msgpack::object_handle oh;
            std::cout << "wait success" << std::endl;
            while (unp.next(oh)) {
                handler_(oh.get(), self);
            }
        }
        else {
            std::cout << "wait error: " << ec << std::endl;
        }
	}

	void reply(std::size_t length)
	{
		// 受信したデータをそのまま返信する
		auto self(shared_from_this());
        boost::system::error_code ec;
        socket_.write_some(boost::asio::buffer(unp.buffer(), length), ec);
        if (!ec) {
//            wait();
            std::cout << "reply success" << std::endl;
        }
        else {
            std::cout << "reply error: " << ec << std::endl;
        }
	}

	// The socket used to communicate with the client.
	stream_protocol::socket socket_;
};

class server
{
public:
	server(boost::asio::io_context &io_context, const std::string &file)
	    : acceptor_(io_context, stream_protocol::endpoint(file))
	{
		accept();
	}

private:
	void accept()
	{
        std::cout << "accept" << std::endl;
		acceptor_.async_accept(
		    [this](boost::system::error_code ec, stream_protocol::socket socket) {
			    if (!ec) {
				    std::cout << "start session: " << std::endl;
				    auto s = std::make_shared<session>(std::move(socket));
                    sessions_.push_back(s);
                    s->start([this] (msgpack::object obj, std::shared_ptr<session> ses) {
                        std::cout << obj << std::endl;
                        msgpack::object_map obj_map = obj.via.map;
                        
                        if (obj_map.size < 2) {
                            std::cout << "invalid message" << std::endl;
                            return;
                        }
                        
                        std::string action;
                        (obj_map.ptr[0].val).convert(action);
                        
                        std::string topic;
                        (obj_map.ptr[1].val).convert(topic);
                        
                        if (action == "subscribe") {
                            if (this->subscribers_.find(topic) != this->subscribers_.end()) {
                                this->subscribers_.at(topic).push_back(ses);
                            }
                            else {
                                std::vector<std::shared_ptr<session>> subscribers;
                                subscribers.push_back(ses);
                                this->subscribers_.insert(std::make_pair(topic, std::move(subscribers)));
                            }
                            std::cout << "subscribe" << std::endl;
                        }
                        else if (action == "broadcast") {
                            std::cout << "broadcast: " << topic << std::endl;
                            auto data = obj_map.ptr[2].val;
                            if (this->subscribers_.find(topic) != this->subscribers_.end()) {
                                std::cout << "broadcast found" << std::endl;
                                for (auto ses : this->subscribers_.at(topic)) {
                                    ses->send(data);
                                }
                            }
                        }
                    });
			    }

			    accept();
		    });
	}

	stream_protocol::acceptor acceptor_;
    std::vector<std::shared_ptr<session>> sessions_;
    std::map<std::string, std::vector<std::shared_ptr<session>>> subscribers_;
};

int main(int argc, char *argv[])
{
    std::cout << "start server" << std::endl;
	try {
		if (argc != 2) {
			std::cerr << "Usage: stream_server <file>\n";
			std::cerr << "*** WARNING: existing file is removed ***\n";
			return 1;
		}

		boost::asio::io_context io_context;

		std::remove(argv[1]);
		server s(io_context, argv[1]);

		io_context.run();
	} catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

#else // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#error Local sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
