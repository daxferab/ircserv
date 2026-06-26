#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

typedef enum e_comnum
{
	PASS,
	NICK,
	USER,
	NONE,
} t_comnum;

class Message
{
	private:
		bool							_valid;

		std::string						_prefix;
		t_comnum						_command;
		std::vector<std::string>		_params;

		bool							_parse(std::string& line);

	public:
		Message(std::string line);
		~Message();

		const std::string&				getPrefix() const;
		t_comnum						getCommand() const;
		const std::vector<std::string>&	getParams() const;
		bool							isValid() const;
};

t_comnum								getCommand(std::string word);

#endif
