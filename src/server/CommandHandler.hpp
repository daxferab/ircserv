#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include "Client.hpp"

class CommandHandler
{
	private:
		// Atributes

	public:
		static void	_execCommand(std::string line, Client& client);
};

#endif
