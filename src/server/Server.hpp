#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"

class Server
{
	private:
		int 	_fd;
		Client*	_clients;
		
	public:
		Server();
		~Server();
		void	init(char* port);
};

#endif
