/* 
* @Author: mam1
* @Date:   2014-12-31 14:13:08
* @Last Modified by:   mam1
* @Last Modified time: 2015-01-02 20:50:33
*/

#include <stdio.h>
#include <unistd.h>     //sleep
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.
#include <ctype.h>      //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "Pcon.h"
#include "sd_card.h"
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.


FILE *sd_open(char *fname,SYS_DAT *cdat){
    FILE 		*sd;
    SYS_DAT     dummy;    

    /* see if the file is there */
    sd = fopen(fname,"r+");
    if(sd != NULL){
        // printf(" system data file <%s> opened\r\n",fname);
        return sd;
    }
    /* file does not exist try and create it */
    sd = fopen(fname,"w");
    if(sd == NULL){
        perror(fname);
        term1();
    }
    /* write a null record */
    memset(&dummy, '\0', sizeof(dummy));
    /* initialize record */
    dummy.major_version = _major_version;
    dummy.minor_version = _minor_version;
    dummy.minor_revision = _minor_revision;
    if(fwrite(&dummy, 1, sizeof(dummy), sd) != sizeof(dummy)){
        printf("\n*** error initializing system data file\r\n");
        perror(fname);
        term1();
    }
    fclose(sd);
    /* set file pointer to start of file */
    sd = fopen(fname,"r+");
    if(sd == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(fname);
        term1();
    }
    printf(" system data file <%s> created and initialized\r\n",fname);
    // printf(" size of cdat <%i>\r\n",(int)sizeof(*cdat));
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
    // printf("  file handle %i\n",sd);
    // printf("  size of sys_dat %i\r\n",(int)sizeof(*cdat));
    // printf("  read system data\r\n");
    rtn = fread(cdat, sizeof(*cdat), 1, sd);
    // printf("  fread returns %i\r\n",rtn);
    if(rtn != 1){
        printf("\n*** error reading system data\n  fread returned %i\r\n",rtn);
        perror(fname);
        term1();
    }
    fclose(sd);    
    return;
}