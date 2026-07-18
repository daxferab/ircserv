#include "Message.hpp"
#include "colors.h"

#include <ios>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

//----------------------------------------------------------------- CONSTRUCTORS

Message::Message(std::string line) { _valid = _parse(line); }
Message::~Message() {}

//------------------------------------------------------------ PRIVATE FUNCTIONS

bool							Message::_parse(std::string& line)
{
	std::stringstream ss(line);
	std::string word;

	_prefix.clear();

	if (!(ss >> word))
		return false;

	std::cout << CYAN << line << RESET << std::endl;

	if (word[0] == ':')
		if (!(ss >> word))
			return false;

	_command = ::getCommand(word);
	_params = _parseParams(ss);
	return true;
}

std::vector<std::string>		Message::_parseParams(std::stringstream& ss)
{
	std::vector<std::string> params;
	std::string word;

	while (ss >> word)
	{
		if (!word.empty() && word[0] == ':')
		{
			word.erase(0, 1);
			std::string rest;
			std::getline(ss, rest);
			word += rest;

			params.push_back(word);
			break;
		}
		// PRIVMSG: everything after target is the message
		if (_command == PRIVMSG && params.size() == 1)
		{
			std::string rest;
			std::getline(ss, rest);
			word += rest;

			params.push_back(word);
			break;
		}
		params.push_back(word);
	}
	return params;
}

//-------------------------------------------------------------- GETTERS/SETTERS

bool							Message::isValid() const { return _valid; }
const std::string&				Message::getPrefix() const { return _prefix; }
t_comnum						Message::getCommand() const { return _command; }
const std::vector<std::string>&	Message::getParams() const { return _params; }

//------------------------------------------------------- OUT OF SCOPE FUNCTIONS

t_comnum	getCommand(std::string word)
{
	std::transform(word.begin(), word.end(), word.begin(), ::toupper);
	if (word == "PASS") return PASS;
	if (word == "NICK") return NICK;
	if (word == "USER") return USER;
	if (word == "JOIN") return JOIN;
	if (word == "PRIVMSG") return PRIVMSG;
	if (word == "TOPIC") return TOPIC;
	if (word == "MODE") return MODE;
	if (word == "INVITE") return INVITE;
	if (word == "PART") return PART;
	if (word == "KICK") return KICK;
	if (word == "QUIT") return QUIT;
	return NONE;
}

