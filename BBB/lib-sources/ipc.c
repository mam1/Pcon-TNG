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
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>     //sleep
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.
#include <sys/ipc.h>
#include <sys/sem.h>

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

    if (sb.st_size < size) {
        /* Stretch the file size to the size of the (mmapped) buffer */
        result = lseek(fd, size - 1, SEEK_SET);
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
        close(fd);
        fd = open(fname, O_RDWR | O_CREAT, mode);
        if (fd == -1)
            handle_error("RDWR open");
        if (fstat(fd, &sb) == -1)
            handle_error("RDWR fstat");
        if (!S_ISREG (sb.st_mode)) {
            fprintf (stderr, "%s is not a file\n", fname);
            exit(1);
        }
        fd = open(fname, O_RDWR | O_CREAT, mode);
        if (fd == -1)
            handle_error("RDWR open");
        if (fstat(fd, &sb) == -1)
            handle_error("RDWR fstat");
        if (!S_ISREG (sb.st_mode)) {
            fprintf (stderr, "%s is not a file\n", fname);
            exit(1);
        }
    }

    return fd;
}

void *ipc_map(int fd, int size) {
    void        *data;
    struct stat         sb;

    data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
        handle_error("mmap");
    if (fstat(fd, &sb) == -1)
        handle_error("fstat2");
    if (!S_ISREG (sb.st_mode)) {
        fprintf (stderr, "is not a file2\n");
        exit(1);
    }
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

int ipc_init_sem() {
    
    int id; /* Number by which the semaphore is known within a program */

    /* The next thing is an argument to the semctl() function. Semctl()
       does various things to the semaphore depending on which arguments
       are passed. We will use it to make sure that the value of the
       semaphore is initially 0. */

    union semun {
        int val;
        struct semid_ds *buf;
        ushort * array;
    } argument;

    argument.val = 0;

    /* Create the semaphore with external key KEY if it doesn't already
       exists. Give permissions to the world. */

    id = semget(KEY, 1, 0666 | IPC_CREAT);

    /* Always check system returns. */

    if (id < 0)
    {
        fprintf(stderr, "Unable to obtain semaphore.\n");
        exit(0);
    }

    /* What we actually get is an array of semaphores. The second
       argument to semget() was the array dimension - in our case
       1. */

    /* Set the value of the number 0 semaphore in semaphore array
       # id to the value 0. */

    if ( semctl(id, 0, SETVAL, argument) < 0)
    {
        fprintf( stderr, "Cannot set semaphore value.\n");
    }
    else
    {
        fprintf(stderr, "Semaphore %d initialized.\n", KEY);
    }


    return 0;
}
