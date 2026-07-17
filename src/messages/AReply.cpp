#include "AReply.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "colors.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>

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
		case 001:
			reply << ":Welcome to the " << serverName << " Network, " << cliName;
			break;
		case 002:
			reply << ":Your host is " << serverName << " running version 1.0";
			break;
		case 005:
			reply << "CHANMODES=o,,kl,it :are supported by this server";
			break;
		case 324:
			reply << context.channel << " " << context.message;
			break;
		case 331:
			reply << context.channel << " :No topic is set";
			break;
		case 332:
			reply << context.channel << " :" << server.getChannelTopic(context.channel);
			break;
		case 341:
			reply << context.target << " " << context.channel;
			break;
		case 353:
			reply << "= " << context.channel << " :" << server.getChannelMembers(context.channel);
			break;
		case 366:
			reply << context.channel << " :End of /NAMES list";
			break;
	// Errors
		case 401:
			reply << context.target << " :No such nick/channel";
			break;
		case 403:
			reply << context.channel << " :No such channel";
			break;
		case 404:
			reply << context.channel << " :Cannot send to channel";
			break;
		case 411:
			reply << context.target << " :No recipient given (" << context.command << ")";
			break;
		case 412:
			reply << context.target << " :No text to send";
			break;
		case 431:
			reply << ":No nickname given";
			break;
		case 432:
			reply << context.target << " :Erroneus nickname";
			break;
		case 433:
			reply << context.target << " :Nickname is already in use";
			break;
		case 441:
			reply << context.target << " " << context.channel << " :They aren't on that channel";
			break;
		case 442:
			reply << context.channel << " :You're not on that channel";
			break;
		case 443:
			reply << context.target << " " << context.channel << " :is already on channel";
			break;
		case 451:
			reply << ":You have not registered";
			break;
		case 461:
			reply << context.command << " :Not enough parameters";
			break;
		case 462:
			reply << ":You may not reregister";
			break;
		case 464:
			reply << ":Password incorrect";
			break;
		case 471:
			reply << context.channel << " :Cannot join channel (+l)";
			break;
		case 472:
			reply << context.message << " :is unknown mode char to me";
			break;
		case 473:
			reply << context.channel << " :Cannot join channel (+i)";
			break;
		case 475:
			reply << context.channel << " :Cannot join channel (+k)";
			break;
		case 482:
			reply << context.channel << " :You're not channel operator";
			break;
	}
	reply << "\r\n";
	std::cout << MAGENTA << reply.str() << RESET << std::endl;
	return reply.str();
}

std::string	AReply::getReply(int command, const Client& client, const t_rplContext& context)
{
	std::stringstream	reply;

	switch (command)
	{
		case ERROR:
			reply << "ERROR :" << context.message;
			break;
		case QUIT:
			reply << ":" << client.getNick() << " QUIT :Quit: " << context.message;
			break;
		case NICK:
			reply << ":" << context.target << " NICK " << client.getNick();
			break;
		case JOIN:
			reply << ":" << client.getNick() << " JOIN " << context.channel;
			break;
		case INVITE:
			reply << ":" << client.getNick() << " INVITE " << context.target << " " << context.channel;
			break;
		case TOPIC:
			reply << ":" << client.getNick() << " TOPIC " << context.channel << " :" << context.message;
			break;
		case KICK:
			reply << ":" << client.getNick() << " KICK " << context.channel << " " << context.target << " :" << context.message;
			break;
		case PART:
			reply << ":" << client.getNick() << " PART " << context.channel << " :" << context.message;
			break;
		case PRIVMSG:
			reply << ":" << client.getNick() << " PRIVMSG " << context.target << " :" << context.message;
			break;
		case MODE:
			reply << ":" << client.getNick() << " MODE " << context.channel << " " << context.message;
			break;
	}
	reply << "\r\n";
	std::cout << MAGENTA << reply.str() << RESET << std::endl;
	return reply.str();
}

/*
	All of them have a prefix with the server ¿name? ¿IP?
ERR_BADCHANMASK (476)		client, channel
ERR_UMODEUNKNOWNFLAG (501)	client
ERR_USERSDONTMATCH (502)	client

RPL_UMODEIS (221)
RPL_AWAY (301)
RPL_CREATIONTIME (329)
RPL_TOPICWHOTIME (333)
RPL_YOUREOPER (381)
*/
