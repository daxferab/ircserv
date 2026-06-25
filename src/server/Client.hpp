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

		std::string	getnick() const;
		int			getfd() const;
		
		void		appendBuffer(char* msg, int data);
		bool		hasFullLine() const;
		std::string	getLine();
};

#endif
