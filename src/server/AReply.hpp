#ifndef AREPLY_HPP
#define AREPLY_HPP

#include <string>
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#define	ERR 0
#define	RPL 1

class AReply
{
	private:
		static std::string	_err(int n, const Client& client, const Server& server, const Channel& channel);
		static std::string	_rpl(int n, const Client& client, const Server& server, const Channel& channel);

	public:
		static std::string	getReply(int type, int n, const Client& client, const Server& server, const Channel& channel);
};

#endif
