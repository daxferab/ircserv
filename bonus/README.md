*This project has been created as part of the 42 curriculum by darmarti, daxferna, pdel-olm.*
# 📢 IRC 📢
## Description
This project's goal is to make an IRC (Internet Relay Chat) Server.\
IRC is a communication protocol that allows messaging between clients through a server.\
The server will have a socket that waits for client connections, binded to localhost and the port given.

When a new connection arrives, it will be added to the server and will be able to perform the allowed commands (register, send messages, join or create channels...).\
There can be normal and operator users (operators are users that create a channel/are given privileges by another channel operator). Operators are allowed to use certain commands like KICK, INVITE, TOPIC or MODE within their channel/s.

## Instructions
Compile the code using `make`.\
To execute the code, the user must specify a port and password.
```
./ircserv <port> <pass>
```
Any IRC client can be used to connect to the server, however, we used Hexchat to test it out.\
To connect, a valid nickname and password should be set.\
Once connected, the following commands can be used:
- NICK: allows nickname to be changed (if valid and not in use)
- JOIN: join a channel if it exists, otherwise, create it and become operator
- send/recieve messages (public or private)
- PART: to leave a channel
- QUIT: to leave the server

If user is operator:
- KICK: eject a client from a channel
- INVITE: invite a client to a channel
- TOPIC: change/view channel topic
- MODE: change channel modes -> +-{i t k o l}

## Resources
[IRC Wikipedia](https://es.wikipedia.org/wiki/Internet_Relay_Chat)\
[Modern IRC Manual](https://modern.ircdocs.horse/)\
[Sockets API linux](https://www.youtube.com/watch?v=XXfdzwEsxFk)\
[Sockets guide](https://beej.us/guide/bgnet/html/)\
[IRC Project from another university](http://chi.cs.uchicago.edu/chirc/index.html) (only the intro)
