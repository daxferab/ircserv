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
		
		bool				_inviteonly;
		bool				_topicRestrict;
		int					_userLimit;

	public:
		Channel();
		Channel(const std::string& name, int clientFd);
		~Channel();

		const std::string&	getName() const;

};

#endif
