#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <array>

namespace ip = boost::asio::ip;

class tcp_proxy : public std::enable_shared_from_this<tcp_proxy>
{
public:
	using socket_type = ip::tcp::socket;
	using ptr_type = std::shared_ptr<tcp_proxy>;

	tcp_proxy(boost::asio::io_service& ios)
		: client_socket_(ios),
		server_socket_(ios)
	{}

	// Запрещаем копирование
	tcp_proxy(const tcp_proxy&) = delete;
	tcp_proxy& operator=(const tcp_proxy&) = delete;

	// Разрешаем перемещение
	tcp_proxy(tcp_proxy&&) = default;
	tcp_proxy& operator=(tcp_proxy&&) = default;

	inline socket_type& client_socket() { return client_socket_; }

	inline socket_type& server_socket() { return server_socket_; }

	void start(const std::string& server_ip, unsigned short server_port);

private:

    void send_to_client(const boost::system::error_code& error, const size_t& bytes_transferred);

    void read_from_server(const boost::system::error_code& error);

    void send_to_server(const boost::system::error_code& error, const size_t& bytes_transferred);

    void read_from_client(const boost::system::error_code& error);

    void close();
    
	void write_log(const char data[], std::size_t length);
	
	socket_type client_socket_;
	socket_type server_socket_;

	enum { max_data_length = 8192 }; //8KB
    std::array<char, max_data_length> client_data_;
    std::array<char, max_data_length> server_data_;

	std::mutex mutex_;

public:

    class acceptor
    {
    public:
	    acceptor(boost::asio::io_service& io_service,
		    const std::string& local_host, unsigned short local_port,
		    const std::string& server_host, unsigned short server_port)
			    : io_service_(io_service),
			    localhost_address(boost::asio::ip::address_v4::from_string(local_host)),
			    acceptor_(io_service_, ip::tcp::endpoint(localhost_address, local_port)),
			    server_port_(server_port),
			    server_host_(server_host)
		    {}

		bool accept_connections();

    private:

	    void handle_accept(const boost::system::error_code& error);

	    boost::asio::io_service& io_service_;
	    ip::address_v4 localhost_address;
	    ip::tcp::acceptor acceptor_;
	    ptr_type session_;
	    unsigned short server_port_;
	    std::string server_host_;
    };

};
