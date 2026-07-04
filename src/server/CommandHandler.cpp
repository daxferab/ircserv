#include "CommandHandler.hpp"
#include "AReply.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include <cstring>

//------------------------------------------------------------- MEMBER FUNCTIONS
void	CommandHandler::execCommand(Message& command, Client& client, Server& server)
{
	if (command.getCommand() != PASS && !client.isAuthenticated())
	{
		AReply::getReply(451, client.getNick(), server.getName()); // TODO: call from server?
		return;
	}
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
		case NONE:
			;// handle invalid command
	}
	(void)client;
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

void	CommandHandler::_pass(const Message& command, Client& client, const Server& server)
{
	if (command.getParams().empty())
		server.authClient(client, "");
	else
		server.authClient(client, command.getParams()[0]);
}

void	CommandHandler::_nick(const Message& command, Client& client, const Server& server)
{
	if (command.getParams().empty())
		server.setClientNick(client, "");
	else
		server.setClientNick(client, command.getParams()[0]);
}

void	CommandHandler::_user(const Message& command, Client& client, const Server& server)
{
	bool	success;

	if (command.getParams().empty())
		success = server.setClientUser(client, ""); // need more params (false)
	else
		success = server.setClientUser(client, command.getParams()[0]); //check if already registered, if it is, return false
	if (success && command.getParams().size() == 4)
		server.setClientName(client, command.getParams()[3]);
}
