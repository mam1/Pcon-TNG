/*
    C socket server example, handles multiple clients using threads
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

//the thread function
void *connection_handler(void *);

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , *new_sock;
	struct sockaddr_in server , client;

	int    		lsock, /* listening socket */
	            csock; /* active connection's socket */
	pid_t  		cpid;  /* child process ID from fork() */
	char   		*cmd = "/usr/local/sbin/Pcon";
	char   		*cmd_args[] = { "/usr/local/sbin/Pcon",
	                            "firstarg",
	                            "secondarg",
	                            "2",
	                            NULL
	                       }; /* note: last item is NULL */

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );

	//Bind
	if ( bind(socket_desc, (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while ( (csock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
		printf("ready to fork\n");
		sleep(2);

		cpid = fork();
		if (cpid < 0) exit(1);  /* exit if fork() fails */
		if ( cpid ) {
			/* In the parent process: */
			puts("parent");
			close( csock ); /* csock is not needed in the parent after the fork */
			waitpid( cpid, NULL, 0 ); /* wait for and reap child process */
		} else {
			/* In the child process: */
			puts("child");
			dup2( csock, STDOUT_FILENO );  	/* duplicate socket on stdout */
			dup2( csock, STDERR_FILENO );  	/* duplicate socket on stderr */
			dup2( csock, STDIN_FILENO );  	/* duplicate socket on stdin */

			close( csock );  /* can close the original after it's duplicated */
			execvp( cmd, cmd_args );   /* execvp() the command */
			sleep(15);
		}

		puts("Handler assigned");
	}

	if (csock < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

