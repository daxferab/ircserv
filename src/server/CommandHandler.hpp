#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Client.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include <vector>

class CommandHandler
{
	private:
		CommandHandler();

		static void	_pass(const Message& command, Client& client, Server& server);
		static void	_nick(const Message& command, Client& client, Server& server);
		static void	_user(const Message& command, Client& client, Server& server);
		static void	_join(const Message& command, Client& client, Server& server);
		static void	_privmsg(const Message& command, Client& client, Server& server);
		static void	_kick(const Message& command, Client& client, Server& server);
		static void	_part(const Message& command, Client& client, Server& server);
		static void	_quit(const Message& command, Client& client, Server& server);
		static void	_mode(const Message& command, Client& client, Server& server);

	public:
		static bool	execCommand(Message& command, Client& client, Server& server);
};

std::vector<std::string> split(const std::string& str, char delimiter);

#endif
