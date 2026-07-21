#include "CommandHandler.hpp"
#include "AReply.hpp"
#include "Message.hpp"
#include "Server.hpp"

#include <cstring>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

//------------------------------------------------------------- MEMBER FUNCTIONS

bool	CommandHandler::execCommand(Message& command, Client& client, Server& server)
{
	if (command.getCommand() == NONE) return true;
	if (!client.isAuthenticated() && command.getCommand() != PASS && command.getCommand() != QUIT)
		return false;
	if (!client.isRegistered() && command.getCommand() != PASS && command.getCommand() != QUIT && command.getCommand() != NICK && command.getCommand() != USER)
		return false;
	switch (command.getCommand())
	{
		case INVITE:
			_invite(command, client, server);
			break;
		case JOIN:
			_join(command, client, server);
			break;
		case KICK:
			_kick(command, client, server);
			break;
		case MODE:
			_mode(command, client, server);
			break;
		case NICK:
			_nick(command, client, server);
			break;
		case PART:
			_part(command, client, server);
			break;
		case PASS:
			_pass(command, client, server);
			break;
		case PRIVMSG:
			_privmsg(command, client, server);
			break;
		case QUIT:
			_quit(command, client, server);
			break;
		case TOPIC:
			_topic(command, client, server);
			break;
		case USER:
			_user(command, client, server);
			break;
		default:
			break;
	}
	return true;
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

void	CommandHandler::_invite(const Message& command, Client& client, Server& server)
{
	server.inviteUser(client, command.getParams()[0], command.getParams()[1]);
}

void	CommandHandler::_join(const Message& command, Client& client, Server& server)
{
	std::vector<std::string>	clients, keys;

	if (command.getParams()[1].empty())
	{
		clients = split(command.getParams()[0], ',');

		for (size_t i = 0; i < clients.size(); ++i)
			server.joinChannel(client, clients[i], "");
	}
	else
	{
		clients = split(command.getParams()[0], ',');
		keys = split(command.getParams()[1], ',');

		for (size_t i = 0; i < clients.size(); ++i)
			server.joinChannel(client, clients[i], keys[i]);
	}
}

void	CommandHandler::_kick(const Message& command, Client& client, Server& server)
{
	std::vector<std::string>	users = split(command.getParams()[1], ',');

	for (size_t i = 0; i < users.size(); ++i)
		server.kickUser(client, command.getParams()[0], users[i], command.getParams()[2]);
}

void	CommandHandler::_mode(const Message& command, Client& client, Server& server)
{

	bool			add = true;
	size_t			param_iter = 2;
	char			c;
	std::set<char>	seen;

	if (command.getParams()[0].empty())
		server.setMode(client, "", add, '\0', "");
	else if (command.getParams()[1].empty())
		server.getMode(client, command.getParams()[0]);
	else
	{
		for (size_t i = 0; i < command.getParams()[1].size(); i++)
		{
			c = command.getParams()[1][i];
			if (c == '+' || c == '-')
				add = c == '+';
			else if (c == 'i' || c == 'l' || c == 'k' || c == 'o' || c == 't')
			{
				if (seen.find(c) != seen.end())
					continue ;
				seen.insert(c);
				if ((c == 'o' || (c == 'k' && add) || (c == 'l' && add)))
					server.setMode(client, command.getParams()[0], add, c, command.getParams()[param_iter++]);
				else
					server.setMode(client, command.getParams()[0], add, c, "");
			}
			else
				server.setMode(client, command.getParams()[0], add, c, "");
		}
	}
}

void	CommandHandler::_nick(const Message& command, Client& client, Server& server)
{
	server.setClientNick(client, command.getParams()[0]);
}

void	CommandHandler::_part(const Message &command, Client &client, Server &server)
{
	std::vector<std::string>	channels = split(command.getParams()[0], ',');

	for (size_t i = 0; i < channels.size(); ++i)
		server.partChannel(client, channels[i], command.getParams()[1]);
}

void	CommandHandler::_pass(const Message& command, Client& client, Server& server)
{
	server.authClient(client, command.getParams()[0]);
}

void	CommandHandler::_privmsg(const Message& command, Client& client, Server& server)
{
	std::vector<std::string>	clients = split(command.getParams()[0], ',');

	for (size_t i = 0; i < clients.size(); ++i)
		server.sendMessage(client, clients[i], command.getParams()[1]);
}

void	CommandHandler::_quit(const Message& command, Client& client, Server& server)
{
	server.quitClient(client, command.getParams()[0]);
}

void	CommandHandler::_topic(const Message& command, Client& client, Server& server)
{
	if (command.getParams()[1].empty())
		server.displayChannelTopic(client, command.getParams()[0]);
	else
		server.setChannelTopic(client, command.getParams()[0], command.getParams()[1]);
}

void	CommandHandler::_user(const Message& command, Client& client, Server& server)
{
	bool	success = server.setClientUser(client, command.getParams()[0]);

	if (success && command.getParams().size() == 4)
		server.setClientName(client, command.getParams()[3]);
}

//------------------------------------------------------- OUT OF SCOPE FUNCTIONS

std::vector<std::string>	split(const std::string& str, char delimiter)
{
	std::vector<std::string>	tokens;
	std::string					token;
	std::istringstream			iss(str);

	while (getline(iss, token, delimiter))
		tokens.push_back(token);
	if (str.empty())
		tokens.push_back("");
	else if (str[str.length() - 1] == delimiter)
		tokens.push_back("");
	return tokens;
}
