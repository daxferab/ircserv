#include "CommandHandler.hpp"
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>

//------------------------------------------------------------- MEMBER FUNCTIONS
void	CommandHandler::execCommand(std::string line, Client& client)
{
	t_command command = _parseCommand(line);

	switch (command.command)
	{
		case PASS:
			// _pass(command, client);
			break;
		case NICK:
			// _pass(command, client);
			break;
		case USER:
			// _pass(command, client);
			break;
		case NONE:
			;// handle invalid command
	}
	(void)client;
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
	command.command = getCommand(word); // normalize to uppercase

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

t_comnum	getCommand(std::string word)
{
	if (word == "PASS") return PASS;
	if (word == "NICK") return NICK;
	if (word == "USER") return USER;
	return NONE;
}
