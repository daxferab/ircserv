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
		server.stop();
		std::cerr << RED << e.what() << std::endl;
	}
}

/*  TODO list
Commands
Make fds non-blocking
Insert communication with clients into epoll (send())
Handle signals


Check memory leaks (final)
Check normi (haha no)
 */