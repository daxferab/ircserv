#include "AReply.hpp"

#include <sstream>
#include <string>

std::string	AReply::getReply(int type, int n, const std::string& clientName, const std::string& servname)
{
	if (type == ERR)
		return _err(n, clientName, servname);
	else if (type == RPL)
		return _rpl(n, clientName, servname);
	return "";
}

std::string	AReply::_err(int n, const std::string& clientName, const std::string& servname)
{
	std::stringstream	reply;
	std::string			cliName =	clientName.empty() ? "*" : clientName;

	reply << ":" << servname;
	switch (n)
	{
		case 401:
			reply << " 401 " <<  cliName << " nick " << ": No such nick/channel";
			break;
		case 403:
			reply << " 403 " <<  cliName << " channel " << ": No such channel";
			break;
		case 404:
			break;
		case 411:
			break;
		case 412:
			break;
		case 431:
			break;
		case 432:
			break;
		case 433:
			break;
		case 436:
			break;
		case 441:
			break;
		case 442:
			break;
		case 443:
			break;
		case 461:
			break;
		case 462:
			reply << " 462 " <<  cliName << " :You may not reregister";
			break;
		case 464:
			break;
		case 471:
			break;
		case 473:
			break;
		case 474:
			break;
		case 475:
			break;
		case 476:
			break;
		case 482:
			break;
		case 501:
			break;
		case 502:
			break;
	}

	return reply.str();
}

std::string	AReply::_rpl(int n, const std::string& clientName, const std::string& servname)
{
	std::stringstream	reply;
	std::string			cliName =	clientName.empty() ? "*" : clientName;

	reply << ":" << servname;
	switch (n)
	{
		case 001:
			reply << " 001 " << cliName << " :Welcome to the " << servname << " Network, " << clientName;
			break;
	}
	return reply.str();
}

/*
	All of them have a prefix with the server ¿name? ¿IP?
ERR_NOSUCHNICK (401)		client, non-existing nick
ERR_NOSUCHCHANNEL (403)		client, channel name
ERR_CANNOTSENDTOCHAN (404)	client, channel name
ERR_NORECIPIENT (411)		client, commandname(i think its always privmsg)
ERR_NOTEXTTOSEND (412)		client
ERR_NONICKNAMEGIVEN (431)	client
ERR_ERRONEUSNICKNAME (432)	client, unavailable nick
ERR_NICKNAMEINUSE (433)		client, unavailable nick
ERR_USERNOTINCHANNEL (441)	client, non-existing nick, channel
ERR_NOTONCHANNEL (442)		client, channel
ERR_USERONCHANNEL (443)		client, nick thats already in, channel
ERR_NEEDMOREPARAMS (461)	client, command
ERR_ALREADYREGISTERED (462)	client
ERR_PASSWDMISMATCH (464)	client
ERR_CHANNELISFULL (471)		client, channel
ERR_INVITEONLYCHAN (473)	client, channel
ERR_BADCHANNELKEY (475)		client, channel
ERR_BADCHANMASK (476)		client, channel
ERR_CHANOPRIVSNEEDED (482)	client, channel
ERR_UMODEUNKNOWNFLAG (501)	client
ERR_USERSDONTMATCH (502)	client

RPL_WELCOME (001)
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
