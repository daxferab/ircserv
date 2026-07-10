#include "Channel.hpp"

//----------------------------------------------------------------- CONSTRUCTORS

Channel::Channel() {}

Channel::Channel(const std::string& name, int clientFd): _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestrict(false), _userLimit(-1)
{
	_users.insert(clientFd);
	_operators.insert(clientFd);
}

Channel::~Channel() {}

//------------------------------------------------------------- GETTERS /SETTERS

const std::string&	Channel::getName() const { return _name; }
const std::string&	Channel::getTopic() const { return _topic; }
const std::string&	Channel::getKey() const { return _key; }
const std::set<int>&	Channel::getUsersList() const { return _users; }
bool				Channel::isInviteOnly() const { return _inviteOnly; }
bool				Channel::isTopicRestricted() const { return _topicRestrict; }
int					Channel::getUserLimit() const { return _userLimit; }
int					Channel::getUserCount() const { return _users.size(); }

void				Channel::setTopic(std::string topic) { _topic = topic; }
void				Channel::setInviteOnly(bool opt) { _inviteOnly = opt; }
void				Channel::setTopicRestricted(bool opt) { _topicRestrict = opt; }
void				Channel::setUserLimit(int num) { _userLimit = num; }

//------------------------------------------------------------- MEMBER FUNCTIONS

bool				Channel::isFull() const { return _userLimit > 0 && _userLimit >= getUserCount(); }
bool				Channel::isKeyOk(std::string key) const { return key == _key; }
bool				Channel::isMember(int fd) const { return _users.find(fd) != _users.end(); }
void				Channel::addUser(int fd) { _users.insert(fd); }
void				Channel::removeUser(int fd)
{
	_users.erase(fd);
	_operators.erase(fd);
}

bool	Channel::setOperator(int clientFd)
{
	return (_operators.insert(clientFd).second);
}

bool	Channel::unsetOperator(int clientFd)
{
	if (_operators.find(clientFd) != _operators.end())
	{
		_operators.erase(_operators.find(clientFd));
		return (true);
	}	
	return (false);
}
