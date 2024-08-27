#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */


#define SERVER_PORT 29229
#define TRUE 1
#define FALSE 0

struct ClientMessage{
	
	enum{Login, Follow, Post, Request, Search, Delete, Unfollow, Logout}requestType;
	
	unsigned int userID;
	unsigned int leaderID;
	char message[140];
};


struct ServerMessage{
	unsigned int leaderID;
	char message[140];
};  





