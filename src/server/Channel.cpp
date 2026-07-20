#include "Channel.hpp"

#include <cstdlib>
#include <limits>
#include <sstream>

//----------------------------------------------------------------- CONSTRUCTORS

Channel::Channel(const std::string& name, int clientFd): _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestrict(true), _userLimit(-1)
{
	_users.insert(clientFd);
	_operators.insert(clientFd);
}

Channel::~Channel() {}

//------------------------------------------------------------ GETTERS / SETTERS
//---------------------------------- GETTERS

const std::string&		Channel::getName() const { return _name; }
const std::string&		Channel::getTopic() const { return _topic; }
const std::string&		Channel::getKey() const { return _key; }
const std::set<int>&	Channel::getUsers() const { return _users; }
const std::set<int>&	Channel::getUsersList() const { return _users; }
bool					Channel::isInvited(int clientFd) const { return _invitedUsers.find(clientFd) != _invitedUsers.end(); }
bool					Channel::isOperator(int fd) const { return _operators.find(fd) != _operators.end(); }
bool					Channel::isInviteOnly() const { return _inviteOnly; }
bool					Channel::isTopicRestricted() const { return _topicRestrict; }
int						Channel::getUserLimit() const { return _userLimit; }
int						Channel::getUserCount() const { return _users.size(); }

//---------------------------------- SETTERS

void					Channel::setTopic(const std::string topic) { _topic = topic; }
bool					Channel::setKey(const std::string key) { if (_key == key) return false; _key = key; return true; }
void					Channel::setInvitedUser(int clientFd) { _invitedUsers.insert(clientFd); }
void					Channel::unsetInvitedUser(int clientFd) { _invitedUsers.erase(clientFd); }
bool					Channel::setOperator(int clientFd) { return _users.find(clientFd) != _users.end() && _operators.insert(clientFd).second; }
bool					Channel::unsetOperator(int clientFd)
{
	if (_operators.find(clientFd) != _operators.end())
	{
		_operators.erase(_operators.find(clientFd));
		return true;
	}
	return false;
}

bool					Channel::setInviteOnly(bool opt) { if (_inviteOnly == opt) return false; _inviteOnly = opt; return true; }
bool					Channel::setTopicRestricted(bool opt) { if (_topicRestrict == opt) return false; _topicRestrict = opt; return true; }
bool					Channel::setUserLimit(int num) { if (_userLimit == num) return false; _userLimit = num; return true; }
int						Channel::setUserLimit(const std::string limit)
{
	long	l;
	if (limit.empty())
		return setUserLimit(-1) ? 0 : -1;
	l = atol(limit.c_str());
	if (l > std::numeric_limits<int>::max())
		return setUserLimit(std::numeric_limits<int>::max()) ? std::numeric_limits<int>::max() : -1;
	if (l > 0)
		return setUserLimit(l) ? l : -1;
	if (l < 0)
		return setUserLimit(-1) ? 0 : -1;
	if (limit.size() >= 1 && limit[0] == '0')
		return setUserLimit(-1) ? 0 : -1;
	if (limit.size() >= 2 && limit[1] == '0' && (limit[0] == '+' || limit[0] == '-'))
		return setUserLimit(-1) ? 0 : -1;
	return -1;
}

//------------------------------------------------------------- MEMBER FUNCTIONS

bool					Channel::isFull() const { return _userLimit > 0 && _userLimit >= getUserCount(); }
bool					Channel::isKeyOk(std::string key) const { return key == _key || _key.empty(); }
void					Channel::addUser(int fd) {
	_users.insert(fd);
	unsetInvitedUser(fd);
}
bool					Channel::isMember(int fd) const { return _users.find(fd) != _users.end(); }
void					Channel::removeUser(int fd)
{
	_users.erase(fd);
	_operators.erase(fd);
}

bool 					Channel::changeOperator(int clientFd, bool set)
{
	return set ? setOperator(clientFd) : unsetOperator(clientFd);
}

std::string				Channel::getModes() const
{
	std::string			modes = "+";
	std::string			params = "";
	std::stringstream	ss;

	if (isInviteOnly())
		modes.append("i");
	if (!getKey().empty())
	{
		modes.append("k");
		params = params + " " + getKey();
	}
	if (getUserLimit() > 0)
	{
		modes.append("l");
		ss << getUserLimit();
		params = params + " " + ss.str();
	}
	if (isTopicRestricted())
		modes.append("t");
	return modes + params;
}
