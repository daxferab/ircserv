#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client
{
	private:
		const int	_fd;

	public:
		Client(int fd);
		~Client();

		int	getfd() const;
};

#endif
