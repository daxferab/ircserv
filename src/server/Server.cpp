#include "Server.hpp"
#include "AReply.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "../utils/colors.h"
#include "CommandHandler.hpp"
#include "Message.hpp"

#include <cerrno>
#include <cstdio>
#include <exception>
#include <map>
#include <netinet/in.h>
#include <set>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netdb.h>
#include <cstring>
#include <sys/ucontext.h>
#include <unistd.h>
#include <iostream>
#include <utility>
#include <signal.h>
#include <sstream>

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
	_fillContext(context, "", "", "PASS", "");

	if (client.isAuthenticated())
		_handleReply(client, AReply::getNReply(462, *this, client, context));
	else if (pass.empty())
		_handleReply(client, AReply::getNReply(461, *this, client, context));
	else if (pass != _password)
		_handleReply(client, AReply::getNReply(464, *this, client, context));
	else
		client.setAuthenticated(true);
}

void	Server::quitClient(Client& client, const std::string& msg)
{
	(void)msg;
	t_rplContext	context;

	_fillContext(context, "", "", "QUIT", msg);

	std::map<std::string, Channel>::iterator it = _channels.begin();
	std::map<std::string, Channel>::iterator end = _channels.end();

	for (; it != end; it++)
		_handleReplyChannel(it->second, AReply::getReply(QUIT, client, context), -1);
	_disconnectClient(client);
}

void	Server::setClientNick(Client& client, const std::string& nick)
{
	t_rplContext	context;
	_fillContext(context, nick, "", "NICK", "");

	if (nick.empty())
		_handleReply(client, AReply::getNReply(431, *this, client, context));
	else if (isReservedChar(nick[0]))
		_handleReply(client, AReply::getNReply(432, *this, client, context));
	else if (_nickInUse(nick))
		_handleReply(client, AReply::getNReply(433, *this, client, context));
	else
	{
		client.setNick(nick);
		_handleReply(client, AReply::getNReply(001, *this, client, context));
		_handleReply(client, AReply::getNReply(005, *this, client, context));
	}
}

bool	Server::setClientUser(Client& client, const std::string& user)
{
	t_rplContext	context;
	_fillContext(context, "", "", "USER", "");

	if (client.isRegistered())
		_handleReply(client, AReply::getNReply(462, *this, client, context));
	else if (user.empty())
		_handleReply(client, AReply::getNReply(461, *this, client, context));
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
	{
		channel = &(_channels.at(name));
		if (channel->isMember(client.getFd()))
			return ;
	}
	invited = channel && channel->isInvited(client.getFd());
	_fillContext(context, client.getNick(), name, "JOIN", "");

	if (name.empty())
		_handleReply(client, AReply::getNReply(461, *this, client, context));
	else if(name[0] != '#')
		_handleReply(client, AReply::getNReply(403, *this, client, context));
	else if (channel && !invited && !channel->isKeyOk(key))
		_handleReply(client, AReply::getNReply(475, *this, client, context)); //TEST
	else if (channel && !invited && channel->isInviteOnly())
		_handleReply(client, AReply::getNReply(473, *this, client, context)); //TEST
	else if (channel && !invited && channel->isFull())
		_handleReply(client, AReply::getNReply(471, *this, client, context)); //TEST
	else
	{
		if (!channel)
		{
			_addChannel(Channel(name, client.getFd()));
			channel = &(_channels.at(name));
		}
		else
			channel->addUser(client.getFd());
		_handleReplyChannel(*channel, AReply::getReply(JOIN, client, context), -1);
		if (!channel->getTopic().empty())
			_handleReply(client, AReply::getNReply(332, *this, client, context)); //TEST
		_handleReply(client, AReply::getNReply(353, *this, client, context)); //TEST
		_handleReply(client, AReply::getNReply(366, *this, client, context)); //TEST
	}
}

void	Server::inviteUser(Client &client, const std::string& nick, const std::string& channelName)
{
	int											userFd = _getClientFd(nick);
	std::map<std::string, Channel>::iterator	channelIt = _channels.find(channelName);
	t_rplContext								context;

	_fillContext(context, client.getNick(), channelName, "INVITE", "");
	if (channelIt == _channels.end())
		_handleReply(client, AReply::getNReply(403, *this, client, context));
	else if (!channelIt->second.isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(442, *this, client, context));
	else if (channelIt->second.isInviteOnly() && !channelIt->second.isOperator(client.getFd())) //test
		_handleReply(client, AReply::getNReply(482, *this, client, context));
	else if (channelIt->second.isMember(userFd))
	{
		context.target = nick;
		_handleReply(client, AReply::getNReply(443, *this, client, context));
	}
	else if (_clients.find(userFd) != _clients.end())
	{
		context.target = nick;
		channelIt->second.setInvitedUser(userFd);
		_handleReply(client, AReply::getNReply(341, *this, client, context));
		_handleReply(_clients.find(userFd)->second, AReply::getReply(INVITE, client, context));
	}
}

