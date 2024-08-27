#include "sharedcode.h"


void DieWithError(char *errorMessage);  /* External error handling function */

//function prototype
int mainMenu();

int main(int argc, char *argv[])
{
	//structures used to communicate between clients, and server
	struct ClientMessage clientMsg;
	struct ServerMessage serverMsg;
	
	unsigned int userID;
	int reqType;
	unsigned int leader;
	char c;
	
    int sock;                        /* Socket descriptor */
    struct sockaddr_in servAddr; /* Echo server address */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    unsigned short servPort;     /* Echo server port */
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    char *servIP;                /* String to send to echo server */
    
    if ((argc < 2) || (argc > 3))    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP>  [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    
    if (argc == 3)
        servPort = atoi(argv[2]);  /* Use given port, if any */
    else
        servPort = 7;  /* 7 is the well-known port for the echo service */

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));    /* Zero out structure */
    servAddr.sin_family = AF_INET;                 /* Internet addr family */
    servAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    servAddr.sin_port   = htons(servPort);     /* Server port */
	
	//sends Login request
	while(TRUE){
		
		//prompts user to login
		printf("Login: ");
		scanf("%d", &userID);
		while ( (c = getchar()) != EOF && c != '\n') { }//called everytime scanf is used so fgets works
		
		//create login request for server
		clientMsg.requestType = Login;
		clientMsg.userID = userID;
		clientMsg.leaderID = NULL;
		strcpy(clientMsg.message, " ");
		
		//send login message
		if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
					&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
			DieWithError("Sendto() failed");
		}
		//recieve confirmation
		fromSize = sizeof(fromAddr);
		if(recvfrom(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
					&fromAddr, &fromSize) != sizeof(serverMsg)){
			DieWithError("recvfrom() failed");
		}
		if(strcmp(serverMsg.message, "taken") == 0){
			printf("UserID: %d has already been taken\n", userID);
		}
		else{
			printf("\nLogged in as: %s\n", serverMsg.message);
			break;
		}
	}
	
	while(TRUE){
		//brings up main menu
		reqType = mainMenu();
		printf("%d", reqType);
		
		//sends Follow request
		if(reqType == 1){
			
			int leaderCount;
			
			//prompts user to enter ID of user they want to follow
			printf("\nEnter the user ID of the user you wish to follow: ");
			scanf("%d",&leader);
			while ( (c = getchar()) != EOF && c != '\n') { }
			
			//create follow request for server
			clientMsg.requestType = Follow;
			clientMsg.leaderID = leader;
			strcpy(clientMsg.message, " ");
			
			//sends request to server
			if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("sendto() follow request failed");
			}
			//receves leaderCount
			fromSize = sizeof(fromAddr);
			if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
						&fromAddr, &fromSize) != sizeof(serverMsg)){
				DieWithError("recvfrom() follow request failed");
			}
			
			printf("You are following:\n");
			leaderCount = atoi(serverMsg.message);
			int x;
			for(x = 0; x < leaderCount; x++){
				
				if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
							&fromAddr, &fromSize) != sizeof(serverMsg)){
					DieWithError("recvfrom() follow request failed");
				}
				printf("%d\n",serverMsg.leaderID);
			}
			
		}
		//sends Post request
		else if(reqType == 2){
		
			//used to store users post
			char input[140];
			int len;
			
			//prompt user to enter post
			printf("\nEnter your post: ");
			fgets(input, sizeof(input), stdin);
			
			len = strlen(input);
			
			//formats post to be read properly
			if(input[len-1] == '\n'){
				input[len-1] = '\0';
			}
			
			//creates Post request for server
			clientMsg.requestType = Post;
			strcpy(clientMsg.message, input);
			//sends request to server
			if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("sendto() Post failed");
			}
			//recieve confirmation from server
			if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
						&fromAddr, &fromSize) != sizeof(serverMsg)){
				DieWithError("recvfrom() follow request failed");
			}
			if(strcmp(clientMsg.message,serverMsg.message) == 0){
				printf("Post successful.\n");
			}
		}
		//sends Request request
		else if(reqType == 3){
			//the number of message to be receved from server
			int retCount;
			
			//creates Request request for server
			clientMsg.requestType = Request;
			
			if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("sendto() Post failed");
			}
			//receves number of posts to be sent 
			if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
						&fromAddr, &fromSize) != sizeof(serverMsg)){
				DieWithError("recvfrom() follow request failed");
			}
			
			//converts string to int
			retCount = atoi(serverMsg.message);
			
			//receves all posts sent by server, and prints to screen
			int x;
			for(x = 0; x < retCount; x++){
				
				if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
							&fromAddr, &fromSize) != sizeof(serverMsg)){
					DieWithError("recvfrom() follow request failed");
				}
				printf("%d:\n%s\n",serverMsg.leaderID,serverMsg.message);
			}
			
			
			
		}
		//sends Search request 
		else if(reqType == 4){
			//used to store input, and count posts to be returned
			char input[140];
			int len;
			int postCount;
			
			//prompts user for input
			printf("Enter you keyword: ");
			fgets(input, sizeof(input), stdin);
			
			len = strlen(input);
			
			//formats string for proper use
			if(input[len-1] == '\n'){
				input[len-1] = '\0';
			}
			
			
			//creates search request
			clientMsg.requestType = Search;
			strcpy(clientMsg.message, input);
			//sends request to server
			if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("Sendto() failed");
			}
			//receves number of posts to be returned
			if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
						&fromAddr, &fromSize) != sizeof(serverMsg)){
				DieWithError("recvfrom() follow request failed");
			}
				
			//converts string to int
			postCount = atoi(serverMsg.message);
			
			//receves all posts matching keyword, and prints to screen
			int x;
			for(x = 0; x < postCount; x++){
				if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
							&fromAddr, &fromSize) != sizeof(serverMsg)){
					DieWithError("recvfrom() follow request failed");
				}
				printf("user, %d, says:\n%s\n",serverMsg.leaderID,serverMsg.message);
			}
			
			
		}
		//sends Delete request
		else if(reqType == 5){
			//counts posts to be receved from server
			int postCount;
			int delopt;
			
			//creates delete request for server
			clientMsg.requestType = Delete;
			//sends to server
			if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("Sendto() failed");
			}
			//receves number of posts to be returned
			fromSize = sizeof(fromAddr);
			if(recvfrom(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
						&fromAddr, &fromSize) != sizeof(serverMsg)){
				DieWithError("recvfrom() failed");
			}
			
			//converts from string to int
			postCount = atoi(serverMsg.message);
			
			//prompts user for input, and displays all posts made by user
			printf("Select the message to be deleted: \n");
			int x;
			for(x = 0; x < postCount; x++){
				if(recvfrom(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
						&fromAddr, &fromSize) != sizeof(serverMsg)){
					DieWithError("recvfrom() failed");
				}
				
				printf("%d. %s\n", x+1, serverMsg.message);
			}
			scanf("%d", &delopt);
			while ( (c = getchar()) != EOF && c != '\n') { }
			
			//converts from int to string to be sent to server
			sprintf(clientMsg.message,"%d",delopt);
			//sends users choice to server
			if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("Sendto() failed");
			}
			
			
			
			
		}
		//sends Unfollow request
		else if(reqType == 6){
			
			/***********************************
				display list of leaders
			***********************************/
			int leaderCount;
			
			//prompts user for input
			printf("\nEnter the id of the user you would like to unfollow: ");
			scanf("%d",&leader);
			while ( (c = getchar()) != EOF && c != '\n') { }
			
			//creates unfollow request
			clientMsg.requestType = Unfollow;
			clientMsg.leaderID = leader;
			strcpy(clientMsg.message, " ");
			//sends request to server
			if(sendto(sock,&clientMsg,sizeof(clientMsg),0,(struct sockaddr *) 
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("sendto() follow request failed");
			}
			//recieves confirmation
			fromSize = sizeof(fromAddr);
			if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
						&fromAddr, &fromSize) != sizeof(serverMsg)){
				DieWithError("recvfrom() follow request failed");
			}
			
			leaderCount = atoi(serverMsg.message);
			
			printf("You are now following:\n");
			int x;
			for(x = 0; x < leaderCount; x++){
				if(recvfrom(sock,&serverMsg,sizeof(serverMsg),0,(struct sockaddr *) 
							&fromAddr, &fromSize) != sizeof(serverMsg)){
					DieWithError("recvfrom() follow request failed");
				}
				printf("%d\n",serverMsg.leaderID);
			}
			
			
		}
		//sends Logout request
		else if(reqType == 7){
			
			//informs user is being logged off, and sends request to server
			printf("\nLogging out user: %d \n", clientMsg.userID);
			clientMsg.requestType = Logout;
			
			if(sendto(sock, &clientMsg, sizeof(clientMsg), 0, (struct sockaddr *)
						&servAddr, sizeof(servAddr)) != sizeof(clientMsg)){
				DieWithError("sendto() failed: logout message");
			}
			break;
		}
		//if input of user is not correct
		else{
			printf("invalid entry, try again.");
		}
	} 
    
    close(sock);
    exit(0);
}

int mainMenu(){
	int choice;
	char c;
	
	//propmts user for input
	printf("\nPlease choose an option: \n");
	printf("1. Follow\n");
	printf("2. Post\n");
	printf("3. Request\n");
	printf("4. Search\n");
	printf("5. Delete\n");
	printf("6. Unfollow\n");
	printf("7. Logout\n");
	
	//reads input, and returns result 
	scanf("%d", &choice);
	while ( (c = getchar()) != EOF && c != '\n') { }
	
	return choice;
}
