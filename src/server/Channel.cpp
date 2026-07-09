#include "Channel.hpp"

/************************* ORTHODOX CANONICAL FORM ****************************/

Channel::Channel() {}

Channel::Channel(const std::string& name, int clientFd): _name(name)
{
	_operators.insert(clientFd);
}

Channel::~Channel() {}


//------------------------------------------------------------- GETTERS /SETTERS

const std::string	&Channel::getName() const
{
	return(_name);
}

const bool Channel::setOperator(int clientFd)
{
	return (_operators.insert(clientFd).second);
}

const bool Channel::unsetOperator(int clientFd)
{
	return (_operators.erase(_operators.find(clientFd)) != _operators.end());
}

/***************************** MEMBER FUNCTIONS *******************************/

