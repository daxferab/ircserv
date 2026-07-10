#include "Server.hpp"
#include "AReply.hpp"
#include "Client.hpp"
#include "../utils/colors.h"
#include "CommandHandler.hpp"

#include <cerrno>
#include <cstdio>
#include <exception>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <utility>
#include <signal.h>

#define MAX_EVENTS 16
#define BUFFERSIZE 512
#define LISTENING_QUEUE 5

// ---------------------------------------------------------------- CONSTRUCTORS

Server::Server(std::string name, std::string password) : _fd(-1), _name(name), _isRunning(false), _password(password){}

Server::~Server() {}

// ----------------------------------------------------- PUBLIC MEMBER FUNCTIONS

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
	if (_epoll >= 0)
	{
		epoll_ctl(_epoll, EPOLL_CTL_DEL, _fd, NULL);
		close(_epoll);
	}
	if (_fd >= 0) close(_fd);
	std::cout << std::endl << RED << "------------- THISCORD SERVER CLOSED! ------------" << RESET << std::endl;
}

std::string	Server::getName() const { return _name; }

void	Server::authClient(Client& client, const std::string& pass)
{
	t_rplContext	context;

	_fillContext(context, client, "", "", "PASS", "");

	if (client.isAuthenticated())
		_handleReply(client, AReply::getReply(462, context));
	else if (pass.empty())
		_handleReply(client, AReply::getReply(461, context));
	else if (pass != _password)
		_handleReply(client, AReply::getReply(464, context));
	else
		client.setAuthenticated(true);
}

void	Server::setClientNick(Client& client, const std::string& nick)
{
	t_rplContext	context;

	_fillContext(context, client, nick, "", "NICK", "");

	if (nick.empty())
		_handleReply(client, AReply::getReply(431, context));
	else if (isReservedChar(nick[0]))
		_handleReply(client, AReply::getReply(432, context));
	else if (_nickInUse(nick))
		_handleReply(client, AReply::getReply(433, context));
	else
	{
		client.setNick(nick);
		_fillContext(context, client, "", "", "", "");
		_handleReply(client, AReply::getReply(001, context));
	}
}

bool	Server::setClientUser(Client& client, const std::string& user)
{
	t_rplContext	context;

	_fillContext(context, client, "", "", "USER", "");

	if (client.isRegistered())
		_handleReply(client, AReply::getReply(462, context));
	else if (user.empty())
		_handleReply(client, AReply::getReply(461, context));
	else
	{
		client.setUser(user);
		return true;
	}
	return false;
}

void	Server::setClientName(Client& client, const std::string& name)
{
	client.setName(name);
}

void	Server::joinChannel(Client& client, const std::string& name, const std::string& key)
{
	t_rplContext	context;
	Channel			*channel = NULL;
	bool			invited;

	if(_channelExists(name))
		channel = &(_channels.at(name));
	invited = channel && false;//TODO invited

	_fillContext(context, client, client.getNick(), name, "JOIN", channel ? channel->getTopic() : "" );

	if (name.empty())
		_handleReply(client, AReply::getReply(461, context));
	else if(name[0] != '#')
		_handleReply(client, AReply::getReply(403, context));
	else if (channel && !invited && !channel->isKeyOk(key))
		_handleReply(client, AReply::getReply(475, context)); //TEST
	else if (channel && !invited && channel->isInviteOnly())
		_handleReply(client, AReply::getReply(473, context)); //TEST
	else if (channel && !invited && channel->isFull())
		_handleReply(client, AReply::getReply(471, context)); //TEST
	else
	{
		if (!channel)
		{
			_addChannel(Channel(name, client.getFd()));
			channel = &(_channels.at(name));
		}
		else
			channel->addUser(client.getFd());
		std::cout << MAGENTA << ":" + client.getNick() + " JOIN " + name + "\r\n" << RESET << std::endl;
		_handleReply(client, ":" + client.getNick() + " JOIN " + name + "\r\n");
		if (!channel->getTopic().empty())
			_handleReply(client, AReply::getReply(332, context)); //TEST
		_handleReply(client, AReply::getReply(353, context)); //TEST
		_handleReply(client, AReply::getReply(366, context)); //TEST
	}
}

// ---------------------------------------------------- PRIVATE MEMBER FUNCTIONS

void	Server::_setup(char* port)
{
	struct addrinfo	hints, *info; //NOTE: dont know if we need to free hints

	_createSignal(SIGINT, _handlesigint);
	_createSignal(SIGQUIT, SIG_IGN);
	
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
	freeaddrinfo(info);
	return false;
}

