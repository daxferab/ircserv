#include "CommandHandler.hpp"
#include "Message.hpp"
#include <cstring>
#include <iostream>

//------------------------------------------------------------- MEMBER FUNCTIONS
void	CommandHandler::execCommand(Message& command, Client& client)
{
	switch (command.getCommand())
	{
		case PASS:
			_pass(command, client);
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

void	CommandHandler::_pass(Message& command, Client& client)
{
	if (command.getParams().empty())//TODO && server has password
	{
		std::cout << "Client " << client.getFd() << " tries a non password\n";//TODO remove when the error return correctly
		return ;//TODO return ERR_NEEDMOREPARAMS 461
	}
	else if (client.getAuthenticated())
	{
		std::cout << "Client " << client.getFd() << " is already registered\n";//TODO remove when the error return correctly
		return ;//TODO return ERR_ALREADYREGISTERED 462
	}
/* 	else if (server has password && command.params[0] != server password)
	{
		std::cout << "Client " << client.getFd() << " can't register with password " << command.params[0] << std::endl;//TODO remove when the error return correctly
		return ;//TODO return ERR_PASSWDMISMATCH 464
	} */

	client.setAuthenticated(true);
	std::cout << "Client " << client.getFd() << " gets authenticated with password " << command.getParams()[0] << std::endl;
}
