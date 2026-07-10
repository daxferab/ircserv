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
} t_rplContext;

class AReply
{
	private:
		AReply();

	public:
		static std::string	getReply(int n, const Server& server, const Client& client, const t_rplContext& context);
};

#endif