void	Server::_initEpoll()
{
	_epoll = epoll_create1(0);
	if (_epoll < 0) throw std::runtime_error("Error creating epoll");
	if (!setFdNonBlocking(_fd))
	{
		close(_fd);
		throw std::runtime_error("Error setting socket as non-blocking");
	}
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
			unsigned int ev = events[i].events;
			try
			{
				if (fd == _fd)
					_acceptClient();
				else {
					if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
						std::map<int, Client>::iterator it = _clients.find(fd);
						if (it != _clients.end()) _disconnectClient(it->second);
						continue;
					}
					if (ev & EPOLLIN) _readFd(fd);
					if (ev & EPOLLOUT) _writeFd(fd);
				}
			} catch (std::exception& e) {
				throw;
			}
		}
	}
}

void	Server::_acceptClient()
{
	struct sockaddr_storage	addr;
	socklen_t				addrlen = sizeof(addr);
	int						client_fd;

	while (true)
	{
		client_fd = accept(_fd, (struct sockaddr *)&addr, &addrlen);
		if (client_fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) break;
			throw std::runtime_error("Error accepting client");
		}
		if (!setFdNonBlocking(client_fd))
		{
			close(client_fd);
			throw std::runtime_error("Error setting socket as non-blocking");
		}
		_addClient(client_fd);
	}
}

void	Server::_createSignal(int signo, void (*handler)(int))
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(signo, &sa, NULL) == -1)
		throw std::runtime_error("Sigaction failed");
}

void	Server::_handlesigint(int signo)
{
	(void)signo;
	throw std::runtime_error("");
}

void	Server::_readFd(const int fd)
{
	char	buf[BUFFERSIZE];
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;
	while (true)
	{
		ssize_t n = recv(fd, buf, BUFFERSIZE, 0);
		if (n > 0)
			_handleLine(it->second, buf, n);
		else if (n == 0) {
			_disconnectClient(it->second);
			break;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			_disconnectClient(it->second);
			break;
		}
	}
}

void	Server::_handleLine(Client& client, char* line, int data)
{
	client.appendBuffer(line, data, IN);
	while (client.hasFullLine(IN))
	{		
		Message	message(client.getLine());
		if (message.isValid())
			if (!CommandHandler::execCommand(message, client, *this))
			{
				t_rplContext	context;
				_fillContext(context, client, "", "", "", "");
				_handleReply(client, AReply::getReply(451, context));
			}
	}
}

void	Server::_handleReply(Client& client, const std::string& message)
{
	client.appendBuffer(message.c_str(), message.size(), OUT);
	struct epoll_event client_ev = newEvent(client.getFd(), EPOLLOUT | EPOLLIN);
	epoll_ctl(_epoll, EPOLL_CTL_MOD, client.getFd(), &client_ev);
}

void	Server::_writeFd(const int fd)
{
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end()) return;
	Client &client = it->second;

	while (true)
	{
		const std::string &out = client.getOutBuffer();
		if (out.empty())
		{
			struct epoll_event ev = newEvent(fd, EPOLLIN);
			epoll_ctl(_epoll, EPOLL_CTL_MOD, fd, &ev);
			break;
		}
		ssize_t s = send(fd, out.c_str(), out.size(), 0);
		if (s > 0)
		{
			client.consumeOut((size_t)s);
			continue;
		}
		else if (s == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			break;
		else
		{
			_disconnectClient(client);
			break;
		}
	}
}

void	Server::_fillContext(t_rplContext& context, const Client& client, const std::string& nick, const std::string& channel, const std::string& command, const std::string& topic) const
{
	context.client = client.getNick();
	context.server = _name;
	context.nick = nick;
	context.channel = channel;
	context.command = command;
	context.topic = topic;
}

void	Server::_addClient(const int fd)
{
	struct epoll_event client_ev = newEvent(fd, EPOLLIN);
	epoll_ctl(_epoll, EPOLL_CTL_ADD, fd, &client_ev);

	std::pair<int, Client>	pair(fd, Client(fd));
	_clients.insert(pair);
	std::cout << GREEN << "Client " << fd << " connected" << RESET << std::endl;
}

void	Server::_disconnectClient(Client& client) //TODO: send QUIT reply
{
	int fd = client.getFd();
	std::map<int, Client>::iterator it = _clients.find(fd);

	std::cout << RED << "Client <" << client << "> disconnected" << RESET << std::endl;
	epoll_ctl(_epoll, EPOLL_CTL_DEL, fd, NULL);
	if (it != _clients.end())
		_clients.erase(it);
	close(fd);
}

void	Server::_addChannel(const Channel& channel)
{
	_channels.insert(std::pair<std::string, Channel>(channel.getName(), channel));
}

bool	Server::_nickInUse(const std::string nick) const
{
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end();it++)
		if (it->second.getNick().compare(nick) == 0) return true;
	return false;
}

bool	Server::_channelExists(const std::string name) const { return _channels.find(name) != _channels.end(); }

//------------------------------------------------------- OUT OF SCOPE FUNCTIONS

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

bool		setFdNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		return false;
	return true;
}
