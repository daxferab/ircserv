#include "Server.hpp"
#include "AReply.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "../utils/colors.h"
#include "CommandHandler.hpp"

#include <cstdio>
#include <exception>
#include <map>
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
#define BUFFERSIZE 512
#define LISTENING_QUEUE 5

// ---------------------------------------------------------------- CONSTRUCTORS
Server::Server(std::string name, std::string password) : _fd(-1), _name(name), _isRunning(false), _password(password){}

Server::~Server() {}

// ------------------------------------------------------------ MEMBER FUNCTIONS
// --------------------------- PRIVATE FUNCTIONS
void	Server::start(char* port)
{
	try {
		_setup(port);
		_initEpoll();
		_eventLoop();
	} catch (std::exception& e) {
		throw;
	}
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

std::string	Server::getName() const { return _name; }

void	Server::authClient(Client& client, const std::string& pass) const
{
	if (client.isAuthenticated())
		_reply(client.getFd(), AReply::getReply(ERR, 462, client.getNick(), getName()));
	else if (pass.empty())
		_reply(client.getFd(), AReply::getReply(ERR, 461, client.getNick(), getName()));
	else if (pass != _password)
		_reply(client.getFd(), AReply::getReply(ERR, 464, client.getNick(), getName()));
	else
	{
		client.setAuthenticated(true);
		std::cout << "Client " << client << " gets authenticated with password " << pass << std::endl;
	}
}

void	Server::setClientNick(Client& client, const std::string& nick) const
{
	if (nick.empty())
		return ; // TODO ERR_NONICKNAMEGIVEN (431)
	if (isReservedChar(nick[0]))
		return ; // TODO ERR_ERRONEUSNICKNAME (432)
	if (_nickInUse(nick))
		return ; // TODO ERR_NICKNAMEINUSE (433)
	client.setNick(nick);
	_reply(client.getFd(), AReply::getReply(RPL, 001, client.getNick(), getName()));	
}

bool	Server::setClientUser(Client& client, const std::string& user) const
{
	if (client.isRegistered())
		{} //TODO: ERR_ALREADYREGISTERED (462)
	else if (user.empty())
		{} //TODO: ERR_NEEDMORPARAMS(461)
	else
	{
		client.setUser(user); //TODO: RPL_WELCOME (001)		return true;
	}
	return false;
}

void	Server::setClientName(Client& client, const std::string& name) const
{
	client.setName(name);
}

// --------------------------- PUBLIC EFUNCTIONS

void	Server::_setup(char* port)
{
	struct addrinfo	hints, *info; //NOTE: dont know if we need to free hints
	
	std::memset(&hints, 0, sizeof(hints));		// remove garbage data
	hints.ai_family = AF_UNSPEC;				// Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;			// TCP
	hints.ai_flags = AI_PASSIVE;				// Auto set host IP

	if (getaddrinfo(NULL, port, &hints, &info) != 0)
		throw std::runtime_error("Network resolution failed");
	if (!_createSocket(info))
		throw std::runtime_error("Unable to set up socket");
	freeaddrinfo(info);

	if (listen(_fd, LISTENING_QUEUE) < 0)
	{
		close(_fd);
		throw std::runtime_error("Unable to listen");
	}
	_isRunning = true;
	std::cout << BLUE << "---------------- THISCORD RUNNING ----------------" << RESET << std::endl;
}

bool	Server::_createSocket(struct addrinfo *info)
{
	for(struct addrinfo *it = info;it != NULL; it = it->ai_next)
	{
		_fd = socket(it->ai_family, it->ai_socktype, 0);
		if (_fd < 0) continue;

		int val = 1;
		if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == 0 && bind(_fd, it->ai_addr, it->ai_addrlen) == 0)
			return true;
		close(_fd);
	}
	return false;
}

void	Server::_initEpoll()
{
	_epoll = epoll_create1(0);
	if (_epoll < 0)
		throw std::runtime_error("Error creating epoll");
	struct epoll_event	sock_ev = newEvent(_fd, EPOLLIN);
	epoll_ctl(_epoll, EPOLL_CTL_ADD, _fd, &sock_ev);
}

void	Server::_eventLoop()
{
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
				_readFd(fd);
			}
		}
	}
}

void	Server::_readFd(const int fd)
{	
	char	line[BUFFERSIZE + 1];
	int		data = recv(fd, line, BUFFERSIZE, 0);
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;		
	if (data < 0)
		std::cout << RED << "Error reading message from " << fd << RESET << std::endl;
	else if (data == 0)
		_disconnectClient(it->second);
	else
		_handleLine(it->second, line, data);
}

void	Server::_handleLine(Client& client, char* line, int data)
{	
	client.appendBuffer(line, data);
	while (client.hasFullLine())
	{		
		Message	message(client.getLine());
		if (message.isValid())
			CommandHandler::execCommand(message, client, *this);
	}
}

void	Server::_reply(const int clientfd, const std::string& message) const
{
	std::cout << "msg: " << message << std::endl;
	send(clientfd, message.c_str(), message.size(), 0);
}

void	Server::_addClient(const int fd)
{
	struct epoll_event client_ev = newEvent(fd, EPOLLIN);
	epoll_ctl(_epoll, EPOLL_CTL_ADD, fd, &client_ev);

	std::pair<int, Client>	pair(fd, Client(fd));
	_clients.insert(pair);
	std::cout << GREEN << "Client " << fd << " connected" << RESET << std::endl;
}

void	Server::_disconnectClient(Client& client)
{
	int fd = client.getFd();
	std::map<int, Client>::iterator it = _clients.find(fd);

	std::cout << RED << "Client <" << client << "> disconnected" << RESET << std::endl;
	epoll_ctl(_epoll, EPOLL_CTL_DEL, fd, NULL);
	if (it != _clients.end())
		_clients.erase(it);
	close(fd);
}

bool	Server::_nickInUse(const std::string nick) const
{
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end();it++)
		if (it->second.getNick().compare(nick) == 0) return true;
	return false;
}

// ----------------------------------------------------------------------- UTILS
epoll_event newEvent(int fd, int flags)
{
	epoll_event	ev;
	ev.events = flags;
	ev.data.fd = fd;
	return ev;
}

bool		isReservedChar(char c)
{
	std::string reserved = "#& =";
	return reserved.find(c) != std::string::npos;
}
