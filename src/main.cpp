#include "irc.h"

static const char *g_error[] =
{
	"Usage: ./ircserv <port> <password>"
};

int	error(int i)
{
	std::cout << g_error[i] << std::endl;

	return 1;
}

int	main(int argc, char *argv[])
{
	if (argc != 3)
		return(error(0));
	(void) argv;
}
