#include "Server.hpp"
#include "Client.hpp"

#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <vector>

// ---------------------------------------------------------------- CONSTRUCTORS
Server::Server() : _fd(-1), _isRunning(false){}

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
	startListening();
}

void	Server::startListening()
{
	struct sockaddr_storage newConnection;									// Use sockaddr_storage so it works for both IPv4 and IPv6
	socklen_t				newLength = sizeof(struct sockaddr_storage);	// Set to the max storage "newConnection" can have (will be modified if accept puts less)
	int						clientfd;
	
	while (_isRunning)
	{
		clientfd = accept(_fd, reinterpret_cast<struct sockaddr *>(&newConnection), &newLength);
		if (clientfd < 0)
			throw std::runtime_error("Unable to accept");
		newClient(clientfd);
	}
}

void	Server::newClient(int fd)
{
	Client newClient(fd);
	std::cout << "Connected client with fd: " <<  fd << std::endl;
	_clients.push_back(newClient);
}

void	Server::disconnectClient(const int fd)
{
	close(fd);
}

void	Server::stop()
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
		disconnectClient(it->getfd());
	close(_fd);
	std::cout << "------------ THISCORD SERVER CLOSED! ------------" << std::endl;
}
