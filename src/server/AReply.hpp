#ifndef AREPLY_HPP
#define AREPLY_HPP

#include <string>

class AReply
{
	public:
		static std::string	getReply(int n, const std::string& clientName, const std::string& servname);
};

#endif
