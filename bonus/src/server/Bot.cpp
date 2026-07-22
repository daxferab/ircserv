#include "Bot.hpp"

#include "Server.hpp"

#include <iostream>

//----------------------------------------------------------------- CONSTRUCTORS

Bot::Bot() : _name("Bot") {}

Bot::Bot(std::string name) : _name(name) {}

Bot::~Bot() {}

//------------------------------------------------------------ GETTERS / SETTERS

std::string Bot::getName() const { return _name; }

//------------------------------------------------------------- MEMBER FUNCTIONS

bool Bot::isTarget(const std::string targetName) const { return _name == targetName; }

void Bot::handleCommand(const std::string message, Client& target, Server& server) const
{
	std::string firstWord = message.substr(0, message.find(" "));
	std::string	response;

	if (firstWord[0] != '!')
		return ;

	if (firstWord == "!Art")		response = "8====D";
	else if (firstWord == "!Hello")	response = "Hola =3";
	else if (firstWord == "!Bye")	response = "Bye =3";
	else							response = "Try a valid command";

	server.botResponse(response, target);
}
