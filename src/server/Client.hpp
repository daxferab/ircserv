#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		const std::string	_nick;
		const int			_fd;

	public:
		Client(int fd);
		~Client();

		int	getfd() const;
};

#endif
