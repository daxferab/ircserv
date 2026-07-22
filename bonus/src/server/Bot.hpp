#ifndef BOT_HPP
# define BOT_HPP

#include <string>

class Server;
class Client;

class Bot
{
	private:
		std::string	_name;

	public:
		Bot();
		Bot(std::string name);
		~Bot();

		std::string	getName() const;

		bool		isTarget(const std::string targetName) const;
		void		handleCommand(const std::string message, Client& target, Server& server) const;
};

#endif