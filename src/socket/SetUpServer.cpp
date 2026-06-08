#include "SetUpServer.hpp"

#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>

void	SetUpServer::listen(char* port)
{
	struct addrinfo	filter;
	struct addrinfo	*info;

	std::memset(&filter, 0, sizeof(filter));	// remove garbage data
	filter.ai_family = AF_UNSPEC;				// Allow IPv4 or IPv6
	filter.ai_socktype = SOCK_STREAM;			// TCP

	if (int netErr = getaddrinfo("localhost", port, &filter, &info) != 0)
		throw std::runtime_error(std::string("Network resolution failed: ") + gai_strerror(netErr));

	int	listener = socket(info->ai_family, info->ai_socktype, info->ai_protocol); // Net flag??
	if (listener < 0)
		throw std::runtime_error("Unable to open socket");

	if (!bind(listener, info->ai_addr, info->ai_addrlen))
		throw std::runtime_error("Unable to bind socket");
}
