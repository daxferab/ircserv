#include "Message.hpp"

#include <sstream>
#include <string>

//----------------------------------------------------------------- CONSTRUCTORS
Message::Message(char* message)
{
	std::string			msg = message;
	std::stringstream	ss;

	ss << msg;
	_prefix = "";
	if (!ss.fail() && ss.str()[0] == ':')
	{
		_prefix = ss.str();
		ss << msg;
	}
	_command = ss.str();
	ss << msg;
	while (!ss.fail())
	{
		_args.push_back(ss.str());
		ss << msg;
	}
}

Message::~Message() {}
