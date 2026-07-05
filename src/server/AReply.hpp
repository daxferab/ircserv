#ifndef AREPLY_HPP
#define AREPLY_HPP

#include <string>

typedef struct s_rplContext
{
	std::string	client;
	std::string	server;
	std::string	nick;
	std::string	channel;
	std::string	command;
} t_rplContext;

class AReply
{
	public:
		static std::string	getReply(int n, const t_rplContext& context);
};

#endif
