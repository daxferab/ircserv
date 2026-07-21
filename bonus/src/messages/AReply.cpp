#include "AReply.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "colors.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

//------------------------------------------------------------- MEMBER FUNCTIONS

std::string	AReply::getNReply(int n, const Server& server, const Client& client, const t_rplContext& context)
{
	std::stringstream	reply;
	std::string			cliName = client.getNick().empty() ? "*" : client.getNick();
	std::string			serverName = server.getName();

	reply << ":" << serverName;
	reply << " " << std::setfill('0') << std::setw(3) << n << " " << cliName << " ";
	switch (n)
	{
	// Replies
		case RPL_WELCOME:
			reply << ":Welcome to the " << serverName << " Network, " << cliName;
			break;
		case RPL_YOURHOST:
			reply << ":Your host is " << serverName << " running version 1.0";
			break;
		case RPL_ISUPPORT:
			reply << "CHANMODES=o,,kl,it :are supported by this server";
			break;
		case RPL_CHANNELMODEIS:
			reply << context.channel << " " << context.message;
			break;
		case RPL_NOTOPIC:
			reply << context.channel << " :No topic is set";
			break;
		case RPL_TOPIC:
			reply << context.channel << " :" << server.getChannelTopic(context.channel);
			break;
		case RPL_INVITING:
			reply << context.target << " " << context.channel;
			break;
		case RPL_NAMREPLY:
			reply << "= " << context.channel << " :" << server.getChannelMembers(context.channel);
			break;
		case RPL_ENDOFNAMES:
			reply << context.channel << " :End of /NAMES list";
			break;
	// Errors
		case ERR_NOSUCHNICK:
			reply << context.target << " :No such nick/channel";
			break;
		case ERR_NOSUCHCHANNEL:
			reply << context.channel << " :No such channel";
			break;
		case ERR_CANNOTSENDTOCHAN:
			reply << context.target << " :Cannot send to channel";
			break;
		case ERR_NORECIPIENT:
			reply << context.target << " :No recipient given (" << context.command << ")";
			break;
		case ERR_NOTEXTTOSEND:
			reply << context.target << " :No text to send";
			break;
		case ERR_NONICKNAMEGIVEN:
			reply << ":No nickname given";
			break;
		case ERR_ERRONEUSNICKNAME:
			reply << context.target << " :Erroneus nickname";
			break;
		case ERR_NICKNAMEINUSE:
			reply << context.target << " :Nickname is already in use";
			break;
		case ERR_USERNOTINCHANNEL:
			reply << context.target << " " << context.channel << " :They aren't on that channel";
			break;
		case ERR_NOTONCHANNEL:
			reply << context.channel << " :You're not on that channel";
			break;
		case ERR_USERONCHANNEL:
			reply << context.target << " " << context.channel << " :is already on channel";
			break;
		case ERR_NOTREGISTERED:
			reply << ":You have not registered";
			break;
		case ERR_NEEDMOREPARAMS:
			reply << context.command << " :Not enough parameters";
			break;
		case ERR_ALREADYREGISTERED:
			reply << ":You may not reregister";
			break;
		case ERR_PASSWDMISMATCH:
			reply << ":Password incorrect";
			break;
		case ERR_CHANNELISFULL:
			reply << context.channel << " :Cannot join channel (+l)";
			break;
		case ERR_UNKNOWNMODE:
			reply << context.message << " :is unknown mode char to me";
			break;
		case ERR_INVITEONLYCHAN:
			reply << context.channel << " :Cannot join channel (+i)";
			break;
		case ERR_BADCHANNELKEY:
			reply << context.channel << " :Cannot join channel (+k)";
			break;
		case ERR_CHANOPRIVSNEEDED:
			reply << context.channel << " :You're not channel operator";
			break;
	}
	reply << "\r\n";
	std::cout << MAGENTA << reply.str() << RESET;
	return reply.str();
}

std::string	AReply::getReply(int command, const Client& client, const t_rplContext& context)
{
	std::stringstream	reply;

	switch (command)
	{
		case INVITE:
			reply << ":" << client.getNick() << " INVITE " << context.target << " " << context.channel;
			break;
		case JOIN:
			reply << ":" << client.getNick() << " JOIN " << context.channel;
			break;
		case KICK:
			reply << ":" << client.getNick() << " KICK " << context.channel << " " << context.target << " :" << context.message;
			break;
		case MODE:
			reply << ":" << client.getNick() << " MODE " << context.channel << " " << context.message;
			break;
		case NICK:
			reply << ":" << context.target << " NICK " << client.getNick();
			break;
		case PART:
			reply << ":" << client.getNick() << " PART " << context.channel << " :" << context.message;
			break;
		case PRIVMSG:
			reply << ":" << client.getNick() << " PRIVMSG " << context.target << " :" << context.message;
			break;
		case QUIT:
			reply << ":" << client.getNick() << " QUIT :Quit: " << context.message;
			break;
		case TOPIC:
			reply << ":" << client.getNick() << " TOPIC " << context.channel << " :" << context.message;
			break;
		case ERROR:
			reply << "ERROR :" << context.message;
			break;
	}
	reply << "\r\n";
	std::cout << MAGENTA << reply.str() << RESET;
	return reply.str();
}

/*
	All of them have a prefix with the server ¿name? ¿IP?
ERR_BADCHANMASK (476)		client, channel
ERR_USERSDONTMATCH (502)	client

RPL_AWAY (301)
RPL_CREATIONTIME (329)
RPL_TOPICWHOTIME (333)
RPL_YOUREOPER (381)
*/
