#include "Channel.hpp"

/************************* ORTHODOX CANONICAL FORM ****************************/

Channel::Channel() {}

Channel::Channel(const std::string& name, Client& client): _name(name)
{
	_operators.insert(std::pair<int, Client&>(client.getFd(), client));
}

Channel::~Channel() {}


//------------------------------------------------------------- GETTERS /SETTERS

const std::string	&Channel::getName() const
{
	return(_name);
}

/***************************** MEMBER FUNCTIONS *******************************/

