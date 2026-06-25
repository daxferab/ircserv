#include "CommandHandler.hpp"
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>

//------------------------------------------------------------- MEMBER FUNCTIONS
void	CommandHandler::execCommand(std::string line, Client& client)
{
	t_command command = _parseCommand(line);

	// switch (command.command)
	// {
		
	// }
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

t_command	CommandHandler::_parseCommand(const std::string& line)
{
	t_command			command;
	std::stringstream	ss(line);
	std::string			word;

	command.prefix = "";

	if (!(ss >> word)) return command;

	if (word[0] == ':')
	{
		command.prefix = word;
		if (!(ss >> word))
			return command;
	}
	command.command = word;

	while (ss >> word)
	{
		if (word[0] == ':')
		{
			command.params.push_back(word.substr(1));
			std::getline(ss, word);
			if (!word.empty())
				command.params.back() += word;
			break;
		}
		command.params.push_back(word);
	}
	return command;
}

void	CommandHandler::_pass(t_command& command, Client& client)
{
	if (command.params.empty())//TODO && server has password
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
	std::cout << "Client " << client.getFd() << " gets authenticated with password " << command.params[0] << std::endl;
}
