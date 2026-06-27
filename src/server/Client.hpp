#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		const int	_fd;
		std::string	_buffer;

		bool		_authenticated;
		bool		_operator;
		std::string	_nick;
		std::string	_username;
		std::string	_realName;

	public:
		Client(int fd);
		~Client();

		std::string	getNick() const;
		int			getFd() const;
		bool		isAuthenticated() const;
		void		setAuthenticated(const bool value);
		
		void		appendBuffer(char* msg, int data);
		bool		hasFullLine() const;
		std::string	getLine();
};

std::ostream&	operator<<(std::ostream& os, const Client& c);

#endif
