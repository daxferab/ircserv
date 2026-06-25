#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <vector>
#include "Client.hpp"

typedef enum e_comnum
{
	PASS,
	NICK,
	USER,
	NONE,
} t_comnum;

typedef struct s_command
{
	std::string					prefix;
	t_comnum					command;
	std::vector<std::string>	params;
} t_command;

class CommandHandler
{
	private:
		static t_command	_parseCommand(const std::string& line);
	public:
		static void			execCommand(std::string line, Client& client);
};

t_comnum	getCommand(std::string word);

#endif
