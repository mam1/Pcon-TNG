#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


int main(void){
//     int c;
//     do {
//         c = getchar();
//         putchar(c);
//         printf(" hex <%x>, dec <%i>\n", c, c);
//     } while(c != EOF);
//     return 0;
// }

	if (getchar() == '\033') { // if the first value is esc
	    getchar(); // skip the [
	    switch(getchar()) { // the real value
	        case 'A':
	            printf("up arrow\n");
	            break;
	        case 'B':
	            printf("down arrow\n");
	            break;
	        case 'C':
	            printf("right arrow\n");
	            break;
	        case 'D':
	            printf("left arrow\n");
	            break;
	    }
	}
}

