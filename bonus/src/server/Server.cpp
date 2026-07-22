#include "Server.hpp"
#include "AReply.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "colors.h"
#include "CommandHandler.hpp"
#include "Message.hpp"

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <set>
#include <signal.h>
#include <stdexcept>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <unistd.h>
#include <utility>

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
	std::cout << RED << "------------- THISCORD SERVER CLOSED! ------------" << RESET;
}

//-----------------------------------------------------------------------GETTERS

std::string	Server::getName() const { return _name; }

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

const Bot&	Server::getBot() const { return _bot; }

//----------------------------------------------------------------------MESSAGES

void	Server::inviteUser(Client &client, const std::string& nick, const std::string& channelName)
{
	int											userFd = _getClientFd(nick);
	std::map<std::string, Channel>::iterator	channelIt = _channels.find(channelName);
	t_rplContext								context;

	_fillContext(context, client.getNick(), channelName, "INVITE", "");
	if (nick.empty() || channelName.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if (channelIt == _channels.end())
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else if (!channelIt->second.isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_NOTONCHANNEL, *this, client, context));
	else if (!channelIt->second.isOperator(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_CHANOPRIVSNEEDED, *this, client, context));
	else if (channelIt->second.isMember(userFd))
	{
		context.target = nick;
		_handleReply(client, AReply::getNReply(ERR_USERONCHANNEL, *this, client, context));
	}
	else if (_clients.find(userFd) != _clients.end())
	{
		context.target = nick;
		channelIt->second.setInvitedUser(userFd);
		_handleReply(client, AReply::getNReply(RPL_INVITING, *this, client, context));
		_handleReply(_clients.find(userFd)->second, AReply::getReply(INVITE, client, context));
	}
}

