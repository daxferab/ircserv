#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Client.hpp"
#include "Message.hpp"

class CommandHandler
{
	private:
		static void			_pass(Message& command, Client& client);

	public:
		static void			execCommand(Message& command, Client& client);
};



#endif