void	Server::setChannelTopic(Client& client, const std::string& channelName, const std::string& topic)
{
	std::map<std::string, Channel>::iterator	channelIt = _channels.find(channelName);
	Channel&									channel = channelIt->second;
	t_rplContext								context;

	_fillContext(context, "", channelName, "TOPIC", topic);
	if (channelIt == _channels.end())
		_handleReply(client, AReply::getNReply(403, *this, client, context));
	else if (!channel.isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(442, *this, client, context));
	else if (channel.isTopicRestricted() && !channel.isOperator(client.getFd()))
		_handleReply(client, AReply::getNReply(482, *this, client, context));
	else
	{
		_handleReplyChannel(channel, AReply::getReply(TOPIC, client, context), -1);
		channel.setTopic(topic);
	}
}

void	Server::displayChannelTopic(Client& client, const std::string& channelName)
{
	std::map<std::string, Channel>::iterator	channelIt = _channels.find(channelName);
	Channel&									channel = channelIt->second;
	t_rplContext								context;

	_fillContext(context, "", channelName, "TOPIC", "");
	if (channelIt == _channels.end())
		_handleReply(client, AReply::getNReply(403, *this, client, context));
	else if (!channel.isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(442, *this, client, context));	
	else if (channel.getTopic().empty())
		_handleReply(client, AReply::getNReply(331, *this, client, context));
	else
		_handleReply(client, AReply::getNReply(332, *this, client, context));
}

void	Server::kickUser(Client& client, const std::string& chanName, const std::string& nick, const std::string& reason)
{
	t_rplContext	context;
	Channel			*channel = NULL;

	_fillContext(context, nick, chanName, "KICK", reason);
	
	if (chanName.empty())
		_handleReply(client, AReply::getNReply(461, *this, client, context));
	else if(!_channelExists(chanName))
	{
		_handleReply(client, AReply::getNReply(403, *this, client, context));
		return;
	}
	channel = &(_channels.at(chanName));

	if(!channel->isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(442, *this, client, context));
	if(!channel->isOperator(client.getFd()))
		_handleReply(client, AReply::getNReply(482, *this, client, context));
	else if (!channel->isMember(_getClientFd(nick)))
		_handleReply(client, AReply::getNReply(441, *this, client, context));
	else
	{
	_handleReplyChannel(*channel, AReply::getReply(KICK, client, context), -1);
		channel->removeUser(_getClientFd(nick));
	}
}

void	Server::partChannel(Client &client, const std::string &name, const std::string &reason)
{
	t_rplContext	context;
	Channel			*channel = NULL;

	if(_channelExists(name))
		channel = &(_channels.at(name));
	_fillContext(context, client.getNick(), name, "PART", reason);

	if (name.empty())														//Not enough params
		_handleReply(client, AReply::getNReply(461, *this, client, context));
	else if (!channel)														//Channel does NOT exit
		_handleReply(client, AReply::getNReply(403, *this, client, context));
	else if (channel && !channel->isMember(client.getFd()))					//Channel exists but client is not a member
		_handleReply(client, AReply::getNReply(442, *this, client, context));
	else
	{
		_handleReplyChannel(*channel, AReply::getReply(PART, client, context), -1);
		channel->removeUser(client.getFd());
	}
}

std::string	Server::getChannelTopic(const std::string& channelName) const
{
	std::map<std::string, Channel>::const_iterator it = _channels.find(channelName);
	if (it == _channels.end())
		return "";
	return it->second.getTopic();
}

std::string	Server::getChannelMembers(const std::string& channelName) const
{
	std::map<std::string, Channel>::const_iterator it = _channels.find(channelName);
	if (it == _channels.end())
		return "";

	const std::set<int>&	users = it->second.getUsersList();
	std::string				list;

	for (std::set<int>::const_iterator fdIt = users.begin(); fdIt != users.end(); ++fdIt)
	{
		std::map<int, Client>::const_iterator clientIt = _clients.find(*fdIt);
		if (clientIt == _clients.end() || clientIt->second.getNick().empty())
			continue;
		if (!list.empty())
			list.append(" ");
		if (it->second.isOperator(clientIt->first)) list.append("@");
		list.append(clientIt->second.getNick());
	}
	return list;
}

void	Server::sendMessage(Client& client, const std::string& target, const std::string& message)
{
	t_rplContext	context;
	int targetFd = _getClientFd(target);
	
	_fillContext(context, target, "", "PRIVMSG", message);
	
	if (target.empty())
		_handleReply(client, AReply::getNReply(411, *this, client, context));
	else if (message.empty())
		_handleReply(client, AReply::getNReply(412, *this, client, context));
	else if (targetFd > 0)
		_sendPrivate(client, targetFd, context);
	else if(_channelExists(target))
		_sendPublic(client, target, context);
	else
		_handleReply(client, AReply::getNReply(401, *this, client, context));
}

