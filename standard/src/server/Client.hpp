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

		std::string	_nick;
		std::string	_username;
		std::string	_realName;

		bool		_authenticated;
		bool		_registered;

	public:
		Client(int fd);
		~Client();

		//----------------------------------- GETTERS / SETTERS

		int					getFd() const;
		const std::string&	getOutBuffer() const;
		std::string			getNick() const;
		const std::string&	getUser() const;
		bool				isAuthenticated() const;
		bool				isRegistered() const;

		void				setNick(const std::string nick);
		void				setUser(const std::string user);
		void				setName(const std::string name);
		void				setAuthenticated(const bool value);
		void				setRegistered();

		//------------------------------------ MEMBER FUNCTIONS

		void				appendBuffer(const char* msg, int data, int type);
		bool				hasFullLine(int type) const;
		std::string			getLine();
		void				consumeLine(int type, size_t end);
		void				consumeOut(size_t count);
};

std::ostream&	operator<<(std::ostream& os, const Client& c);

#endif
