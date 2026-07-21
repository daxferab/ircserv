#include "Client.hpp"

#include <features.h>
#include <ostream>
#include <string>

//----------------------------------------------------------------- CONSTRUCTORS

Client::Client(int fd) : _fd(fd), _authenticated(false), _registered(false) {}

Client::~Client() {}

//------------------------------------------------------------- GETTERS /SETTERS
//---------------------------------- GETTERS

int					Client::getFd() const { return _fd; }
const std::string&	Client::getOutBuffer() const { return _outBuffer; }
std::string			Client::getNick() const { return _nick.empty() ? "" : _nick; }
const std::string&	Client::getUser() const { return _username; }
bool				Client::isAuthenticated() const { return _authenticated; }
bool				Client::isRegistered() const { return _registered; }

//---------------------------------- SETTERS

void				Client::setNick(const std::string nick) { _nick = nick; }
void				Client::setUser(const std::string user) { _username = user; }
void				Client::setName(const std::string name) { _realName = name; }
void				Client::setAuthenticated(const bool value) { _authenticated = value; }
void				Client::setRegistered() { _registered = true; }

//------------------------------------------------------------- MEMBER FUNCTIONS

void				Client::appendBuffer(const char* msg, int data, int type)
{
	if (type == OUT)
		_outBuffer.append(msg, data);
	if (type == IN)
		_inBuffer.append(msg, data);
}

bool				Client::hasFullLine(int type) const
{
	if (type == OUT)
		return _outBuffer.find("\r\n") != std::string::npos;
	return _inBuffer.find("\r\n") != std::string::npos;
}

std::string			Client::getLine()
{
	size_t		pos = _inBuffer.find("\r\n");
	std::string	line = _inBuffer.substr(0, pos);

	_inBuffer.erase(0, pos + 2);
	return line;
}

void				Client::consumeLine(int type, size_t end)
{
	if (type == OUT)
		_outBuffer.erase(0, end);
	else
	{
		size_t end = _inBuffer.find("\r\n") + 2;
		_inBuffer.erase(0, end);
	}
}

void				Client::consumeOut(size_t count)
{
	if (count >= _outBuffer.size())
		_outBuffer.clear();
	else
		_outBuffer.erase(0, count);
}

//----------------------------------------------------------- OPERATOR OVERLOADS

std::ostream&		operator<<(std::ostream& os, const Client& c)
{
	std::string	nick = c.getNick();
	if (nick.empty())
		os << c.getFd();
	else
		os << nick;
	return os;
}
