#include "Client.hpp"

//----------------------------------------------------------------- CONSTRUCTORS

Client::Client(int fd) : _fd(fd){}

Client::~Client() {}

//------------------------------------------------------------- MEMBER FUNCTIONS

int	Client::getfd() const { return(_fd); }
