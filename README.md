# np_assignment3

[Draft, principles are there]

In this assignment you are to implement a command line chat system, based on TCP. 
# a) TCP Chat Client

Implement a simple chat-client. Must be able to set IP:PORT and nickname at runtime, e.g. using command-line arguments. See protocol description below. There is a testing-server at 18.219.51.6:4711, 13.53.35.7:4711 and 13.53.76.30:4711. For user-input you should read non-blocking without echo. I.E. you should be able to print any messages coming from the server, while the user is typing their message.
# b) TCP Chat Server

Implement a chat-server which is compatible with the chat-client. It must be able to handle multiple users simultaneous. 
# Chat Protocol
_______________________________________________
Client 	Direction 	Server
Connect() 	-->
 	
	<-- 	Hello <VERSION>

NICK <nick> 	--> 	

	<-- 	OK/ERR <text>

MSG <text> 	--> 	

	<-- 	MSG nick <text>/ERROR <text>
_______________________________________________

Nicknames is limited to 12 characters (A-Za-z0-9\_) and messages is limited to 255 characters (any characters except control characters and newlines, utf-8 encoding). Each message is followed by newline (CR).

 

Hints; On the client, read command line arguments (REQUIREMENT).  Check all inputs and the the NICK syntax in particular (reqular_expressions). If anything is wrong, print error exit, do not contact the server. On server, do not let the client into the 'chat' until a valid NICK has been received (regular expression to test). If the client tries to change NICK once in the chat, allow it, IF the new NICK is valid and would not conflict with any existing one. 

Use netcat/nc to test both client and server. 

As the server echos back whatever you send it, if properly formatted, then you should not print its reply (unless an error), as your client already has that printed. 

 

Your submission should be in the form of a tar.gz file, that contains your solution. The solution should contain a git repo, source code and make file to build the solutions. I expect; make, make clean, make client and make server to work and build corresponding solutions (see previous assignments how to do this).  Your file should be named <akronym>-uppgift3.tar.gz, see previous assignments how to create this file. 

 

I'll expect to be able to start the client as; 

./cchat 13.53.35.7:4711 bobbo

This would connect the client to 13.53.35.7 port 4711 and use the nickname bobbo.

The server I expect start as follows;

./cserverd 5010

This would start a the server on port 5010 on any available IPv4 address.
