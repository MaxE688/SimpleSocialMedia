#include "sharedcode.h"



void DieWithError(char *errorMessage);  /* External error handling function */

//The node used for the linked-list of user posts
typedef struct postNode{
	unsigned int userID;
	char text[140];
	char tags[140];
	struct postNode *next;
};
//The node used for the linked-list of leaders for each user
typedef struct leaderNode{
	unsigned int userID;
	struct leaderNode *next;
};
//The node used for the linked-list of users
typedef struct userNode{
	unsigned int userID;
	struct leaderNode *leaderRoot;
	struct leaderNode *pointer;
	struct leaderNode *pointer2;
	struct userNode *next;
};
//The node used for the temperary linked-list of posts to be returned to
//the server
typedef struct retNode{
	unsigned int userID;
	char text[140];
	struct retNode *next;
};

int main(int argc, char *argv[])
{
	//struct of messages sent between client and server
	struct ClientMessage clientMsg;
	struct ServerMessage serverMsg;
	
	//used for linked-list of posts to return
	struct retNode *retRoot = NULL;
	struct retNode *retPointer = NULL;
	
	//used for linked-list of users
	struct userNode *userRoot = NULL;
	struct userNode *pointer = NULL;
	struct userNode *pointer2 = NULL;

	//used for linked-list of posts made by user
	struct postNode *postRoot = NULL;
	struct postNode *postPointer = NULL;
	struct postNode *postPointer2 = NULL;
	char text[140];
	char tags[140];
	
    int sock;                        /* Socket */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in clientAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short servPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */

    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    servPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(servPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");
  
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(clientAddr);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, &clientMsg, sizeof(clientMsg), 0,
            (struct sockaddr *) &clientAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

        printf("Handling client %s\n", inet_ntoa(clientAddr.sin_addr));
		
		
		
		
		
		//If the client sends Login request
		if(clientMsg.requestType == Login){
			int taken = FALSE;
			
			//if user is already logged on the client will send another login
			// request after user selects different name
			pointer = userRoot;
			while(pointer != NULL){
				if(clientMsg.userID == pointer->userID){
					taken = TRUE;
					break;
				}
				pointer = pointer->next;
			}
			if(taken){
				
				strcpy(serverMsg.message, "taken");
				
				if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
							&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
					DieWithError("sendto() failed");
				}
			}
			else{
				//If the list exists new node is added to end,
				//if not, then node is set as the root
				if(userRoot == NULL){
					userRoot = (struct userNode *) malloc(sizeof(struct userNode));
					userRoot->userID = clientMsg.userID;
					userRoot->next = NULL;
					pointer = userRoot; //might not need
				}
				else{
					pointer = userRoot;
					while(pointer->next != NULL){
						pointer = pointer->next;
					}
					pointer->next = (struct userNode *) malloc(sizeof(struct userNode));
					pointer = pointer->next;
					pointer->userID = clientMsg.userID;
					pointer->next = NULL;
				}
				
				//converts int to string to be sent to client
				sprintf(serverMsg.message, "%d", pointer->userID);
				serverMsg.leaderID = NULL;
				
				if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
							&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
					DieWithError("sendto() failed");
				}
				
				//prints userID of every node in list
				pointer = userRoot;
				while(pointer != NULL){
					printf("%d\n", pointer->userID);
					pointer = pointer->next;
				}
			}	
			
		}
		//If the client sends Follow request
		else if(clientMsg.requestType == Follow){
			
			
			/*******************************************
					Client side should be done
				
				Setup so users can't follow users 
				who don't exist
				
				send list of leaders to client
			*******************************************/
			int leaderCount = 0;
			
			
			//looks through list of users for user who made request
			pointer = userRoot;
			while(TRUE){
				if(pointer->userID == clientMsg.userID){
					break;
				}
				pointer = pointer->next;
			}
			
			//If the user is not following anybody then leaderNode is created
			//If the user is following someone, adds leaderNode to end of list
			pointer->pointer = pointer->leaderRoot;
			if(pointer->leaderRoot == NULL){
				pointer->leaderRoot = (struct leaderNode *) malloc(sizeof(struct leaderNode));
				pointer->leaderRoot->userID = clientMsg.leaderID;
				pointer->leaderRoot->next = NULL;
				leaderCount++;
			}
			else{
				while(TRUE){
					leaderCount++;
					if(pointer->pointer->next == NULL){
						pointer->pointer->next = (struct leaderNode *) malloc(sizeof(struct leaderNode));
						pointer->pointer = pointer->pointer->next;
						pointer->pointer->userID = clientMsg.leaderID;
						pointer->pointer->next = NULL;
						leaderCount++;
						break;
					}
					pointer->pointer = pointer->pointer->next;
				}
			}
			
			sprintf(serverMsg.message,"%d",leaderCount);
			//sends leaderCount to client
			if(sendto(sock, &serverMsg, sizeof(serverMsg),0,(struct sockaddr *) 
						&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
				DieWithError("sendto() follow confirmation failed");			
			}
			
			pointer->pointer = pointer->leaderRoot;
			int x;
			for(x = 0; x < leaderCount; x++){
				
				serverMsg.leaderID = pointer->pointer->userID;
				
				if(sendto(sock, &serverMsg, sizeof(serverMsg),0,(struct sockaddr *) 
							&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
					DieWithError("sendto() follow confirmation failed");			
				}
				pointer->pointer = pointer->pointer->next;
			}
			
			//prints list of all leaders ID's 
			pointer->pointer = pointer->leaderRoot;
			while(pointer->pointer != NULL){
				printf("%d\n", pointer->pointer->userID);
				pointer->pointer = pointer->pointer->next;
			}
			
			
			
		}
		//If the client sends Post request
		else if(clientMsg.requestType == Post){
			//tagPos is used to place tags in the string of tags
			int tagPos = 0;
			//looks for tags in the users post, and pulls them out to be compared for search
			int x;
			for(x = 0; x < 140; x++){
				if(clientMsg.message[x] == '#'){
					int y;
					for(y = x; y < 140-x;y++){
						if(clientMsg.message[y] == ' ' || clientMsg.message[y] == '\0'){
							
							int i;
							int j = 0;
							for(i = x; i < y; i++){
								tags[tagPos+j] = clientMsg.message[i];
								j++;
							}
							tagPos += y-x;
							break;
						}
					}
				}
			}
			tags[tagPos] = '\0';
			
			//If there are no posts it will create Root for list
			//If there are posts it will add new post to end of list
			if(postRoot == NULL){
				postRoot = (struct postNode *) malloc(sizeof(struct postNode));
				postRoot->userID = clientMsg.userID;
				strcpy(postRoot->text, clientMsg.message);
				strcpy(postRoot->tags, tags);
				postRoot->next = NULL;
			}
			else{
				postPointer = postRoot;
				while(postPointer->next != NULL){
					postPointer = postPointer->next;
				}
				postPointer->next = (struct postRoot *) malloc(sizeof(struct postNode));
				postPointer = postPointer->next;
				postPointer->userID = clientMsg.userID;
				strcpy(postPointer->text, clientMsg.message);
				strcpy(postPointer->tags, tags);
				postPointer->next = NULL;
				
				
			}
			
			//prints the tags of all posts
			postPointer = postRoot;
			while(postPointer != NULL){
				printf("%s\n", postPointer->tags);
				postPointer = postPointer->next;
			}
			
			//sends confirmation to client
			strcpy(serverMsg.message, clientMsg.message);
			if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
						&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
				DieWithError("sendto() failed");
			}
			
		}
		//If the client sends Request request
		else if(clientMsg.requestType == Request){
			/*************************************
						read requirements
			*************************************/
			//counts the number of posts that need to be returned to client
			int retCount = 0;
			
			//finds the user making the request
			pointer = userRoot;
			while(pointer->userID != clientMsg.userID){
				pointer = pointer->next;
			}
			
			//Finds all posts of users leaders
			pointer->pointer = pointer->leaderRoot;
			postPointer = postRoot;
			while(pointer->pointer != NULL){
				while(postPointer != NULL){
					if(postPointer->userID == pointer->pointer->userID){
						//add post node to return list, increment postCount
						retCount++;
						
						if(retRoot == NULL){
							
							retRoot = (struct retNode *) malloc(sizeof(struct retNode));
							retPointer = retRoot;
							retPointer->userID = postPointer->userID;
							strcpy(retPointer->text, postPointer->text);
							retPointer->next = NULL;
						}
						else{
							retPointer = retRoot;
							while(retPointer->next != NULL){
								retPointer = retPointer->next;
							}
							retPointer->next = (struct retNode *) malloc(sizeof(struct retNode));
							retPointer = retPointer->next;
							retPointer->userID = postPointer->userID;
							strcpy(retPointer->text, postPointer->text);
							retPointer->next = NULL;
						}
						
					}
					postPointer = postPointer->next;
				}
				pointer->pointer = pointer->pointer->next;
			}
			
			//converts retCount to string to be sent to client, then sends data
			sprintf(serverMsg.message,"%d",retCount);
			if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
						&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
				DieWithError("sendto() failed");
			}
			
			//sends each matching node to client 
			retPointer = retRoot;
			int x;
			for(x = 0; x < retCount; x++){
				//send data to client
				
				serverMsg.leaderID = retPointer->userID;
				strcpy(serverMsg.message, retPointer->text);
				
				if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
							&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
					DieWithError("sendto() failed");
				}
				retPointer = retPointer->next;
			}
			
			//clears the the list for use again
			retPointer = retRoot;
			while(retPointer != NULL){
				
				retRoot = retPointer->next;
				free(retPointer);
				retPointer = retRoot;
			}
			
		}
		//If the client sends Search request
		else if(clientMsg.requestType == Search){
			/**************************************
				Search the tags of every post, 
				pull each tag out of post 
				individually and compare to 
				keyword if there's a match
				add to "search" list then
				send each element back to client
				for printing.
			**************************************/
			//used to count posts to send to client
			int resultCount = 0;
			char tag[140];
			
			//goes through tags of post and compares to users keyword
			postPointer = postRoot;
			while(postPointer != NULL){
				//looks for delimiter '#' to pull individual tags of post
				int x;
				for(x = 0; x < strlen(postPointer->tags); x++){
					if(postPointer->tags[x] == '#'){
						
						int y;
						for(y = x+1; y <= strlen(postPointer->tags)-x; y++){
							//looks for end of tag to pull out of string
							if(postPointer->tags[y] == '#' || postPointer->tags[y] == '\0'
								|| postPointer->tags[y] == ' '){
								
								//puts individual tag into place holder for comparison
								int i;
								int j = 0;
								for(i = x+1; i < y; i++){
									tag[j] = postPointer->tags[i];
									j++;
								}
								tag[y-x-1] = '\0';
								printf("%s.\n",tag);
								
								//if tag match users keyword the post is added to list to be returned
								if(strcmp(tag,clientMsg.message) == 0){
									
									//add to return list, increment counter
									if(retRoot == NULL){
										retRoot = (struct retNode *) malloc(sizeof(struct retNode));
										retRoot->userID = postPointer->userID;
										strcpy(retRoot->text, postPointer->text);
										retRoot->next = NULL;
									}
									else{
										retPointer = retRoot;
										while(retPointer->next != NULL){
											retPointer = retPointer->next;
										}
										
										retPointer->next = (struct retNode *) malloc(sizeof(struct retNode));
										retPointer = retPointer->next;
										retPointer->userID = postPointer->userID;
										strcpy(retPointer->text, postPointer->text);
										retPointer->next = NULL;
									}
									resultCount++;
								}
								break;
							}
						}
					}
				}
				postPointer = postPointer->next;
			}
			
			//converts resultCount to string then sends to client
			sprintf(serverMsg.message,"%d",resultCount);
			if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
						&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
				DieWithError("sendto() failed");
			}
			
			//sends each node that matches keyword to client
			retPointer = retRoot;
			int x;
			for(x = 0; x < resultCount; x++){
				
				serverMsg.leaderID = retPointer->userID;
				strcpy(serverMsg.message, retPointer->text);
				
				if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
							&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
					DieWithError("sendto() failed");
				}
				retPointer = retPointer->next;
			}
			
			//clears the return list for reuse
			retPointer = retRoot;
			while(retPointer != NULL){
				
				retRoot = retPointer->next;
				free(retPointer);
				retPointer = retRoot;
			}
			
		}
		//If the client sends Delete request
		else if(clientMsg.requestType == Delete){
			//counts number of posts to be send to client
			int postCount = 0;
			int delopt;
			
			//counts all posts made by user
			postPointer = postRoot;
			while(postPointer != NULL){
				if(clientMsg.userID == postPointer->userID){
					postCount++;
				}
				
				postPointer = postPointer->next;
			}
			
			//converts postCount to string then sends data to client
			sprintf(serverMsg.message,"%d",postCount);
			if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
						&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
				DieWithError("sendto() failed");
			}
			
			//sends each post made by user to client
			postPointer = postRoot;
			int x;
			for(x = 0; x < postCount; x++){
				
				while(postPointer->userID != clientMsg.userID){
					postPointer = postPointer->next;
				}
				
				strcpy(serverMsg.message, postPointer->text);
				
				if(sendto(sock, &serverMsg, sizeof(serverMsg), 0, (struct sockaddr *)
							&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
					DieWithError("sendto() failed");
				}
				postPointer = postPointer->next;
			}
			
			//recieves users choice of post to delete
			if (recvfrom(sock, &clientMsg, sizeof(clientMsg), 0,
						(struct sockaddr *) &clientAddr, &cliAddrLen) < 0){
				DieWithError("recvfrom() failed");
			}
			
			//converts string from client into int 
			delopt = atoi(clientMsg.message);
			postCount = 0;
			
			//searches list of posts until it reaches the post to be removed then removes it
			postPointer = postRoot;
			postPointer2 = postPointer->next;
			if(postPointer->userID == clientMsg.userID){
				postCount++;
				if(postCount == delopt){
					postRoot = postPointer2;
					free(postPointer);
				}
			}
			while(postPointer2 != NULL){
				if(postPointer2->userID == clientMsg.userID){
					postCount++;
					if(postCount == delopt){
						//delete postCount
						printf("I'm found");
						postPointer->next = postPointer2->next;
						free(postPointer2);
						break;
					}
				}
				postPointer = postPointer2;
				postPointer2 = postPointer2->next;
			}
			
			
			printf("\n\n\n");
			
			//prints all posts in list of posts
			postPointer = postRoot;
			while(postPointer != NULL){
				printf("%s\n", postPointer->text);
				postPointer = postPointer->next;
			}
			
		}
		//If the client sends Unfollow request
		else if(clientMsg.requestType == Unfollow){
			
			/******************************************************
						Client side should be done
					
				parse list of leaders for specific user until 
				leader is found. If no leader is found return 
				appropriate output
				
				Send list of leaders after unfollow is done
			*****************************************************/
			int leaderCount = 0;
			
			
			//finds the user who made request
			pointer = userRoot;
			while(TRUE){
				if(pointer->userID == clientMsg.userID){
					break;
				}
				pointer = pointer->next;
			}
			
			//counts the leaders
			pointer->pointer = pointer->leaderRoot;
			while(pointer->pointer != NULL){
				leaderCount++;
				pointer->pointer = pointer->pointer->next;
			}
			
			//creates pointers
			pointer->pointer = pointer->leaderRoot;
			pointer->pointer2 = pointer->leaderRoot->next;
			
			//if no user is found this string will not be changed
			strcpy(serverMsg.message, "Could not find user: ");
			
			//if leader exists
			if(pointer->pointer != NULL){
				
				//adds to end of lits, if list doesnt exists creates root
				while(TRUE){
					if(pointer->leaderRoot->userID == clientMsg.leaderID){
						free(pointer->leaderRoot);
						pointer->leaderRoot = pointer->pointer->next;
						strcpy(serverMsg.message, "User succefully unfollowed: ");
						leaderCount--;
						break;
					}
					else if(pointer->pointer2->userID == clientMsg.leaderID){
						//remove node from list
						pointer->pointer->next = pointer->pointer2->next;
						free(pointer->pointer2);//might cause problem
						strcpy(serverMsg.message, "User succefully unfollowed: ");
						leaderCount--;
						break;
					}
					pointer->pointer = pointer->pointer2;
					pointer->pointer2 = pointer->pointer2->next;
				}
			}
			
			sprintf(serverMsg.message,"%d",leaderCount);
			//sends confirmation to client
			if(sendto(sock, &serverMsg, sizeof(serverMsg),0,(struct sockaddr *) 
						&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
				DieWithError("sendto() follow confirmation failed");			
			}
			
			pointer->pointer = pointer->leaderRoot;
			int x;
			for(x = 0; x < leaderCount; x++){
				
				serverMsg.leaderID = pointer->pointer->userID;
				
				if(sendto(sock, &serverMsg, sizeof(serverMsg),0,(struct sockaddr *) 
							&clientAddr, sizeof(clientAddr)) != sizeof(serverMsg)){
					DieWithError("sendto() follow confirmation failed");			
				}
				pointer->pointer = pointer->pointer->next;
			}
			
			
			
			
			//prints list of leaders
			pointer->pointer = pointer->leaderRoot;
			while(pointer->pointer != NULL){
				printf("%d\n", pointer->pointer->userID);
				pointer->pointer = pointer->pointer->next;
			}
			
			
			
		}
		//If the client sends Logout request
		else if(clientMsg.requestType == Logout){
			
			//finds user who made request and removes from list
			pointer = userRoot;
			pointer2 = pointer->next;
			if(pointer->userID == clientMsg.userID){
				userRoot = pointer2;
				free(pointer);
			}
			while(pointer2 != NULL){
				if(pointer2->userID == clientMsg.userID){
					pointer->next = pointer2->next;
					free(pointer2);
					printf("found\n");
				}
				pointer = pointer2;
				pointer2 = pointer2->next;
			}
			
			//prints list of users
			pointer = userRoot;
			while(pointer != NULL){
				printf("%d\n", pointer->userID);
				pointer = pointer->next;
			}
			
		}
        
    }
    /* NOT REACHED */
}

