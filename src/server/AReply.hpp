#ifndef AREPLY_HPP
#define AREPLY_HPP

#include <string>

class Server;
class Client;

typedef struct s_rplContext
{
	std::string	nick;
	std::string	channel;
	std::string	command;
	std::string message;
	std::string target;
} t_rplContext;

class AReply
{
	private:
		AReply();

	public:
		static std::string	getNReply(int n, const Server& server, const Client& client, const t_rplContext& context);
		static std::string	getReply(int command, const Client& client, const t_rplContext& context);
};

#endif
