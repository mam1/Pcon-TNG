#include <stdio.h>
#include <errno.h>
#include <unistd.h>     //sleep
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.
#include <ctype.h>      //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "Pcon.h"
#include "sd_card.h"
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.
#include "trace.h"

FILE *sys_open(char *fname,_SYS_DAT2 *sdat){
    FILE            *sys_file;
 
    sys_file = fopen(fname,"r+");
    if(sys_file != NULL){
        printf(" system data file <%s> opened\r\n",fname);
        return sys_file;
    }

    /* file does not exist try and create it */
    sys_file = fopen(fname,"w");
    if(sys_file == NULL){
        perror(fname);
        term1();
    }

    /* initialize config data */
    sdat->config.major_version = _major_version;
    sdat->config.minor_version = _minor_version;
    sdat->config.minor_revision = _minor_revision;
    sdat->config.channels = _NUMBER_OF_CHANNELS;
    sdat->config.sensors = _NUMBER_OF_SENSORS;
    sdat->config.commands = _CMD_TOKENS;
    sdat->config.states = _CMD_STATES;

    if(fwrite(sdat, 1, sizeof(*sdat), sys_file) != sizeof(*sdat)){
        printf("\n*** error initializing system data file\r\n");
        perror(fname);
        term1();
    }

    /* set file pointer to start of file */
    fclose(sys_file);
    sys_file = fopen(fname,"r+");
    if(sys_file == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(fname);
        term1();
    }
    printf(" system data file <%s> created and initialized\r\n",fname);
    return sys_file;


}

void sys_load(char *fname,_SYS_DAT2 *sdat){
    FILE *sd;
    int     rtn;
    sd = sys_open(fname,sdat);
#ifdef _TRACE
    trace(_TRACE_FILE_NAME, "sd_card", 0, NULL, "\nloading system data", 1);
    printf("  file handle %i\n",(int)sd);
    printf("  size of sys_dat %i\r\n",(int)sizeof(*sdat));
    printf("  read system data\r\n");
#endif 
    rtn = fread(sdat, sizeof(*sdat), 1, sd);
#ifdef _TRACE
    printf("  fread returns %i\r\n",rtn);
#endif 
    if(rtn != 1){
        printf("\n*** error reading system data\n  fread returned %i\r\n",rtn);
        perror(fname);
        term1();
    }
    fclose(sd);    
    return;
}

void sys_save(char *fname,_SYS_DAT2 *sdat){
    FILE *sd;
    sd = sys_open(fname,sdat);
    if(fwrite(sdat, sizeof(*sdat), 1, sd) != 1){
        perror(fname);
        term1();
    }
    fclose(sd);
    return;
}