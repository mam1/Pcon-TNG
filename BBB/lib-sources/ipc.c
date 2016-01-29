/************************************************************************/
/*                                                                      */
/*  ipc.c - support interprocess communication                          */
/*                                                                      */
/************************************************************************/

#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>    //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include "ipc.h"

int ipc_open(char *fname, int size) {
    int                 fd;
    struct stat         sb;
    mode_t              mode = S_IRWXO | S_IRWXG | S_IRWXU;
    int                 result;

    fd = open(fname, O_RDWR | O_CREAT, mode);
    if (fd == -1)
        handle_error("open");
    if (fstat(fd, &sb) == -1)           
        handle_error("fstat");
    if (!S_ISREG (sb.st_mode)) {
        fprintf (stderr, "%s is not a file\n", fname);
        exit(1);
    }
    printf("    file size from stat() = %i\n",(int)sb.st_size ); 
    printf("    file block size from stat() = %i\n",(int)sb.st_blksize );
    printf("    file blocks from stat() = %i\n",(int)sb.st_blocks );   
    printf("  ipc_open: file descriptor %i returned from open\n", fd);

    /* Stretch the file size to the size of the (mmapped) array of ints
     */
    result = lseek(fd, size-1, SEEK_SET);
    if (result == -1) {
    close(fd);
    perror("Error calling lseek() to 'stretch' the file");
    exit(EXIT_FAILURE);
    }

    result = write(fd, "", 1);
    if (result != 1) {
    close(fd);
    perror("Error writing last byte of the file");
    exit(EXIT_FAILURE);
    }
    if (fstat(fd, &sb) == -1)           
        handle_error("fstat");
    if (!S_ISREG (sb.st_mode)) {
        fprintf (stderr, "%s is not a file\n", fname);
        exit(1);
    }
    printf("    file size from stat() = %i\n",(int)sb.st_size ); 
    printf("    file block size from stat() = %i\n",(int)sb.st_blksize );
    printf("    file blocks from stat() = %i\n",(int)sb.st_blocks );   
    printf("  ipc_open: file descriptor %i returned from open\n", fd);
    

    return fd;
}

void *ipc_map(int fd, int size) {
    void        *data;
    struct stat         sb;

    printf("    ipc_map: called with fd=%i, size=%i\n",fd,size);

    data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED)
        handle_error("mmap");

    printf("    ipc_map: file mapped to memory\n");

                        if (fstat(fd, &sb) == -1)           
                            handle_error("fstat2");
                        if (!S_ISREG (sb.st_mode)) {
                            fprintf (stderr, "is not a file2\n");
                            exit(1);
                        }
                        printf("    file size from stat() = %i\n",(int)sb.st_size ); 
                        printf("    file block size from stat() = %i\n",(int)sb.st_blksize );
                        printf("    file blocks from stat() = %i\n",(int)sb.st_blocks );   
                        printf("  ipc_open: file descriptor %i returned from open\n", fd);

    printf("    try and refference shared memory\n");

    memset(data,0,size);

    printf("     **********************\n");
    printf("     **********************\n");
    printf("     **********************\n");
    printf("     **********************\n");
    printf("     **********************\n");

    return data;
}

void ipc_close(int fd, void *data, int size) {
    if (munmap(data, size) == -1)
        handle_error("munmap");

    close(fd);
    printf("  ipc_close: file unmapped\n");
    return;
}

int ipc_size(void) {

    long        page_size = sysconf (_SC_PAGESIZE);
    int         pages;
    // printf("    ipc data size %i\n",sizeof(IPC_DAT));
    pages = sizeof(IPC_DAT) / page_size;
    // printf("    1 -pages = %i\n",pages);
    if (pages < 1) pages = 1;
    else if (sizeof(IPC_DAT) % page_size != 0) pages += 1;
    // printf("    2 -pages = %i\n",pages);
    printf("  ipc_size: system page size %i, ipc data %i, ipc data requires %i pages, %i bytes\n", (int)page_size, sizeof(IPC_DAT), pages, pages * (int)page_size);
    return (pages * (int)page_size);
}
