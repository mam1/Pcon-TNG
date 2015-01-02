/* 
* @Author: mam1
* @Date:   2014-12-31 14:13:08
* @Last Modified by:   mam1
* @Last Modified time: 2015-01-02 16:08:49
*/

#include <stdio.h>
#include "Pcon.h"
#include "sd_card.h"

FILE *sd_open(char *fname){
    FILE 		*sd;
    uint8_t     null = 0;

    /* see if the file is there */
    sd = fopen(fname,"R");
    if(sd != NULL){
        printf(" system data file <%s> opened\r\n",fname);
        return sd;
    }

    /* file does not exist try and create it */
    sd = fopen(fname,"w");
    if(sd == NULL){
        printf("\n*** error opening system data file\r\n");
        perror(fname);
        term1();
    }
    /* write a null record */
    if(fwrite(&null, 1, sizeof(sys_dat), sd) != sizeof(sys_dat)){
        printf("\n*** error initializing system data file\r\n");
        perror(fname);
        term1();
    }
    fclose(sd);
    sd = fopen(fname,"w");
    if(sd == NULL){
        printf("\n*** error reopening system data file\r\n");
        perror(fname);
        term1();
    }
    printf(" system data file <%s> created and initilaized\r\n",fname);
    printf("size of sys_dat %i, sd <%i>\r\n",sizeof(sys_dat),(int)sd);
    return sd;
}

void save_channel_data(char *fname,SYS_DAT *s){
    FILE *sd;
    sd = sd_open(fname);
    if(fwrite(&sys_dat, sizeof(sys_dat), 1, sd) != sizeof(sys_dat)){
        printf("\n*** error reading system data file\r\n");
        perror(fname);
        term1();
    }
    fclose(sd);
    return;
}
void load_channel_data(char *fname,SYS_DAT *s){
    FILE *sd;
    sd = sd_open(fname);
    printf("size of sys_dat %i, sd <%i>\r\n",sizeof(sys_dat),(int)sd);
    printf("fread retunrs %i\r\n",fread(&sys_dat, sizeof(sys_dat), 1, sd));
    // if(fread(&sys_dat, sizeof(sys_dat), 1, sd) != sizeof(sys_dat)){
    //     printf("\n*** error reading system data file\r\n");
    //     perror(fname);
    //     term1();
    // }
    fclose(sd);    
    return;
}