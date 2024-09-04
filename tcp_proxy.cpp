#include "tcp_proxy.h"

#include <fstream>
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>

void tcp_proxy::start(const std::string& server_ip, unsigned short server_port)
{
	auto self(shared_from_this());
	server_socket_.async_connect(
		ip::tcp::endpoint(
			boost::asio::ip::address::from_string(server_ip),
			server_port),
        [self](const boost::system::error_code& error) {
            if (!error) {
                self->read_from_client(error);
                self->read_from_server(error);
            } 
            else {
                std::cerr << "Failed to connect to server: " << error.message() << std::endl;
                self->close();
            }
    });
}

void tcp_proxy::send_to_client(const boost::system::error_code& error,
    const size_t& bytes_transferred)
{
    if (!error) {
        async_write(client_socket_,
            boost::asio::buffer(server_data_, bytes_transferred),
            boost::bind(&tcp_proxy::read_from_server,
                shared_from_this(),
                boost::asio::placeholders::error));
    }
    else {
        std::cerr << "Error sending data to client: " << error.message() << std::endl;
        close();
    }
}

void tcp_proxy::read_from_server(const boost::system::error_code& error)
{
    if (!error)
    {
        server_socket_.async_read_some(
            boost::asio::buffer(server_data_, max_data_length),
            boost::bind(&tcp_proxy::send_to_client,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else {
        std::cerr << "Error reading from server: " << error.message() << std::endl;
        close();
    }
}

void tcp_proxy::send_to_server(const boost::system::error_code& error,
    const size_t& bytes_transferred)
{
    if (!error)
    {
        async_write(server_socket_,
            boost::asio::buffer(client_data_, bytes_transferred),
            boost::bind(&tcp_proxy::read_from_client,
                shared_from_this(),
                boost::asio::placeholders::error));
    }
    else {
        std::cerr << "Error sending data to server: " << error.message() << std::endl;
        close();
    }
}

void tcp_proxy::read_from_client(const boost::system::error_code& error)
{
    if (!error)
    {
		auto self(shared_from_this());
		client_socket_.async_read_some(
			boost::asio::buffer(client_data_, max_data_length),
			[self](boost::system::error_code ec, std::size_t length) {
				if (!ec) {
					self->write_log(self->client_data_.data(), length);
					self->send_to_server(
						ec,
						length
					);
				}
			});
	}
    else {
        std::cerr << "Error reading from client: " << error.message() << std::endl;
        close();
    }
}

void tcp_proxy::close()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (client_socket_.is_open())
    {
		client_socket_.close();
    }

    if (server_socket_.is_open())
    {
		server_socket_.close();
    }
}

void tcp_proxy::write_log(const char data[], std::size_t length) {

    std::lock_guard<std::mutex> lock(mutex_);
	char payload_length_c[3] = {data[0], data[1], data[2]};
	char sequence_id = data[3];
	char command = data[4];
	if (command != 0x03 && command != 0x16) { return; }

	int payload_length = int(uint32_t(payload_length_c[0]) | 
                             uint32_t(payload_length_c[1]) << 8 | 
                             uint32_t(payload_length_c[2]) << 16) - 1;

	std::time_t t = std::time(nullptr);
	std::tm now;
    localtime_s(&now, &t);
	char buffer[128];
	strftime(buffer, sizeof(buffer), "%m-%d-%Y %X", &now);

	std::ofstream log_file("sql_log.txt", std::ios_base::app);
	log_file.write(buffer, strlen(buffer));
	log_file.write(" / - / ", 7);
	log_file.write(data + 5, payload_length);
	log_file << "\n";
	log_file.close();
}

bool tcp_proxy::acceptor::accept_connections() {
   try
   {
	    session_ = std::shared_ptr<tcp_proxy>(new tcp_proxy(io_service_));
		    acceptor_.async_accept(session_->client_socket(),
		    boost::bind(&acceptor::handle_accept,
			   this,
			   boost::asio::placeholders::error));
    }
    catch (std::exception& e)
    {
	    std::cerr << "Acceptor exception: " << e.what() << std::endl;
	    return false;
	}
	return true;
}

void tcp_proxy::acceptor::handle_accept(const boost::system::error_code& error) {
    if (!error)
    {
 	    session_->start(server_host_, server_port_);
 	    if (!accept_connections())
	    {
	 	    std::cerr << "Failure during call to accept." << std::endl;
	    }
    }
    else
    {
	    std::cerr << "Error during accept: " << error.message() << std::endl;
    }
}