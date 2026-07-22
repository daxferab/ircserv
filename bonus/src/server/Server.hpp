#ifndef SERVER_HPP
#define SERVER_HPP

#include "AReply.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "Bot.hpp"

#include <map>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>

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
		Bot								_bot;
		
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
		void		_handleReplyChannel(const Channel& channel, const std::string message, int client_fd);
		void		_writeFd(const int fd);
		void		_fillContext(t_rplContext& context, const std::string& target, const std::string& channel, const std::string& command, const std::string& message) const;

		void		_addClient(const int fd);
		void		_disconnectClient(Client& client);
		void		_addChannel(const Channel& channel);
		void		_deleteChannel(Channel& channel);

		bool		_nickInUse(const std::string nick) const;
		bool		_channelExists(const std::string name) const;
		Channel*	_getChannel(const std::string& name);
		int			_getClientFd(const std::string& nick) const;

	public:
		Server(std::string name, std::string password);
		~Server();
		void		start(char* port);
		void		stop();

		//---------------------------------------------------------------GETTERS

		std::string	getName() const;
		std::string	getChannelMembers(const std::string& channelName) const;
		std::string	getChannelTopic(const std::string& channelName) const;
		const Bot&	getBot() const;

		//--------------------------------------------------------------MESSAGES

		void		inviteUser(Client& client, const std::string& nick, const std::string& channelName);
		void		joinChannel(Client& client, const std::string& channel, const std::string& key);
		void		kickUser(Client& client, const std::string& channel, const std::string& nick, const std::string& reason);
		void		getMode(Client& client, std::string channel_name);
		void		setMode(Client& client, std::string channel_name, bool add, char type, std::string parameter);
		void		setClientNick(Client& client, const std::string& nick);
		void		partChannel(Client& client, const std::string& name, const std::string& reason);
		void		authClient(Client& client, const std::string& pass);
		void		sendMessage(Client& client, const std::string& target, const std::string& message);
		void		quitClient(Client& client, const std::string& msg);
		void		displayChannelTopic(Client& client, const std::string& channelName);
		void		setChannelTopic(Client& client, const std::string& channelName, const std::string& topic);
		void		setClientName(Client& client, const std::string& name);
		bool		setClientUser(Client& client, const std::string& user);
		void		dccSendFile(Client& client, const std::string& target, const std::string& message, bool isChecksum);
		void		botResponse(const std::string response, Client& target);

};

epoll_event	newEvent(int fd, int flags);
bool		isReservedChar(char c);
bool		setFdNonBlocking(int fd);

#endif
