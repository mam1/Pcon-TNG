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
	int 			a[8];
	int 			index;
	int 			i, n;		

	index = 0;
	for(i=0;i<8;i++)
		a[i] = 0;
 	printf("\n ***** test list maint *****\n\n");
	while(1){
		printf(" enter add delete or quit > ");
		fgets(buff, sizeof(buff), stdin);
		strtok(buff, "\n");
		printf(" buff <%s>\n", buff);
		if(strcmp(buff,"quit") == 0)
			return 0;

		printf(" enter channel number > ");
		scanf ("%d",&n);


		if(strcmp(buff,"add") == 0){
			printf("adding\n");
			printf("  ->index %i\n", index);
			add_elm(a, &index, n, sizeof(a));
			printf("  ->index %i\n", index);

		}

		else if(strcmp(buff,"delete") == 0){
			printf("deleting\n");
			printf("  ->index %i\n", index);
			del_elm(a, &index, n, sizeof(a));
			printf("  ->index %i\n", index);
		}

		for(i=0;i<8;i++)
			printf(" %i --- %i\n", i,  a[i]);
		printf("\n");

		fgets(buff, sizeof(buff), stdin);
		for(i=0;i<sizeof(buff);i++)
			buff[i] = '\0';
	}
	return 0;
}