
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define RUNNING_DIR "/tmp"
#define LOCK_FILE "daemond.lock"
#define LOG_FILE "daemond.log"

/*
 *       daemon.c
 *
 *       Copyright 2010 Vasudev Kamath <kamathvasudev@gmail.com>
 *     
 *       This program is free software; you can redistribute it and/or modify
 *       it under the terms of the GNU Lesser General Public License as published by
  *
 *       GNU General Public License for more details.
 *      
 *       You should have received a copy of the GNU General Public License
 *       along with this program; if not, write to the Free Software
 *       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *       MA 02110-1301, USA.
 */

void log_message(char *filename,char *message){
    FILE *logfile;
    logfile = fopen(filename,"a");
    if(!logfile)
        return;
    fprintf(logfile,"%s",message);
    fclose(logfile);
}

void signal_handler(int sig){
    switch(sig){
    case SIGHUP:
        log_message(LOG_FILE,"Hangup Signal Catched");
        break;
    case SIGTERM:
        log_message(LOG_FILE,"Terminate Signal Catched");
        exit(0);
        break;
    }
}

void daemonize(){
    int i,lfp;
    char str[10];

    if(getppid() == 1)
        return;
    i = fork();

    if(i < 0)
        exit(1);
    if(i > 0)
        exit(0);
    setsid();

    for(i = getdtablesize(); i >= 0; --i)
        close(i);

    i = open("/dev/null",O_RDWR);
    dup(i);
    dup(i);
    umask(027);

    chdir(RUNNING_DIR);

    lfp = open(LOCK_FILE,O_RDWR|O_CREAT,0640);
    if(lfp < 0)
        exit(1);
    if(lockf(lfp,F_TLOCK,0) < 0)
        exit(1);

    sprintf(str,"%d\n",getpid());
    write(lfp,str,strlen(str));

    signal(SIGCHLD,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);

    signal(SIGHUP,signal_handler);
    signal(SIGTERM,signal_handler);
}

int main(int argc,char **argv){
    daemonize();
    while(1)
        sleep(1);
}


// 
// 