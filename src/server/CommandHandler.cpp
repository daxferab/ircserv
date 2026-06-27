#include "CommandHandler.hpp"
#include "Message.hpp"
#include <cstring>
#include <iostream>

//------------------------------------------------------------- MEMBER FUNCTIONS
void	CommandHandler::execCommand(Message& command, Client& client, Server& server)
{
	if (command.getCommand() != PASS || !client.isAuthenticated())
		{
			std::cout << "user " << client.getNick() << " not registered" << std::endl;
			return;
		}// ERR_NOTREGISTERED 451
	switch (command.getCommand())
	{
		case PASS:
			_pass(command, client, server);
			break;
		case NICK:
			_nick(command, client, server);
			break;
		case USER:
			// _user(command, client);
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
	server.authClient(client, command.getParams()[0]);
}

void	CommandHandler::_nick(const Message& command, Client& client, const Server& server)
{
	
}
