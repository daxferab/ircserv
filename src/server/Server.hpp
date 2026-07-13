#ifndef SERVER_HPP
#define SERVER_HPP

#include "AReply.hpp"
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
		std::string						_name;
		int								_epoll;
		bool							_isRunning;
		std::map<int, Client>			_clients;
		std::map<std::string, Channel>	_channels;
		const std::string				_password;

		void		_setup(char* port);
		bool		_createSocket(struct addrinfo *info);
		void		_initEpoll();
		void		_eventLoop();
		void		_acceptClient();
	
		void		_createSignal(int signo, void (*handler)(int));
		static void	_handlesigint(int signo);

		void		_readFd(const int fd);
		void		_handleLine(Client& client, char* line, int data);
		void		_handleReply(Client& client, const std::string& message);
		void		_writeFd(const int fd);
		void		_fillContext(t_rplContext& context, const std::string& nick, const std::string& channel, const std::string& command, const std::string& reason) const;
		
		void		_addClient(const int fd);
		void		_disconnectClient(Client& client);
		void		_addChannel(const Channel& channel);

		bool		_nickInUse(const std::string nick) const;
		bool		_channelExists(const std::string name) const;
		int			_getClientFd(const std::string& nick) const;

	public:
		Server(std::string name, std::string password);
		~Server();
		void		start(char* port);
		void		stop();

		std::string	getName() const;
		void		authClient(Client& client, const std::string& pass);
		void		quitClient(Client& client, const std::string& msg);
		void		setClientNick(Client& client, const std::string& nick);
		bool		setClientUser(Client& client, const std::string& user);
		void		setClientName(Client& client, const std::string& name);
		void		joinChannel(Client& client, const std::string& channel, const std::string& key);
		void		kickUser(Client& client, const std::string& channel, const std::string& nick, const std::string& reason);
		void		partChannel(Client& client, const std::string& name, const std::string& reason);
		std::string	getChannelTopic(const std::string& channelName) const;
		std::string	getChannelMembers(const std::string& channelName) const;
};

epoll_event	newEvent(int fd, int flags);
bool		isReservedChar(char c);
bool		setFdNonBlocking(int fd);

#endif
