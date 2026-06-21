#include "Server.hpp"
#include "Client.hpp"

#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <vector>

#define MAX_EVENTS 16
#define MAX_MSG_SIZE 512

// ---------------------------------------------------------------- CONSTRUCTORS
Server::Server(std::string password) : _fd(-1), _isRunning(false), _password(password){}

Server::~Server() {}

// ------------------------------------------------------------ MEMBER FUNCTIONS
void	Server::init(char* port)
{
	struct addrinfo	hints, *info;
	
	std::memset(&hints, 0, sizeof(hints));		// remove garbage data
	hints.ai_family = AF_UNSPEC;				// Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;			// TCP
	hints.ai_flags = AI_PASSIVE;				// Auto set host IP

	if (getaddrinfo(NULL, port, &hints, &info) != 0)
		throw std::runtime_error("Network resolution failed");

	bool	success = false;
	for(struct addrinfo *it = info;it != NULL; it = it->ai_next)
	{
		_fd = socket(it->ai_family, it->ai_socktype, 0);
		if (_fd < 0) continue;

		int yes = 1;
		if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == 0 && bind(_fd, it->ai_addr, it->ai_addrlen) == 0)
		{
			success = true;
			break;
		}
		close(_fd);
	}
	freeaddrinfo(info);
	if (!success)
		throw std::runtime_error("Unable to set up socket");

	if (listen(_fd, 5) < 0)
	{
		close(_fd);
		throw std::runtime_error("Unable to listen");
	}
	_isRunning = true;
	_startListening();
}

void	Server::_startListening()
{
	int fdepoll = epoll_create1(0);
	if (fdepoll < 0)
		throw std::runtime_error("Error creating epoll");
	struct epoll_event ev[MAX_EVENTS];
	ev[0].events = EPOLLIN;
	ev[0].data.fd = _fd;
	epoll_ctl(fdepoll, EPOLL_CTL_ADD, _fd, ev);
	while (_isRunning)
	{
		int n = epoll_wait(fdepoll, ev, MAX_EVENTS, -1);

		for (int i = 0; i < n; i++)
		{
			int fd = ev[i].data.fd;
			if (fd == _fd)
			{
				struct sockaddr_storage addr;
				socklen_t addrlen = sizeof(addr);
				int client_fd = accept(_fd, (struct sockaddr *)&addr, &addrlen);
				struct epoll_event client_ev;
				client_ev.events = EPOLLIN;
				client_ev.data.fd = client_fd;
				epoll_ctl(fdepoll, EPOLL_CTL_ADD, client_fd, &client_ev);
				_addClient(client_fd);
			}
			else
			{
				char	message[MAX_MSG_SIZE + 1];
				int		data = recv(fd, message, MAX_MSG_SIZE, 0);
				if (data <= 0)
				{
					_disconnectClient(fd); //or fail if < 0
					continue;
				}
				message[data] = '\0';
				std::cout << "bytes: " << data << " msg: ";
				std::cout.write(message, data);
				std::cout << std::endl;
			}
		}
	}
}

void	Server::_addClient(int fd)
{
	Client		newClient(fd);

	std::cout << "Connected client with fd: " <<  fd << std::endl;
	_clients.push_back(newClient);
}

void	Server::_disconnectClient(const int fd)
{
	close(fd);
}

void	Server::stop()
{
	_isRunning = false;
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
		_disconnectClient(it->getfd());
	close(_fd);
	std::cout << "------------ THISCORD SERVER CLOSED! ------------" << std::endl;
}
