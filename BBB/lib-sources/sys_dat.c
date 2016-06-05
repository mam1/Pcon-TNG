#include <stdio.h>
#include <errno.h>
#include <unistd.h>     //sleep
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.
#include <ctype.h>      //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "Pcon.h"
// #include "sd_card.h"
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.
#include "trace.h"
#include "sys_dat.h"
#include "sch.h"
#include "typedefs.h"
#include "shared.h"

extern int              trace_flag;                     //trace file is active

FILE *sys_open(char *fname,_SYS_DAT *sdat){
    FILE            *sys_file;
 
    sys_file = fopen(fname,"r+");
    if(sys_file != NULL){
        // printf(" system data file <%s> opened\r\n",fname);
        return sys_file;
    }

    /* file does not exist try and create it */
    sys_file = fopen(fname,"w");
    if(sys_file == NULL){
        perror(fname);
        exit(1);
    }

    /* initialize config data */
    sdat->config.major_version = _MAJOR_VERSION;
    sdat->config.minor_version = _MINOR_VERSION;
    sdat->config.minor_revision = _MINOR_REVISION;
    sdat->config.channels = _NUMBER_OF_CHANNELS;
    sdat->config.sensors = _NUMBER_OF_SENSORS;
    sdat->config.commands = _CMD_TOKENS;
    sdat->config.states = _CMD_STATES;
    

    if(fwrite(sdat, 1, sizeof(*sdat), sys_file) != sizeof(*sdat)){
        printf("\n*** error initializing system data file\r\n");
        perror(fname);
        exit(1);
    }

    /* set file pointer to start of file */
    fclose(sys_file);
    sys_file = fopen(fname,"r+");
    if(sys_file == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(_TRACE_FILE_NAME);
        exit(1);
    }
    printf(" system data file <%s> created and initialized\r\n",fname);
    return sys_file;
}

void sys_load(FILE *sd, _SYS_DAT *sdat){
    int     rtn;
    char 	b[20];

#if defined (_ATRACE) || defined (_FTRACE)
    trace(_TRACE_FILE_NAME, "sys_load", 0, NULL, "loading system data", 1);
    sprintf(b, "file handle %i", (int)sd);
    trace1("sys_dat", "Pcon", b);
    sprintf(b, "size of sys_dat %i", (int)sizeof(*sdat));
    trace1("sys_dat", "Pcon", b);
    trace1("sys_dat", "Pcon", "loading system data from system file");
#endif 
    rtn = fread(sdat, sizeof(*sdat), 1, sd);
#if defined (_ATRACE) || defined (_FTRACE)
    // printf("  fread returns %i\r\n",rtn);
#endif 
    if(rtn != 1){
        printf("\n*** error reading system data\n  fread returned %i\r\n",rtn);
        perror(_TRACE_FILE_NAME);
        exit(1);
    }   
    return;
}

int sys_save(FILE *sd ,_SYS_DAT *sdat){
	char 	b[128];
#if defined (_ATRACE) || defined (_FTRACE)
	sprintf(b, "saving system file with major_version = %i", sdat->config.major_version);
	trace1("sys_dat", "Pcon", b); 
#endif
    if(fwrite(sdat, sizeof(*sdat), 1, sd) != 1){
        perror(_TRACE_FILE_NAME);
        return 1;
    }
    return 0;
}

int sys_comp(_CONFIG_DAT *config){
    // printf("sys_comp called with minor revision = %i, system = %i\n",config->minor_revision, _MINOR_REVISION);


    if(_MAJOR_VERSION != config->major_version){
        printf("\n*** major verions do not match\n");
        if(trace_flag == true){
            trace1(_TRACE_FILE_NAME, "sys_comp", "major verions do not match");
            trace3(_TRACE_FILE_NAME, "sys_comp", "major version = ",_MAJOR_VERSION);
        }
        return 1;
    }
    if(_MINOR_VERSION != config->minor_version){
        printf("\n*** minor versions do not match\n");
        if(trace_flag == true){
            trace1(_TRACE_FILE_NAME, "sys_comp", "minor verions do not match");
            trace3(_TRACE_FILE_NAME, "sys_comp", "minor version = ",_MINOR_VERSION);
        }
        return 1;
    }
    if( _MINOR_REVISION != config->minor_revision){
        printf("\n*** minor revisions do not match\n");
        if(trace_flag == true){
            trace1(_TRACE_FILE_NAME, "sys_comp", "minor revisions do not match");
            trace3(_TRACE_FILE_NAME, "sys_comp", "minor revision = ",_MINOR_REVISION);
        }
        return 1;
    }
    if(_NUMBER_OF_CHANNELS != config->channels){
        printf("\n*** number of channels do not match\n");
        if(trace_flag == true){
            trace1(_TRACE_FILE_NAME, "sys_comp", "number of channels do not match");
            trace3(_TRACE_FILE_NAME, "sys_comp", "channels = ",_NUMBER_OF_CHANNELS);
        }
        return 1;
    }
    if(_NUMBER_OF_SENSORS != config->sensors){
        printf("\n*** number of sensors do not match\n");
        if(trace_flag == true){
            trace1(_TRACE_FILE_NAME, "sys_comp", "number of sensors do not match");
            trace3(_TRACE_FILE_NAME, "sys_comp", "sensors = ",_NUMBER_OF_SENSORS); 
        }
        return 1;
    }
    if(_CMD_TOKENS != config->commands){
        printf("\n*** number of commands do not match\n");
        if(trace_flag == true){
            trace1(_TRACE_FILE_NAME, "sys_comp", "number of commands do not match");
            trace3(_TRACE_FILE_NAME, "sys_comp", "tokens = ",_CMD_TOKENS);
        }
        return 1;
    }
    if(_CMD_STATES != config->states){
        printf("\n*** number of states do not match\n");
        if(trace_flag == true){
            trace1(_TRACE_FILE_NAME, "sys_comp", "number of states do not match");
            trace3(_TRACE_FILE_NAME, "sys_comp", "states = ",_CMD_STATES);
        }
    return 1;
    }

    return 0;
}

/* write system info to stdout */
void sys_disp(_SYS_DAT *s){
	printf(" Pcon  %d.%d.%d \n\r", s->config.major_version, s->config.minor_version, s->config.minor_revision);
	printf(" configured for controlling %i channels\n\r",s->config.channels);
	printf(" configured for reading %i sensors\n\r",s->config.sensors);
	// printf(" input buffer size: %d characters\n\r", _INPUT_BUFFER_SIZE);
	// printf(" system data size: %d bytes\r\n", sizeof(*s));
	// printf(" system schedule size: %d bytes\r\n", sizeof(s->sys_sch));
	// printf(" stored schedule templates: %i\r\n", s->schlib_index);

	return;
}
