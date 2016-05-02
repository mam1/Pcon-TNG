#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "typedefs.h"
#include "list_maint.h"


int main(void){
	char 			buff[256];		


	while(1){
		fgets(buff, sizeof(buff), stdin);
		if(strcmp(buff,"quit") == 0)
			return 0;







	}
	return 0;
}