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

int ipc_open(char *fname) {
    int         fd;
    struct stat sb;
    // mode_t       mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = open(fname, O_RDWR | O_CREAT);
    if (fd == -1)
        handle_error("open");

    if (fstat(fd, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");

    printf("  ipc_open: file descriptor %i returned from open\n", fd);
    return fd;
}

void *ipc_map(int fd, int size) {
    void        *data;
    int         pa_offset;

    pa_offset = 0 & ~(sysconf(_SC_PAGE_SIZE) - 1);   // offset for mmap() must be page aligned
    printf("page offset %i\n", pa_offset);

    data = mmap((caddr_t)0, size + pa_offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, pa_offset);
    if (data == MAP_FAILED)
        // if((caddr_t)data == (caddr_t)(-1))
        handle_error("mmap");

    printf("  ipc_map: file mapped to memory\n");
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
