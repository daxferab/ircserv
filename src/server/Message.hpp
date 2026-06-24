#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
class Message
{
	private:
		std::string					_prefix;
		std::string					_command;
		std::vector<std::string>	_args;

	public:
		Message(char* message);
		~Message();
};

#endif
