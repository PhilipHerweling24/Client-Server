/*
**Author
Name : Philip Herweling
Student Number : c18470774

**Information 
- I commented the code through out explaining what everything does
- I indented the code on notebook++, just incase the code doesnt look 
	indented properly on other editors.

**Lab Assignment Description:
This code is for the client server lab assignment.
(1)	For this lab assignment i had to write code which recieved data 
	from the client and stored it in the recvbuffer.
	
(2)	I had to seperate the two strings which are stored in the recvbuffer 
	i.e. username and password and store them in two new buffers which 
	I called usernameBuff and passwordBuff
	
(3) I then checked if the username and pasword were the right ones
	i.e. "admin" and "pass", if they were i sent back confirmation to 
	the client informing the user they successfully logged in.
	
(4) if the username and password entered were incorrect i returned 
	a message to the client which informed the user there log-in attempt 
	failed and how many attempts they had left e.g(user only gets 3 attempts).
	
(5) if the user failed to enter the right username and password 3 times i
	returned a message to the client telling the user that they were denied access
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"
#include <unistd.h>

//Final code tested and commented!!!

static const int MAXPENDING = 5; // Maximum outstanding connection requests

int main(int argc, char *argv[]) {
	
	char sendbuffer[BUFSIZE]; // Buffer for sending data to the client
	char recvbuffer[BUFSIZE];
	char usernameBuff[BUFSIZE];
	char passwordBuff[BUFSIZE]; 
	int numBytes = 0;

	if (argc != 2){
		DieWithUserMessage("Parameter(s)", "<Server Port>");
	}

	in_port_t servPort = atoi(argv[1]); // First arg:  local port

	// Create socket for incoming connections
	int servSock; // Socket descriptor for server
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		DieWithSystemMessage("socket() failed");
	}

	// Construct local address structure
	struct sockaddr_in servAddr;                  // Local address
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(servPort);          // Local port

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
		DieWithSystemMessage("bind() failed");
	}

	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0){
		DieWithSystemMessage("listen() failed");
	}
		

	for (;;) 
	{ // Infinite for loop; runs forever

		//count variable is used to keep track 
		//of how many times the user has attempted to sign in
		int count = 3;    		

	    // Wait for a client to connect
    	int clntSock = accept(servSock, (struct sockaddr *) NULL, NULL);
		
		//checks to see if clint connection failed
	    if (clntSock < 0){
      		DieWithSystemMessage("accept() failed");
		}

		//Infinite while loop allows the server
		//to recieve data from the client
		//more then once and send data more than once
		//i.e. an infinite amount of times 
		while(1){
			
			//This is where the data being sent from the client 
			//is being is being storede in the recieve buffer
    		while ((numBytes = recv(clntSock, recvbuffer, BUFSIZE -1, 0)) > 0) 
			{
				recvbuffer[numBytes] = '\0';
		 		fputs(recvbuffer, stdout);
				
				//if the recv buffer recieves the two strings
				//i.e. username and password, it breaks out of the 
				//while loop
		 		if (strstr(recvbuffer, "\r\n\r\n") > 0){
					break;
				}//end of if 
				
			}//end of innner while loop

			//if numbytes is < 0 then the recieve failed
			//an an error message is displayed
			if (numBytes < 0){
				DieWithSystemMessage("recv() failed");
			}

			//This seperates the two strings in the recieve buffer
			//into two seperate buffers i.e. the username is stored in the 
			//usernameBuff and the password is stored in the passwordBuff
    		sscanf(recvbuffer, "%s %s", usernameBuff, passwordBuff);

			//This if checks if the string stored in the usernameBuff 
			//is "admin" and checks if the string in passwordBuff 
			//is "pass"
    		if (strcmp(usernameBuff, "admin") == 0 && strcmp(passwordBuff, "pass") == 0){
				
				//if the condition is met "PROCEED\r\n" is stored
				//in the sendbuffer
				snprintf(sendbuffer, sizeof(sendbuffer), "PROCEED\r\n");
    		}//end if
			
			//the else if is entered when the count variable is greater than 1
			//This makes sure the user doesnt get more than three attempts to 
			//sign in
			else if (count > 1){
				
				//if the condition is met the count variable
				//is decremented by 1. this is done so the user
				//can only get three attempts to sign in
				count--;
				
				//this if checks if count is greater than
				if(count > 1){
					//if the condition is met "Failed attempt, %d attempts left\r\n" 
					//is stored in the sendbuffer, this tells the user they entered 
					//the wrong username or password and tells them how many attempts 
					//they have left
					snprintf(sendbuffer, sizeof(sendbuffer), "Failed attempt, %d attempts left\r\n",count);
				}
				//this else changes the message that is returned to let the user
				//know they are on there final attempt
				else{
					snprintf(sendbuffer, sizeof(sendbuffer), "Failed attempt, This is your final attempt\r\n");
				}
			}//end of else if
			
			//else statement is entered when the count variable is
			//zero i.e. when the user enters the wrong username or 
			//password three times the else statement is entered
			else{
				
				//"DENIED\r\n" is stored in the sendbuffer
				snprintf(sendbuffer, sizeof(sendbuffer), "DENIED\r\n");
    		}//end of else

    		// clntSock is connected to a client!
			//This sends the sendbuffer to the clients recieve buffer
    		ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0); 
				
			//checks if numBytesSent is less than zero
    		if (numBytesSent < 0){
				
				//if condition is met the send failed
				//and the user is shown the following message
      			DieWithSystemMessage("send() failed");
			}//end of if
			
			//this checks the the data in the sendbuffer, 
			//if "PROCEED\r\n" or "DENIED\r\n" is stored in the 
			if(strcmp(sendbuffer, "PROCEED\r\n") == 0 || strcmp(sendbuffer, "DENIED\r\n") == 0 ){
				
				//if the condition is met then it breaks out of the
				//infinte while loop because at that point the user 
				//either entered in the correct username and password
				//and was granted access or has used up there three 
				//attempts and were denied access 
				break;
			}//end of if

		}//end of while loop
		
		close(clntSock); // Close client socket
	
  	}  //end infinite for loop 
	
}//end of class
