#include "AReply.hpp"
#include "../utils/colors.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>

std::string	AReply::getReply(int n, const t_rplContext& context)
{
	std::stringstream	reply;
	std::string			cliName =	context.client.empty() ? "*" : context.client;

	reply << ":" << context.server;
	reply << " " << std::setfill('0') << std::setw(3) << n << " ";
	switch (n)
	{
	// Replies
		case 001:
			reply << cliName << " :Welcome to the " << context.server << " Network, " << cliName;
			break;
		case 002:
			reply << cliName << " :Your host is " << context.server << " running version 1.0";
			break;
	// Errors
		case 401:
			reply << cliName << " " << context.nick << " :No such nick/channel";
			break;
		case 403:
			reply << cliName << " " << context.channel << " :No such channel";
			break;
		case 431:
			reply << cliName << " :No nickname given";
			break;
		case 432:
			reply << cliName << " " << context.nick << " :Erroneus nickname";
			break;
		case 433:
			reply << cliName << " " << context.nick << " :Nickname is already in use";
			break;
		case 451:
			reply << cliName << " :You have not registered";
			break;
		case 461:
			reply << cliName << " " << context.command << " :Not enough parameters";
			break;
		case 462:
			reply << cliName << " :You may not reregister";
			break;
		case 464:
			reply << cliName << " :Password incorrect";
			break;
	}
	reply << "\r\n";
	std::cout << MAGENTA << reply.str() << RESET << std::endl;
	return reply.str();
}

/*
	All of them have a prefix with the server ¿name? ¿IP?
ERR_CANNOTSENDTOCHAN (404)	client, channel name
ERR_NORECIPIENT (411)		client, commandname(i think its always privmsg)
ERR_NOTEXTTOSEND (412)		client
ERR_USERNOTINCHANNEL (441)	client, non-existing nick, channel
ERR_NOTONCHANNEL (442)		client, channel
ERR_USERONCHANNEL (443)		client, nick thats already in, channel
ERR_NEEDMOREPARAMS (461)	client, command
ERR_PASSWDMISMATCH (464)	client
ERR_CHANNELISFULL (471)		client, channel
ERR_INVITEONLYCHAN (473)	client, channel
ERR_BADCHANNELKEY (475)		client, channel
ERR_BADCHANMASK (476)		client, channel
ERR_CHANOPRIVSNEEDED (482)	client, channel
ERR_UMODEUNKNOWNFLAG (501)	client
ERR_USERSDONTMATCH (502)	client

RPL_UMODEIS (221)
RPL_AWAY (301)
RPL_CHANNELMODEIS (324)
RPL_CREATIONTIME (329)
RPL_NOTOPIC (331) 
RPL_TOPIC (332)
RPL_TOPICWHOTIME (333)
RPL_INVITING (341)
RPL_NAMREPLY (353)
RPL_ENDOFNAMES (366)
RPL_YOUREOPER (381)

ERROR message (reply to QUIT)
*/
