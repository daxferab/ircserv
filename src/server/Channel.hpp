#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>

#include <string>
class Channel
{
	private:
		const std::string	_name;
		std::string			_topic;
		std::string			_key;

		std::set<int>		_users;
		std::set<int>		_operators;
		
		bool				_inviteOnly;
		bool				_topicRestrict;
		int					_userLimit;

	public:
		Channel();
		Channel(const std::string& name, int clientFd);
		~Channel();

		const std::string&	getName() const;
		const std::string&	getTopic() const;
		const std::string&	getKey() const;
		bool				isInviteOnly() const;
		bool				isTopicRestricted() const;
		int					getUserLimit() const;
		int					getUserCount() const;

		void				setTopic(std::string topic);
		void				setInviteOnly(bool opt);
		void				setTopicRestricted(bool opt);
		void				setUserLimit(int num);

		void				addUser(int fd);
		bool				isMember(int fd) const;
		void				removeUser(int fd);
};

#endif