void	Server::setMode(Client& client, std::string channel_name, bool add, char type, std::string parameter)
{
	t_rplContext	context;
	Channel			*channel = NULL;
	bool			changes = false;

	_fillContext(context, client.getNick(), channel_name, "MODE", std::string(1, type));
	if (channel_name.empty())
		_handleReply(client, AReply::getNReply(461, *this, client, context));
	else if (!_channelExists(channel_name))
		_handleReply(client, AReply::getNReply(403, *this, client, context));
	else
	{
		channel = &(_channels.at(channel_name));
		if (!channel->isMember(client.getFd()))
			_handleReply(client, AReply::getNReply(442, *this, client, context));
		else if (!channel->isOperator(client.getFd()))
			_handleReply(client, AReply::getNReply(482, *this, client, context));
		else if (type == 'i')
		{
			changes = channel->setInviteOnly(add);
			_fillContext(context, client.getNick(), channel_name, "MODE", (add ? "+i" : "-i"));
		}
		else if (type == 'k')
		{
			if (!(add && parameter.empty()) && channel->setKey(parameter))
			{
				changes = true;
				_fillContext(context, client.getNick(), channel_name, "MODE", (add ? "+k " : "-k ") + parameter);
			}
		}
		else if (type == 'l')
		{
			int n = channel->setUserLimit(parameter);
			std::stringstream ss;
			ss << n;
			changes = n >= 0;
			if (n > 0)
				_fillContext(context, client.getNick(), channel_name, "MODE", "+l " + ss.str());
			if (n == 0)
				_fillContext(context, client.getNick(), channel_name, "MODE", "-l" );
		}
		else if (type == 'o' && add)
		{
			changes = channel->setOperator(_getClientFd(parameter));
			_fillContext(context, client.getNick(), channel_name, "MODE", "+o " + parameter);
		}
		else if (type == 'o' && !add)
		{
			changes = channel->unsetOperator(_getClientFd(parameter));
			_fillContext(context, client.getNick(), channel_name, "MODE", "-o " + parameter);
		}
		else if (type == 't')
		{
			changes = channel->setTopicRestricted(add);
			_fillContext(context, client.getNick(), channel_name, "MODE", (add ? "+t" : "-t"));
		}
		else
			_handleReply(client, AReply::getNReply(472, *this, client, context));
	}
	if (changes)
		_handleReplyChannel(*channel, AReply::getReply(MODE, client, context), -1);
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
			quitClient(it->second, "");
			break;
		} else
			break;
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
				_handleReply(client, AReply::getNReply(451, *this, client, context));
			}
	}
}

void	Server::_handleReply(Client& client, const std::string& message)
{
	client.appendBuffer(message.c_str(), message.size(), OUT);
	struct epoll_event client_ev = newEvent(client.getFd(), EPOLLOUT | EPOLLIN);
	epoll_ctl(_epoll, EPOLL_CTL_MOD, client.getFd(), &client_ev);
}

void	Server::_handleReplyChannel(const Channel& channel, const std::string message, int client_fd)
{
	std::set<int>	clients = channel.getUsersList();
	for (std::set<int>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		Client& client = _clients.find(*it)->second;
		if (client_fd != client.getFd())
			_handleReply(client, message);
	}
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
			quitClient(client, "");
			break;
		}
	}
}

void	Server::_fillContext(t_rplContext& context, const std::string& target, const std::string& channel, const std::string& command, const std::string& message) const
{
	context.target = target;
	context.channel = channel;
	context.command = command;
	context.message = message;
}

void	Server::_sendPrivate(const Client& client, const int fd, t_rplContext& context)
{
	_handleReply(_clients.find(fd)->second, AReply::getReply(PRIVMSG, client, context));
}

void	Server::_sendPublic(Client& client, const std::string& channelName, t_rplContext& context)
{
	Channel	channel = _channels.find(channelName)->second;

	if (!channel.isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(404, *this, client, context));

	std::set<int>			members = channel.getUsers();
	std::set<int>::iterator	it = members.begin();
	std::set<int>::iterator	end = members.end();

	for (; it != end; it++)
		if (_clients.find(*it)->first != client.getFd())
			_handleReply(_clients.find(*it)->second, AReply::getReply(PRIVMSG, client, context));
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
	t_rplContext	context;

	_fillContext(context, client.getNick(), "", "", "Client disconnected");

	_handleReply(client, AReply::getReply(ERROR, client, context));
	std::cout << RED << "Client <" << client << "> disconnected" << RESET << std::endl;
	epoll_ctl(_epoll, EPOLL_CTL_DEL, fd, NULL);
	std::map<std::string, Channel>::iterator itc = _channels.begin();
	std::map<std::string, Channel>::iterator end = _channels.end();

	for (; itc != end; ++itc)
		partChannel(client, itc->first, context.message);

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

int		Server::_getClientFd(const std::string& nick) const
{
	std::map<int, Client>::const_iterator itc = _clients.begin();
	std::map<int, Client>::const_iterator end = _clients.end();

	for (; itc != end; ++itc)
		if (itc->second.getNick() == nick)
			return itc->second.getFd();
	return -1;
}

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
