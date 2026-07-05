#include "AReply.hpp"

#include <sstream>
#include <string>

std::string	AReply::getReply(int n, const std::string& clientName, const std::string& servname)
{
	std::stringstream	reply;
	std::string			cliName =	clientName.empty() ? "*" : clientName;

	reply << ":" << servname;
	switch (n)
	{
	// Replies
		case 001:
			reply << " 001 " << cliName << " :Welcome to the " << servname << " Network, " << cliName;
			break;
		case 002:
			reply << " 002 " << cliName << " :Your host is " << servname << " running version 1.0";
			break;
	// Errors
		case 401:
			reply << " 401 " <<  cliName << " " << "nick" << " :No such nick/channel"; //TODO: nick
			break;
		case 403:
			reply << " 403 " <<  cliName << " " << "channel" << " :No such channel";//TODO: channel
			break;
		case 431:
			reply << " 431 " <<  cliName  << " :No nickname given";
			break;
		case 432:
			reply << " 432 " <<  cliName  << " " << "nick" << " :Erroneus nickname"; //TODO: nick
			break;
		case 433:
			reply << " 433 " <<  cliName  << " " << "nick" << " :Nickname is already in use"; //TODO: nick
			break;
		case 451:
			reply << " 451 " <<  cliName << " :You have not registered";
			break;
		case 461:
			reply << " 461 " <<  cliName << " " << "command" << " :You may not reregister";
			break;
		case 462:
			reply << " 462 " <<  cliName << " :You may not reregister";
			break;
		case 464:
			reply << " 464 " <<  cliName << " :Password incorrect";
			break;
	}
	reply << "\r\n";
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
