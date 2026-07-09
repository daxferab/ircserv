#include "CommandHandler.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include <cstring>

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
		case QUIT:
			_quit(command, client, server);
			break;
		case NONE:
			;// handle invalid command
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
	server.joinChannel(client, command.getParams()[0], command.getParams()[1]);
}

void	CommandHandler::_quit(const Message& command, Client& client, Server& server)
{
	if (command.getParams().empty())
		server.quitClient(client, "");
	server.quitClient(client, command.getParams()[0]);
}
