#include <stdio.h>
#include "cgi.h"

int main(void) {
  
    printf("Content-type: text/plain\n\n");
  	dispevars();
    printf("step counter\n");
	// count();

    return 0;
}