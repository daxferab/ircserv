#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <vector>
#include "Client.hpp"

typedef struct s_command
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
} t_command;

class CommandHandler
{
	private:
		static t_command	_parseCommand(const std::string& line);
	public:
		static void			execCommand(std::string line, Client& client);
};

#endif
