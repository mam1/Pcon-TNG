#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "typedefs.h"
#include "ipc.h"

IPC_DAT        ipc_dat;                   //ipc data
char           ipc_file[] = {_IPC_FILE};  //name of ipc file
void           *data;                     //pointer to ipc data
int            fd;                        //file descriptor for ipc data file

int main (int argc, char **argv) {

  // int tflag = 0;
  // int wflag = 0;
  // int index;
  // int c;


  printf("\n*** test-ipc ver 1.5\n\n");
/* test arguments to see if this code should update or watch shared memory */
  // opterr = 0;
  // if (argc != 2) {
  //   printf("\n  a single argement is required:\n    -t (pgm modifies shared memory)\n    -w (pgm watches shared memory\n");
  //   printf("\n  program terminated\n\n");
  //   exit(1);
  // }
  // while ((c = getopt (argc, argv, "tw")) != -1)
  //   switch (c) {
  //   case 't':
  //     tflag = 1;
  //     break;
  //   case 'w':
  //     wflag = 1;
  //     break;
  //   case '?':
  //     if (isprint (optopt))
  //       fprintf (stderr, "\n  Unknown option `-%c'.\n", optopt);
  //     else
  //       fprintf (stderr,
  //                "\n  Unknown option character `\\x%x'.\n", optopt);
  //     printf("\n  program terminated\n\n");
  //     return 1;
  //   default:
  //     abort ();
  //   }
  // for (index = optind; index < argc; index++)
  //   printf ("Non-option argument %s\n", argv[index]);

  // if (wflag == 1)
  //   printf("  program configured to watch shared memory\n");
  // else if (tflag == 1)
  //   printf("  program configured to update shared memory\n");
  // else {
  //   printf("  **** this should never happen\n");
  //   return 1;
  // }

  /* set up shared memory */  
//  char *cptr = _IPC_FILE;
  fd = ipc_open(ipc_file,ipc_size());              // create/open ipc file
  data = ipc_map(fd,ipc_size());        // map file to memory
  uint8_t *data_byte;
  printf("  %s mapped to memory at %x\n",ipc_file, (uint32_t)data);
  data_byte = data;
  printf("first byte of shared memory <%x>\n",*data_byte);

  memcpy(&ipc_dat,data,sizeof(ipc_dat));    // move shared memory data to local structure

  printf("\nnormal termination\n\n");
  return 0;
}


// set up file mapped shared memory for inter process communication
// data = ipc_map(fd,ipc_size());       // map file to memory







// memcpy(&ipc_dat,data,sizeof(ipc_dat));   // move shared memory data to local structure
// printf(" %i bytes of data moved\n",sizeof(ipc_dat));
// printf("starting loop with force_update - %i\n",ipc_dat.force_update);
// while(1){
//  if(ipc_dat.force_update){
//    ipc_dat.force_update = 0;
//    memcpy(data,&ipc_dat,sizeof(ipc_dat));  // move local data into shared memory
//    printf(" %i bytes of data moved\n",sizeof(ipc_dat));
//    printf("update forced, flag cleared\n");
//  }
//  memcpy(&ipc_dat,data,sizeof(ipc_dat));    // move shared memory data to local structure
// }

