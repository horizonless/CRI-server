# CRI-server
Chatting Remotely over the Internet
This protocal is similar to IRC [RFC1459](http://www.networksorcery.com/enp/rfc/rfc1459.txt).

The server file needs to be complied first.
For local server test
First open three terminals. One for servers,the other two are for clients.
type "make" on server terminal.
```
gcc -o CRI.out CRI.c
```
By
```
./CRI.out --opt-pass=password
```
you can run the server and get the port number. 
On the "client" terminal,use the command "nc local host portnumber" to get the server.
You can have multi client chat with each other all funcionality is working fine on my computer.

The server also support IPV6 correctly with AF_INET6 and all the IPV6 things implemented.

The following command can be used to interact with the server:
- USER
- LIST
- JOIN
- PART
- OPERATOR
- KICK
- PRIVMSG
- QUIT


| Command | Description |
| --- | --- |
| **USER** nickname | The USER command must be the first command sent to the server immediately following a connection from a client. Otherwise, the server MUST disconnect. Users cannot change their nickname at a later time. User names should be at most 20 characters and should adhere to the following regular expression:[a-zA-Z][_0-9a-zA-Z] |
| **LIST** #channel | The LIST command should output the current count and list all of the channels currently available on the server. If a valid channel is supplied, LIST should output the current count and print the names of all users on channel #channel. |
|**JOIN** #channel|The JOIN command must add the current user to the named channel. If the named channel does not exist, it must be created. Channel names not starting with a # should not be created. Users currently on the channel should receive a message that USER has joined. Channel names should be at most 20 characters and should adhere to the following regular expression: #[a-zA-Z][_0-9a-zA-Z]|
|**PART** #channel|The PART command must remove the current user from the named channel. Users currently on the channel should receive a message that USER has left. PARTing from a channel the user is not a member of should produce an error message and not modify the user’s current channel list. With no arguments, this command should remove the current user from all channels.|
|**OPERATOR** password|The OPERATOR command must make the current user an operator of the server if and only if the given password matches the password supplied on the server’s command line. When starting the CRI server, an optional flag --opt-pass <password> may be supplied. If no such flag is passed then no users can be promoted to operator status. If the password is incorrect (or none was given on the command line) the server should print an error message but not disconnect the user. Passwords should be at most 20 characters and should adhere to the following regular expression: [a-zA-Z][_0-9a-zA-Z]|
|**KICK** #channel user|The KICK command must remove the named user from the named channel if and only if the current user is an operator of the server.|
|**PRIVMSG** (#channel or user) message|The PRIVMSG command will send the specified message to either all members of the named channel, or to the named user alone. Messages can be any printable ASCII character strings at most 512 characters in length.|
|**QUIT**|The QUIT command will remove the current user from all channels and disconnect from the server. Example output below. User input is in bold|

