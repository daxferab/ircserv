#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include <string>
#include <vector>

class Server
{
	private:
		int						_fd;
		bool					_isRunning;
		std::vector<Client>		_clients;
		std::vector<Channel>	_channels;
		std::string				_password;

		void	_startListening();
		void	_addClient(int fd);
		void	_disconnectClient(int fd);
		
	public:
		Server(std::string password);
		~Server();
		void	init(char* port);
		void	stop();
};

#endif
