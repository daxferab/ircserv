#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <map>

#include <string>
class Channel
{
	private:
		const std::string		_name;
		std::string				_topic;
		std::string				_key;

		std::map<int, Client>	_users;
		std::map<int, Client>	_operators;
		
		bool					_inviteonly;
		bool					_topicRestrict;
		int						_userLimit;

	public:
		Channel();
		~Channel();
};

#endif
