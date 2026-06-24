#include "server/Server.hpp"
#include "utils/colors.h"
#include <exception>
#include <iostream>

int	main(int ac, char* av[])
{
	if (ac != 3)
	{
		std::cerr << RED << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	Server server(av[2]);
	try {
		server.start(av[1]);
	} catch (std::exception &e) {
		std::cerr << RED << e.what() << std::endl;
	}
}
