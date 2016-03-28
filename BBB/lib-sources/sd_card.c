/* 
* @Author: mam1
* @Date:   2014-12-31 14:13:08
* @Last Modified by:   mam1
* @Last Modified time: 2015-01-02 20:50:33
*/

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


FILE *sd_open(char *fname,SYS_DAT *cdat){
    FILE 			*sd, *sys_file;
    SYS_DAT     	dummy;
    _SYS_DAT2 		sys_dat; 
    char 			sys_file_name[10] = "sys66.dat" ; 

    /* see if the file is there */
    sd = fopen(fname,"r+");
    if(sd != NULL){
        printf(" system data file <%s> opened\r\n",fname);
        return sd;
    }

    sys_file = fopen(sys_file_name,"r+");
    if(sys_file != NULL){
        printf(" system data file <%s> opened\r\n",fname);
        // return sys_file;
    }
    /* file does not exist try and create it */
    sd = fopen(fname,"w");
    if(sd == NULL){
        perror(fname);
        term1();
    }
    sys_file = fopen(sys_file_name,"w");
    if(sys_file == NULL){
        perror(fname);
        term1();
    }
    /* write a null record */
    memset(&dummy, '\0', sizeof(dummy));
    /* initialize record */
    dummy.major_version = _major_version;
    dummy.minor_version = _minor_version;
    dummy.minor_revision = _minor_revision;

    sys_dat.config.major_version = _major_version;
    sys_dat.config.minor_version = _minor_version;
    sys_dat.config.minor_revision = _minor_revision;
    sys_dat.config.channels = _NUMBER_OF_CHANNELS;
    sys_dat.config.sensors = _NUMBER_OF_SENSORS;
    sys_dat.config.commands = _CMD_TOKENS;
    sys_dat.config.states = _CMD_STATES;

    if(fwrite(&dummy, 1, sizeof(dummy), sd) != sizeof(dummy)){
        printf("\n*** error initializing system data file\r\n");
        perror(fname);
        term1();
    }
    fclose(sd);
    if(fwrite(&sys_dat, 1, sizeof(sys_dat), sys_dat) != sizeof(sys_dat)){
        printf("\n*** error initializing system data file\r\n");
        perror(sys_file_name);
        term1();
    }
    fclose(sys_dat);
    /* set file pointer to start of file */
    sd = fopen(fname,"r+");
    if(sd == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(fname);
        term1();
    }
    printf(" system data file <%s> created and initialized\r\n",fname);
    // printf(" size of cdat <%i>\r\n",(int)sizeof(*cdat));
    /***********************************************/
    sys_file = fopen(fname,"r+");
    if(sys_file == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(sys_file_name);
        term1();
    }
    printf(" system data file <%s> created and initialized\r\n",sys_file_name);

    /***********************************************/

    return sd;
}

FILE *sd_open2(char *fname,_SYS_DAT2 *cdat){
    FILE 			*sys_file;

    /* see if the file is there */
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
    /* write a null record */
    memset(&dummy, '\0', sizeof(dummy));
    /* initialize record */
    dummy.major_version = _major_version;
    dummy.minor_version = _minor_version;
    dummy.minor_revision = _minor_revision;

    sys_dat.config.major_version = _major_version;
    sys_dat.config.minor_version = _minor_version;
    sys_dat.config.minor_revision = _minor_revision;
    sys_dat.config.channels = _NUMBER_OF_CHANNELS;
    sys_dat.config.sensors = _NUMBER_OF_SENSORS;
    sys_dat.config.commands = _CMD_TOKENS;
    sys_dat.config.states = _CMD_STATES;

    if(fwrite(&dummy, 1, sizeof(dummy), sd) != sizeof(dummy)){
        printf("\n*** error initializing system data file\r\n");
        perror(fname);
        term1();
    }
    fclose(sd);
    if(fwrite(&sys_dat, 1, sizeof(sys_dat), sys_dat) != sizeof(sys_dat)){
        printf("\n*** error initializing system data file\r\n");
        perror(fname);
        term1();
    }
    fclose(sys_dat);
    /* set file pointer to start of file */
    sd = fopen(fname,"r+");
    if(sd == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(fname);
        term1();
    }
    printf(" system data file <%s> created and initialized\r\n",fname);
    // printf(" size of cdat <%i>\r\n",(int)sizeof(*cdat));
    /***********************************************/
    sys_file = fopen(fname,"r+");
    if(sys_file == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(fname);
        term1();
    }
    printf(" system data file <%s> created and initialized\r\n",fname);

    /***********************************************/

    return sd;
}

void save_system_data(char *fname,SYS_DAT *cdat){
    FILE *sd;
    sd = sd_open(fname,cdat);
    if(fwrite(cdat, sizeof(*cdat), 1, sd) != 1){
        perror(fname);
        term1();
    }
    fclose(sd);
    return;
}
void load_system_data(char *fname,SYS_DAT *cdat){
    FILE *sd;
    int     rtn;
    sd = sd_open(fname,cdat);
#ifdef _TRACE
    trace(_TRACE_FILE_NAME, "sd_card", 0, NULL, "\nloading system data", 1);
    printf("  file handle %i\n",(int)sd);
    printf("  size of sys_dat %i\r\n",(int)sizeof(*cdat));
    printf("  read system data\r\n");
#endif 
    rtn = fread(cdat, sizeof(*cdat), 1, sd);
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

void save_system_data2(char *fname,_SYS_DAT2 *cdat){
    FILE *system_file;
    system_file = sd_open(fname,cdat);
    if(fwrite(cdat, sizeof(*cdat), 1, sd) != 1){
        perror(fname);
        term1();
    }
    fclose(sd);
    return;
}
void load_system_data2(char *fname,SYS_DAT *cdat){
    FILE *sd;
    int     rtn;
    sd = sd_open(fname,cdat);
#ifdef _TRACE
    trace(_TRACE_FILE_NAME, "sd_card", 0, NULL, "\nloading system data", 1);
    printf("  file handle %i\n",(int)sd);
    printf("  size of sys_dat %i\r\n",(int)sizeof(*cdat));
    printf("  read system data\r\n");
#endif 
    rtn = fread(cdat, sizeof(*cdat), 1, sd);
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

int comp_conf(_CONFIG_DAT *conf)){

	if (conf.channels != _NUMBER_OF_CHANNELS
		return 1;
    if (conf.sensors != _NUMBER_OF_SENSORS
    	return 1;
    if (conf.commands != _CMD_TOKENS
    	return 1;
    if (conf.states != _CMD_STATES
    	return 1;

    return 0;
}