#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include <netdb.h>
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
		bool		_createSocket(struct addrinfo *info);
		void		_initEpoll();
		void		_eventLoop();

		void		_readFd(const int fd);
		void		_handleLine(Client& client, char* line, int data);
		void		_addClient(const int fd);
		void		_disconnectClient(Client& client);
		
	public:
		Server(std::string password);
		~Server();
		void		start(char* port);
		void		stop();

		bool		authClient(Client& client, const std::string pass) const;
};

epoll_event	newEvent(int fd, int flags);

#endif
