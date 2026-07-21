#ifndef AREPLY_HPP
#define AREPLY_HPP

#include <string>

class Server;
class Client;

typedef struct s_rplContext
{
	std::string	target;
	std::string	channel;
	std::string	command;
	std::string message;
} t_rplContext;

typedef enum e_errno
{
	RPL_WELCOME = 001,
	RPL_YOURHOST = 002,
	RPL_ISUPPORT = 005,
	RPL_CHANNELMODEIS = 324,
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	RPL_INVITING = 341,
	RPL_NAMREPLY = 353,
	RPL_ENDOFNAMES = 366,
	ERR_NOSUCHNICK = 401,
	ERR_NOSUCHCHANNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE = 433,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_USERONCHANNEL = 443,
	ERR_NOTREGISTERED = 451,
	ERR_NEEDMOREPARAMS = 461,
	ERR_ALREADYREGISTERED = 462,
	ERR_PASSWDMISMATCH = 464,
	ERR_CHANNELISFULL = 471,
	ERR_UNKNOWNMODE = 472,
	ERR_INVITEONLYCHAN = 473,
	ERR_BADCHANNELKEY = 475,
	ERR_CHANOPRIVSNEEDED = 482,
} t_errno;

class AReply
{
	private:
		AReply();

	public:
		static std::string	getNReply(int n, const Server& server, const Client& client, const t_rplContext& context);
		static std::string	getReply(int command, const Client& client, const t_rplContext& context);
};

#endif
