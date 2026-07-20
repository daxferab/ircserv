#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include <sys/types.h>

class Channel
{
	private:
		const std::string	_name;
		std::string			_topic;
		std::string			_key;

		std::set<int>		_users;
		std::set<int>		_invitedUsers;
		std::set<int>		_operators;

		bool				_inviteOnly;
		bool				_topicRestrict;
		int					_userLimit;

	public:
		//---------------------------------------- CONSTRUCTORS

		Channel(const std::string& name, int clientFd);
		~Channel();

		//----------------------------------- GETTERS / SETTERS

		const std::string&		getName() const;
		const std::string&		getTopic() const;
		const std::string&		getKey() const;
		const std::set<int>&	getUsers() const;
		const std::set<int>&	getUsersList() const;
		bool					isInvited(int clientFd) const;
		bool					isOperator(int fd) const;
		bool					isInviteOnly() const;
		bool					isTopicRestricted() const;
		int						getUserLimit() const;
		int						getUserCount() const;

		void					setTopic(const std::string topic);
		bool					setKey(const std::string key);
		void					setInvitedUser(int clientFd);
		void					unsetInvitedUser(int clientFd);
		bool					setOperator(int clientFd);
		bool					unsetOperator(int clientFd);
		bool					setInviteOnly(bool opt);
		bool					setTopicRestricted(bool opt);
		bool					setUserLimit(int num);
		int						setUserLimit(const std::string limit);

		//------------------------------------ MEMBER FUNCTIONS

		bool					isFull() const;
		bool					isKeyOk(std::string key) const;
		void					addUser(int fd);
		bool					isMember(int fd) const;
		void					removeUser(int fd);

		bool					changeOperator(int clientFd, bool set);
		std::string				getModes() const;
};

#endif
