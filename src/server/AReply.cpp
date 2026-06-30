#include "AReply.hpp"

#include <sstream>

std::string	AReply::getReply(int type, int n, const Client& client, const Server& server, const Channel& channel)
{
	if (type == ERR)
		return _err(n, client, server, channel);
	else if (type == RPL)
		return _rpl(n, client, server, channel);
	return NULL;
}

std::string	AReply::_err(int n, const Client& client, const Server& server, const Channel& channel)
{
	std::stringstream	reply;
	std::string			nick =	client.getNick().empty() ? "*" : client.getNick();

	switch (n)
	{
		case 401:
			// reply << ":thiscord.irc 401 " <<  nick << ;
			break;
		case 402:
			break;
		case 403:
			break;
		case 404:
			break;
		case 405:
			break;
		case 407:
			break;
		case 411:
			break;
		case 412:
			break;
		case 413:
			break;
		case 414:
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

	return reply;
}

std::string	AReply::_rpl(int n, const Client& client, const Server& server, const Channel& channel)
{
	
}

/*
ERR_NOSUCHNICK (401)
ERR_NOSUCHSERVER (402)
ERR_NOSUCHCHANNEL (403)
ERR_CANNOTSENDTOCHAN (404)
ERR_TOOMANYCHANNELS (405)
ERR_TOOMANYTARGETS (407)
ERR_NORECIPIENT (411)
ERR_NOTEXTTOSEND (412)
ERR_NOTOPLEVEL (413)
ERR_WILDTOPLEVEL (414)
ERR_NONICKNAMEGIVEN (431)
ERR_ERRONEUSNICKNAME (432)
ERR_NICKNAMEINUSE (433)
ERR_NICKCOLLISION (436)
ERR_USERNOTINCHANNEL (441)
ERR_NOTONCHANNEL (442)
ERR_USERONCHANNEL (443) 
ERR_NEEDMOREPARAMS (461)
ERR_ALREADYREGISTERED (462)
ERR_PASSWDMISMATCH (464)
ERR_CHANNELISFULL (471)
ERR_INVITEONLYCHAN (473)
ERR_BANNEDFROMCHAN (474)
ERR_BADCHANNELKEY (475)
ERR_BADCHANMASK (476)
ERR_CHANOPRIVSNEEDED (482)
ERR_UMODEUNKNOWNFLAG (501)
ERR_USERSDONTMATCH (502)

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
