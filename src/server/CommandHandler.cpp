#include "CommandHandler.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include <cstring>
#include <unistd.h>
#include <string>
#include <sstream>
#include <vector>

//------------------------------------------------------------- MEMBER FUNCTIONS

bool	CommandHandler::execCommand(Message& command, Client& client, Server& server)
{
	if (command.getCommand() != PASS && !client.isAuthenticated())
		return false;
	switch (command.getCommand())
	{
		case PASS:
			_pass(command, client, server);
			break;
		case NICK:
			_nick(command, client, server);
			break;
		case USER:
			_user(command, client, server);
			break;
		case JOIN:
			_join(command, client, server);
			break;
		case INVITE:
			_invite(command, client, server);
			break;
		case PRIVMSG:
			_privmsg(command, client, server);
			break;
		case KICK:
			_kick(command, client, server);
			break;
		case PART:
			_part(command, client, server);
			break;
		case QUIT:
			_quit(command, client, server);
			break;
		case MODE:
			_mode(command, client, server);
			break;
		case NONE:
			break;// handle invalid command
		default:
			break;
	}
	return true;
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

void	CommandHandler::_pass(const Message& command, Client& client, Server& server)
{
	if (command.getParams().empty())
		server.authClient(client, "");
	else
		server.authClient(client, command.getParams()[0]);
}

void	CommandHandler::_nick(const Message& command, Client& client, Server& server)
{
	if (command.getParams().empty())
		server.setClientNick(client, "");
	else
		server.setClientNick(client, command.getParams()[0]);
}

void	CommandHandler::_user(const Message& command, Client& client, Server& server)
{
	bool	success;

	if (command.getParams().empty())
		success = server.setClientUser(client, ""); // need more params (false)
	else
		success = server.setClientUser(client, command.getParams()[0]); //check if already registered, if it is, return false
	if (success && command.getParams().size() == 4)
		server.setClientName(client, command.getParams()[3]);
}

void	CommandHandler::_join(const Message& command, Client& client, Server& server)
{
	std::vector<std::string>	clients, keys;

	if (command.getParams().size() >= 2)
	{
		clients = split(command.getParams()[0], ',');
		keys = split(command.getParams()[1], ',');

		for (size_t i = 0; i < clients.size(); ++i)
			server.joinChannel(client, clients[i], keys[i]);
	}
	else if (command.getParams().size() == 1)
	{
		clients = split(command.getParams()[0], ',');

		for (size_t i = 0; i < clients.size(); ++i)
			server.joinChannel(client, clients[i], "");
	}
}

void	CommandHandler::_part(const Message &command, Client &client, Server &server)
{
	std::vector<std::string>	channels = split(command.getParams()[0], ',');

	for (size_t i = 0; i < channels.size(); ++i)
		server.partChannel(client, channels[i], command.getParams()[1]);
}

void	CommandHandler::_invite(const Message& command, Client& client, Server& server)
{
	std::string	channel = command.getParams()[1].empty() ? "" : command.getParams()[1];

	server.inviteUser(client, command.getParams()[0], channel);
}

void	CommandHandler::_privmsg(const Message& command, Client& client, Server& server)
{
	std::vector<std::string>	clients;
	std::string					message = command.getParams()[1].empty() ? "" : command.getParams()[1];

	clients = split(command.getParams()[0], ',');
	for (size_t i = 0; i < clients.size(); ++i)
		server.sendMessage(client,  clients[i], message);
}

void	CommandHandler::_quit(const Message& command, Client& client, Server& server)
{
	if (command.getParams().empty())
		server.quitClient(client, "");
	server.quitClient(client, command.getParams()[0]);
}

void	CommandHandler::_kick(const Message& command, Client& client, Server& server)
{
	std::vector<std::string>	users = split(command.getParams()[1], ',');
	std::string					reason;

	if (command.getParams().size() > 2)
		reason = command.getParams()[2];
	else
		reason = "";

	for (size_t i = 0; i < users.size(); ++i)
	{
		server.kickUser(client, command.getParams()[0], users[i], reason);
	}
}

void	CommandHandler::_mode(const Message& command, Client& client, Server& server)
{

	bool	add = true;
	size_t	param_iter = 2;

	if (command.getParams().size() == 0)
		server.setMode(client, "", add, '\0', "");
	else if (command.getParams().size() > 1)
	{
		for (size_t i = 0; i < command.getParams()[1].size(); i++)
		{
			char c = command.getParams()[1][i];
			//TODO if not +-, add the char to some list and only use it once
			if (c == '+' || c == '-')
				add = c == '+';
			else if ((c == 'o' || (c == 'k' && add) || (c == 'l' && add)) && command.getParams().size() > param_iter)
				server.setMode(client, command.getParams()[0], add, c, command.getParams()[param_iter++]);
			else
				server.setMode(client, command.getParams()[0], add, c, "");
		}
	}
}

//------------------------------------------------------- OUT OF SCOPE FUNCTIONS

std::vector<std::string> split(const std::string& str, char delimiter)
{
	std::vector<std::string>	tokens;
	std::string					token;
	std::istringstream			iss(str);

	while (getline(iss, token, delimiter))
		tokens.push_back(token);
	return (tokens);
}
