int main(int argc , char *argv[])
{


	int    		lsock, /* listening socket */
	            csock; /* active connection's socket */
	pid_t  		cpid;  /* child process ID from fork() */
	char   		*cmd = "somecommand";
	char   		*cmd_args[] = { "somecommand",
	                            "firstarg",
	                            "secondarg",
	                            "howevermanyargs",
	                            NULL
	                       }; /* note: last item is NULL */

	int 		Pcon, Dcon;


#define RUNNING			-1
#define NOT_RUNNING		0

	Pcon = NOT_RUNNING;
	Dcon = NOT_RUNNING;
	/*  ...
	    call socket(), bind(), listen(), etc.
	    ... */

	for (;;) {  /* loop, accepting connections */
		if ( (csock = accept( lsock, NULL, NULL )) == -1) exit(1);


		cpid = fork();
		if (cpid < 0) exit(1);  /* exit if fork() fails */
		if ( cpid ) {
			/* In the parent process: */
			close( csock ); /* csock is not needed in the parent after the fork */
			waitpid( cpid, NULL, 0 ); /* wait for and reap child process */
		} else {
			/* In the child process: */
			dup2( csock, STDOUT_FILENO );  /* duplicate socket on stdout */
			dup2( csock, STDERR_FILENO );  /* duplicate socket on stderr too */
			close( csock );  /* can close the original after it's duplicated */
			execvp( Pcon, NULL );   /* execvp() the command */
		}
	}

	return 1;
}