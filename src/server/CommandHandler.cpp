#include "CommandHandler.hpp"
#include <string>
#include <iostream>
#include <sstream>

//------------------------------------------------------------- MEMBER FUNCTIONS
void	CommandHandler::execCommand(std::string line, Client& client)
{
	t_command command = _parseCommand(line);
	std::cout << "line: " << line << std::endl;
	
	(void)command;
	(void)client;
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

t_command	CommandHandler::_parseCommand(const std::string& line)
{
	t_command			command;
	std::stringstream	ss;

	ss << line;
	command.prefix = "";
	if (!ss.fail() && ss.str()[0] == ':')
	{
		command.prefix = ss.str();
		ss << line;
	}
	command.command = ss.str();
	ss << line;
	// while (!ss.fail())
	// {
	// 	command.params.push_back(ss.str());
	// 	ss << line;
	// }
	return command;
}
