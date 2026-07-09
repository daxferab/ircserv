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

/***************************** MEMBER FUNCTIONS *******************************/

