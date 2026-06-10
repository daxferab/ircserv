#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <vector>

class Server
{
	private:
		int				_fd;
		bool				_isRunning;
		std::vector<Client>	_clients;

		void	startListening();
		void	newClient(int fd);
		void	disconnectClient(int fd);
		
	public:
		Server();
		~Server();
		void	init(char* port);
		void	stop();
};

#endif
