#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

typedef enum e_comnum
{
	INVITE,
	JOIN,
	KICK,
	MODE,
	NICK,
	PART,
	PASS,
	PRIVMSG,
	QUIT,
	TOPIC,
	USER,
	ERROR,
	NONE,
} t_comnum;

class Message
{
	private:
		bool						_valid;

		std::string					_prefix;
		t_comnum					_command;
		std::vector<std::string>	_params;

		bool							_parse(std::string& line);
		std::vector<std::string>		_parseParams(std::stringstream& ss);

	public:
		Message(std::string line);
		~Message();

		//----------------------------------- GETTERS / SETTERS

		bool							isValid() const;
		const std::string&				getPrefix() const;
		t_comnum						getCommand() const;
		const std::vector<std::string>&	getParams() const;
};

t_comnum	getCommand(std::string word);

#endif