void	Server::joinChannel(Client& client, const std::string& name, const std::string& key)
{
	t_rplContext	context;
	Channel			*channel = _getChannel(name);
	bool			invited;

	invited = channel && channel->isInvited(client.getFd());
	_fillContext(context, client.getNick(), name, "JOIN", "");
	if (name.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if(name[0] != '#')
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else if(channel && channel->isMember(client.getFd()))
		return ;
	else if (channel && !invited && !channel->isKeyOk(key))
		_handleReply(client, AReply::getNReply(ERR_BADCHANNELKEY, *this, client, context));
	else if (channel && !invited && channel->isInviteOnly())
		_handleReply(client, AReply::getNReply(ERR_INVITEONLYCHAN, *this, client, context));
	else if (channel && !invited && channel->isFull())
		_handleReply(client, AReply::getNReply(ERR_CHANNELISFULL, *this, client, context));
	else
	{
		if (!channel)
		{
			_addChannel(Channel(name, client.getFd()));
			channel = _getChannel(name);
		}
		else
			channel->addUser(client.getFd());
		_handleReplyChannel(*channel, AReply::getReply(JOIN, client, context), -1);
		if (!channel->getTopic().empty())
			_handleReply(client, AReply::getNReply(RPL_TOPIC, *this, client, context));
		_handleReply(client, AReply::getNReply(RPL_NAMREPLY, *this, client, context));
		_handleReply(client, AReply::getNReply(RPL_ENDOFNAMES, *this, client, context));
	}
}

void	Server::kickUser(Client& client, const std::string& chanName, const std::string& nick, const std::string& reason)
{
	t_rplContext	context;
	Channel			*channel = _getChannel(chanName);

	_fillContext(context, nick, chanName, "KICK", reason);
	if (chanName.empty() || nick.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if(!channel)
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else if(!channel->isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_NOTONCHANNEL, *this, client, context));
	else if(!channel->isOperator(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_CHANOPRIVSNEEDED, *this, client, context));
	else if (!channel->isMember(_getClientFd(nick)))
		_handleReply(client, AReply::getNReply(ERR_USERNOTINCHANNEL, *this, client, context));
	else
	{
		_handleReplyChannel(*channel, AReply::getReply(KICK, client, context), -1);
		channel->removeUser(_getClientFd(nick));
		if (channel->getUserCount() < 1) _deleteChannel(*channel);
	}
}

void	Server::getMode(Client& client, std::string channel_name)
{
	t_rplContext	context;

	_fillContext(context, client.getNick(), channel_name, "MODE", "");
	if (channel_name.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if (!_channelExists(channel_name))
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else
	{
		_fillContext(context, client.getNick(), channel_name, "MODE", _channels.at(channel_name).getModes());
		_handleReply(client, AReply::getNReply(RPL_CHANNELMODEIS, *this, client, context));
	}
}

void	Server::setMode(Client& client, std::string channel_name, bool add, char type, std::string parameter)
{
	t_rplContext		context;
	Channel				*channel = _getChannel(channel_name);
	bool				changes = false;
	std::stringstream	ss;
	int					n;

	_fillContext(context, client.getNick(), channel_name, "MODE", std::string(1, type));
	if (channel_name.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if (!channel)
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else if (!channel->isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_NOTONCHANNEL, *this, client, context));
	else if (!channel->isOperator(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_CHANOPRIVSNEEDED, *this, client, context));
	else
	{
		switch (type)
		{
		case 'i':
			changes = channel->setInviteOnly(add);
			_fillContext(context, client.getNick(), channel_name, "MODE", (add ? "+i" : "-i"));
			break;
		case 'k':
			if (!(add && parameter.empty()) && channel->setKey(parameter))
			{
				changes = true;
				_fillContext(context, client.getNick(), channel_name, "MODE", (add ? "+k " : "-k ") + parameter);
			}
			break;
		case 'l':
			n = channel->setUserLimit(parameter);
			ss << n;
			changes = n >= 0;
			if (n > 0)
				_fillContext(context, client.getNick(), channel_name, "MODE", "+l " + ss.str());
			if (n == 0)
				_fillContext(context, client.getNick(), channel_name, "MODE", "-l" );
			break;
		case 'o':
			changes = channel->changeOperator(_getClientFd(parameter), add);
			_fillContext(context, client.getNick(), channel_name, "MODE", (add ? "+o " : "-o ") + parameter);
			break;
		case 't':
			changes = channel->setTopicRestricted(add);
			_fillContext(context, client.getNick(), channel_name, "MODE", (add ? "+t" : "-t"));
			break;
		default:
			_handleReply(client, AReply::getNReply(ERR_UNKNOWNMODE, *this, client, context));
			break;
		}
	}
	if (changes)
		_handleReplyChannel(*channel, AReply::getReply(MODE, client, context), -1);
}

void	Server::setClientNick(Client& client, const std::string& nick)
{
	t_rplContext	context;
	_fillContext(context, nick, "", "NICK", "");

	if (nick.empty())
		_handleReply(client, AReply::getNReply(ERR_NONICKNAMEGIVEN, *this, client, context));
	else if (nick == client.getNick())
		return;
	else if (isReservedChar(nick[0]))
		_handleReply(client, AReply::getNReply(ERR_ERRONEUSNICKNAME, *this, client, context));
	else if (_nickInUse(nick))
		_handleReply(client, AReply::getNReply(ERR_NICKNAMEINUSE, *this, client, context));
	else if (client.getNick().empty())
	{
		client.setNick(nick);
		if (!client.getUser().empty())
		{
			_handleReply(client, AReply::getNReply(RPL_WELCOME, *this, client, context));
			_handleReply(client, AReply::getNReply(RPL_ISUPPORT, *this, client, context));
			client.setRegistered();
		}
	}
	else
	{
		context.target = client.getNick();
		client.setNick(nick);

		std::map<int, Client>::iterator it = _clients.begin();
		std::map<int, Client>::iterator end = _clients.end();
		for (; it != end; it++)
			_handleReply(it->second, AReply::getReply(NICK, client, context));
	}
}

void	Server::partChannel(Client &client, const std::string &name, const std::string &reason)
{
	t_rplContext	context;
	Channel			*channel = _getChannel(name);

	_fillContext(context, client.getNick(), name, "PART", reason);
	if (name.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if (!channel)
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else if (!channel->isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_NOTONCHANNEL, *this, client, context));
	else
	{
		_handleReplyChannel(*channel, AReply::getReply(PART, client, context), -1);
		channel->removeUser(client.getFd());
		if (channel->getUserCount() < 1) _deleteChannel(*channel);
	}
}

void	Server::authClient(Client& client, const std::string& pass)
{
	t_rplContext	context;
	_fillContext(context, "", "", "PASS", "");

	if (client.isAuthenticated())
		_handleReply(client, AReply::getNReply(ERR_ALREADYREGISTERED, *this, client, context));
	else if (pass.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if (pass != _password)
		_handleReply(client, AReply::getNReply(ERR_PASSWDMISMATCH, *this, client, context));
	else
		client.setAuthenticated(true);
}

void	Server::sendMessage(Client& client, const std::string& target, const std::string& message)
{
	t_rplContext	context;
	int 			targetFd = _getClientFd(target);
	Channel*		channel = _getChannel(target);

	_fillContext(context, target, "", "PRIVMSG", message);

	if (target.empty())
		_handleReply(client, AReply::getNReply(ERR_NORECIPIENT, *this, client, context));
	else if (message.empty())
		_handleReply(client, AReply::getNReply(ERR_NOTEXTTOSEND, *this, client, context));
	else if (targetFd > 0)
		_handleReply(_clients.find(targetFd)->second, AReply::getReply(PRIVMSG, client, context));
	else if(channel)
	{
		if (channel->isMember(client.getFd())) 
			_handleReplyChannel(_channels.find(target)->second, AReply::getReply(PRIVMSG, client, context), client.getFd());
		else
			_handleReply(client, AReply::getNReply(ERR_CANNOTSENDTOCHAN, *this, client, context));
	}
	else
		_handleReply(client, AReply::getNReply(ERR_NOSUCHNICK, *this, client, context));
}

void	Server::quitClient(Client& client, const std::string& msg)
{
	t_rplContext	context;

	_fillContext(context, "", "", "QUIT", msg);

	std::map<std::string, Channel>::iterator it = _channels.begin();
	std::map<std::string, Channel>::iterator end = _channels.end();

	for (; it != end; it++)
		_handleReplyChannel(it->second, AReply::getReply(QUIT, client, context), -1);
	_disconnectClient(client);
}

void	Server::displayChannelTopic(Client& client, const std::string& channelName)
{
	t_rplContext	context;
	Channel*		channel = _getChannel(channelName);

	_fillContext(context, "", channelName, "TOPIC", "");
	if (channelName.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else if (!channel)
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else if (channel->getTopic().empty())
		_handleReply(client, AReply::getNReply(RPL_NOTOPIC, *this, client, context));
	else
		_handleReply(client, AReply::getNReply(RPL_TOPIC, *this, client, context));
}

void	Server::setChannelTopic(Client& client, const std::string& channelName, const std::string& topic)
{
	t_rplContext	context;
	Channel*		channel = _getChannel(channelName);

	_fillContext(context, "", channelName, "TOPIC", topic);
	if (!channel)
		_handleReply(client, AReply::getNReply(ERR_NOSUCHCHANNEL, *this, client, context));
	else if (!channel->isMember(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_NOTONCHANNEL, *this, client, context));
	else if (channel->isTopicRestricted() && !channel->isOperator(client.getFd()))
		_handleReply(client, AReply::getNReply(ERR_CHANOPRIVSNEEDED, *this, client, context));
	else
	{
		_handleReplyChannel(*channel, AReply::getReply(TOPIC, client, context), -1);
		channel->setTopic(topic);
	}
}

void	Server::setClientName(Client& client, const std::string& name)
{
	client.setName(name);
}

bool	Server::setClientUser(Client& client, const std::string& user)
{
	t_rplContext	context;
	_fillContext(context, "", "", "USER", "");

	if (client.isRegistered())
		_handleReply(client, AReply::getNReply(ERR_ALREADYREGISTERED, *this, client, context));
	else if (user.empty())
		_handleReply(client, AReply::getNReply(ERR_NEEDMOREPARAMS, *this, client, context));
	else
	{
		client.setUser(user);
		if (!client.getNick().empty())
		{
			client.setRegistered();
			_handleReply(client, AReply::getNReply(RPL_WELCOME, *this, client, context));
			_handleReply(client, AReply::getNReply(RPL_ISUPPORT, *this, client, context));	
		}
		return true;
	}
	return false;
}

void	Server::dccSendFile(Client& client, const std::string& target, const std::string& message, bool isChecksum)
{
	t_rplContext	context;
	int				targetFd = _getClientFd(target);

	_fillContext(context, target, "", "PRIVMSG", message);

	if (isChecksum && _getClientFd(target) == -1)
		return ;

	if (target.empty())
		_handleReply(client, AReply::getNReply(ERR_NORECIPIENT, *this, client, context));
	else if (message.empty())
		_handleReply(client, AReply::getNReply(ERR_NOTEXTTOSEND, *this, client, context));
	else if (targetFd > 0)
		_handleReply(_clients.find(targetFd)->second, AReply::getReply(PRIVMSG, client, context));
	else
		_handleReply(client, AReply::getNReply(ERR_NOSUCHNICK, *this, client, context));
}

void Server::botResponse(const std::string response, Client& target)
{
	t_rplContext	context;
	std::stringstream reply;

	_fillContext(context, target.getNick(), "", "PRIVMSG", response);

	reply << ":" << _bot.getName() << " PRIVMSG " << context.target << " :" << context.message << "\r\n";
	_handleReply(target, reply.str());
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
			break;
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
	std::cout << std::endl;
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
				_handleReply(client, AReply::getNReply(ERR_NOTREGISTERED, *this, client, context));
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
		else if (s == -1)
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

	std::map<std::string, Channel>::iterator itc = _channels.begin();
	std::map<std::string, Channel>::iterator end = _channels.end();

	for (; itc != end; ++itc)
		if (itc->second.isMember(client.getFd()))
			partChannel(client, itc->first, context.message);

	_fillContext(context, client.getNick(), "", "", "Client disconnected");

	_handleReply(client, AReply::getReply(ERROR, client, context));
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

void	Server::_deleteChannel(Channel& channel)
{
	_channels.erase(_channels.find(channel.getName()));
}

bool	Server::_nickInUse(const std::string nick) const
{
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end();it++)
		if (it->second.getNick().compare(nick) == 0) return true;
	if (_bot.getName().compare(nick) == 0) return true;
	return false;
}

bool	Server::_channelExists(const std::string name) const { return _channels.find(name) != _channels.end(); }

Channel*	Server::_getChannel(const std::string& name)
{
	std::map<std::string, Channel>::iterator	channelIt = _channels.find(name);
	if (channelIt == _channels.end())
		return NULL;
	return &channelIt->second;
}

int			Server::_getClientFd(const std::string& nick) const
{
	std::map<int, Client>::const_iterator itc = _clients.begin();
	std::map<int, Client>::const_iterator end = _clients.end();

	for (; itc != end; ++itc)
		if (itc->second.getNick() == nick)
			return itc->second.getFd();
	return -1;
}

//------------------------------------------------------- OUT OF SCOPE FUNCTIONS

epoll_event	newEvent(int fd, int flags)
{
	epoll_event	ev;
	ev.events = flags;
	ev.data.fd = fd;
	return ev;
}

bool	isReservedChar(char c)
{
	std::string reserved = "#& =";
	return reserved.find(c) != std::string::npos;
}

bool	setFdNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		return false;
	return true;
}
