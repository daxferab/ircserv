#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <unistd.h>

#define IN 0
#define OUT 1

class Client
{
	private:
		const int	_fd;
		std::string	_inBuffer;
		std::string	_outBuffer;

		bool		_authenticated;
		bool		_registered;

		std::string	_nick;
		std::string	_username;
		std::string	_realName;

	public:
		Client(int fd);
		~Client();

		std::string	getNick() const;
		void		setNick(const std::string nick);
		void		setUser(const std::string user);
		void		setName(const std::string name);
		int			getFd() const;
		const std::string& getOutBuffer() const;
		bool		isAuthenticated() const;
		void		setAuthenticated(const bool value);
		bool		isRegistered() const;
			
		void		appendBuffer(const char* msg, int data, int type);
		bool		hasFullLine(int type) const;
		void		consumeLine(int type, size_t end);
		std::string	getLine();
		void	consumeOut(size_t count);
};

std::ostream&	operator<<(std::ostream& os, const Client& c);

#endif
