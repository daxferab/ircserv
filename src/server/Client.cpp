#include "Client.hpp"
#include <string>

//----------------------------------------------------------------- CONSTRUCTORS

Client::Client(int fd) : _fd(fd){}

Client::~Client() {}

//------------------------------------------------------------- GETTERS /SETTERS
std::string Client::getnick() const
{
	return _nick.empty() ? "" : _nick; 
}

int Client::getfd() const
{
	return _fd;
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

