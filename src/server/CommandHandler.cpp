#include "CommandHandler.hpp"
#include "Message.hpp"
#include <cstring>

//------------------------------------------------------------- MEMBER FUNCTIONS
void	CommandHandler::execCommand(Message& command, Client& client, Server& server)
{
	if (command.getCommand() != PASS || !client.isAuthenticated())
		{}// ERR_NOTREGISTERED 451
	switch (command.getCommand())
	{
		case PASS:
			_pass(command, client, server);
			break;
		case NICK:
			// _nick(command, client);
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

void	CommandHandler::_pass(Message& command, Client& client, Server& server)
{
	if (command.getParams().empty())
		server.authClient(client, "");
	server.authClient(client, command.getParams()[0]);
}
