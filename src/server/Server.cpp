#include "Server.hpp"
#include "Client.hpp"
#include "../utils/colors.h"
#include "CommandHandler.hpp"

#include <cstdio>
#include <exception>
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
#include <utility>

#define MAX_EVENTS 16
#define MAX_MSG_SIZE 512
#define LISTENING_QUEUE 5

// ---------------------------------------------------------------- CONSTRUCTORS
Server::Server(std::string password) : _fd(-1), _isRunning(false), _password(password){}

Server::~Server() {}

// ------------------------------------------------------------ MEMBER FUNCTIONS
void	Server::start(char* port)
{
	try {
		_setupSocket(port);
		_listenLoop();
	} catch (std::exception& e) {
		throw;
	}
}

void	Server::_setupSocket(char* port)
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

		int val = 1;
		if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == 0 && bind(_fd, it->ai_addr, it->ai_addrlen) == 0)
		{
			success = true;
			break;
		}
		close(_fd);
	}
	freeaddrinfo(info);
	if (!success)
		throw std::runtime_error("Unable to set up socket");

	if (listen(_fd, LISTENING_QUEUE) < 0)
	{
		close(_fd);
		throw std::runtime_error("Unable to listen");
	}
	_isRunning = true;
	std::cout << BLUE << "---------------- THISCORD RUNNING ----------------" << RESET << std::endl;
}

void	Server::_listenLoop()
{
	_epoll = epoll_create1(0);
	if (_epoll < 0)
		throw std::runtime_error("Error creating epoll");
	struct epoll_event	sock_ev = newEvent(_fd, EPOLLIN);
	epoll_ctl(_epoll, EPOLL_CTL_ADD, _fd, &sock_ev);

	while (_isRunning)
	{
		struct epoll_event events[MAX_EVENTS];
		int n = epoll_wait(_epoll, events, MAX_EVENTS, -1);

		for (int i = 0; i < n; i++)
		{
			int fd = events[i].data.fd;
			if (fd == _fd)
			{
				struct sockaddr_storage addr;
				socklen_t addrlen = sizeof(addr);
				int client_fd = accept(_fd, (struct sockaddr *)&addr, &addrlen);
				_addClient(client_fd);
			} else {
				_handleMessage(fd);
			}
		}
	}
}

void	Server::_handleMessage(int fd)
{
	char	message[MAX_MSG_SIZE + 1];
	int		data = recv(fd, message, MAX_MSG_SIZE, 0);
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;
	if (data < 0)
		std::cout << RED << "Error reading message from " << fd << RESET << std::endl;
	else if (data == 0)
		_disconnectClient(it->second);
	else
	{
		Client& client = it->second;
		client.appendBuffer(message, data);
		while (client.hasFullLine())
		{
			std::string	line = client.getLine();
			CommandHandler::execCommand(line, client);
		}
	}
}

void	Server::_addClient(int fd)
{
	struct epoll_event client_ev = newEvent(fd, EPOLLIN);
	epoll_ctl(_epoll, EPOLL_CTL_ADD, fd, &client_ev);

	std::pair<int, Client>	pair(fd, Client(fd));
	_clients.insert(pair);

	std::cout << GREEN << "Client " << fd << " connected" << RESET << std::endl;
}

void	Server::_disconnectClient(Client& client)
{
	int fd = client.getfd();
	std::map<int, Client>::iterator it = _clients.find(fd);

	std::cout << RED << "Client <" << client.getnick() << "> disconnected" << RESET << std::endl;
	epoll_ctl(_epoll, EPOLL_CTL_DEL, fd, NULL);
	if (it != _clients.end())
		_clients.erase(it);
	close(fd);
}

void	Server::stop()
{
	_isRunning = false;
	while (!_clients.empty())
		_disconnectClient(_clients.begin()->second);
	epoll_ctl(_epoll, EPOLL_CTL_DEL, _fd, NULL);
	close(_fd);
	std::cout << RED << "------------ THISCORD SERVER CLOSED! ------------" << RESET << std::endl;
}

// ----------------------------------------------------------------------- UTILS
epoll_event newEvent(int fd, int flags)
{
	epoll_event	ev;
	ev.events = flags;
	ev.data.fd = fd;
	return ev;
}
