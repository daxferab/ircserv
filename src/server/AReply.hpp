#ifndef AREPLY_HPP
#define AREPLY_HPP

#include <string>

#define	ERR 0
#define	RPL 1

class AReply
{
	private:
		static std::string	_err(int n, const std::string& clientName, const std::string& servname);
		static std::string	_rpl(int n, const std::string& client, const std::string& servname);

	public:
		static std::string	getReply(int type, int n, const std::string& clientName, const std::string& servname);
};

#endif
