#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <vector>

class Client;
class Message;
class Server;

class CommandHandler
{
	private:
		CommandHandler();

		static void	_invite(const Message& command, Client& client, Server& server);
		static void	_join(const Message& command, Client& client, Server& server);
		static void	_kick(const Message& command, Client& client, Server& server);
		static void	_mode(const Message& command, Client& client, Server& server);
		static void	_nick(const Message& command, Client& client, Server& server);
		static void	_part(const Message& command, Client& client, Server& server);
		static void	_pass(const Message& command, Client& client, Server& server);
		static void	_privmsg(const Message& command, Client& client, Server& server);
		static void	_quit(const Message& command, Client& client, Server& server);
		static void	_topic(const Message& command, Client& client, Server& server);
		static void	_user(const Message& command, Client& client, Server& server);
		static void	_dcc(const Message& command, Client& client, Server& server);

	public:
		static bool	execCommand(Message& command, Client& client, Server& server);
};

std::vector<std::string>	split(const std::string& str, char delimiter);
bool						isDcc(const std::string& message);
bool						isChecksum(const std::string& message);

#endif
