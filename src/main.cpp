#include "Server.hpp"
#include "colors.h"
#include "utils/colors.h"
#include <exception>
#include <iostream>

int	main(int ac, char* av[])
{
	if (ac != 3)
	{
		std::cerr << RED << "Usage: ./ircserv <port> <password>" << RESET << std::endl;
		return 1;
	}

	Server server("Thiscord", av[2]);
	try {
		server.start(av[1]);
	} catch (std::exception& e) {
		if (!e.what()) std::cerr << RED << e.what() << RESET << std::endl;
		server.stop();
	}

	return 0;
}

/*  TODO list
Commands
Make fds non-blocking
Insert communication with clients into epoll (send())
Handle signals


Check memory leaks (final)
Check normi (haha no)
 */