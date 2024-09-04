#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "tcp_proxy.h"

int main(int argc, const char** argv) {

	if (argc != 5)
	{
		std::cerr << "usage: tcp_proxy_sql <local host ip> <local port> <server host ip> <server port>" << std::endl;
		return 1;
	}

	const unsigned short local_port = static_cast<unsigned short>(::atoi(argv[2]));
	const unsigned short server_port = static_cast<unsigned short>(::atoi(argv[4]));
	const std::string local_host = argv[1];
	const std::string server_host = argv[3];
	
	boost::asio::io_context io_contex;

	try
	{
		tcp_proxy::acceptor acceptor(io_contex,
			local_host, local_port,
			server_host, server_port);

		acceptor.accept_connections();

		io_contex.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;

}