#include "Message.hpp"

#include <sstream>
#include <iostream>
//----------------------------------------------------------------- CONSTRUCTORS

Message::Message(std::string line)
{
	_valid = _parse(line);
}

Message::~Message() {}

//------------------------------------------------------------- PUBLIC FUNCTIONS
//---------------------------- GETTERS/SETTERS

const std::string&	Message::getPrefix() const { return _prefix; }

t_comnum	Message::getCommand() const { return _command; }

const std::vector<std::string>&	Message::getParams() const { return _params; }

bool	Message::isValid() const { return _valid; }

//------------------------------------------------------------ PRIVATE FUNCTIONS
bool	Message::_parse(std::string& line)
{
	std::stringstream	ss(line);
	std::string			word;

	_prefix = "";
	if (!(ss >> word)) return false;

	if (word[0] == ':')
	{
		_prefix = word;
		if (!(ss >> word)) return false;
	}
	_command = ::getCommand(word); // normalize to uppercase

	while (ss >> word)
	{
		if (word[0] == ':')
		{
			_params.push_back(word.substr(1));
			std::getline(ss, word);
			if (!word.empty())
				_params.back() += word;
			break;
		}
		_params.push_back(word);
	}
	return true;
}

//------------------------------------------------------- OUT OF SCOPE FUNCTIONS
t_comnum	getCommand(std::string word)
{
	if (word == "PASS") return PASS;
	if (word == "NICK") return NICK;
	if (word == "USER") return USER;
	return NONE;
}