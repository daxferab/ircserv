#include "Client.hpp"
#include <ostream>
#include <string>

//----------------------------------------------------------------- CONSTRUCTORS

Client::Client(int fd) : _fd(fd), _authenticated(false) {}

Client::~Client() {}

//------------------------------------------------------------- GETTERS /SETTERS
std::string Client::getNick() const
{
	return _nick.empty() ? "" : _nick;
}

int Client::getFd() const
{
	return _fd;
}

bool	Client::isAuthenticated() const
{
	return _authenticated;
}

void	Client::setAuthenticated(const bool value)
{
	_authenticated = value;
}

//------------------------------------------------------------- MEMBER FUNCTIONS
void	Client::appendBuffer(char* msg, int data)
{
	_buffer.append(msg, data);
}

bool		Client::hasFullLine() const
{
	return _buffer.find("\r\n") != std::string::npos;
}

std::string	Client::getLine()
{
	size_t		pos = _buffer.find("\r\n");
	std::string	line = _buffer.substr(0, pos);

	_buffer.erase(0, pos + 2);
	return (line);
}

//----------------------------------------------------------- OPERATOR OVERLOADS
std::ostream&	operator<<(std::ostream& os, const Client& c)
{
	std::string	nick = c.getNick();
	if (nick.empty())
		os << c.getFd();
	else
		os << nick;
	return os;
}
