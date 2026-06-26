#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include <string>
#include <sys/epoll.h>
#include <map>

class Server
{
	private:
		int								_fd;
		int								_epoll;
		bool							_isRunning;
		std::map<int, Client>			_clients;
		std::map<std::string, Channel>	_channels;
		const std::string				_password;

		void		_setup(char* port);
		void		_listenLoop();

		void		_addClient(const int fd);
		void		_handleMessage(const int fd);
		void		_disconnectClient(Client& client);
		
	public:
		Server(std::string password);
		~Server();
		void		start(char* port);
		void		stop();
};

epoll_event	newEvent(int fd, int flags);

#endif
