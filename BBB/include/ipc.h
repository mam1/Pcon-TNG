#ifndef IPC_H_
#define IPC_H_

#include "shared.h"
#include "typedefs.h"

/* ipc version info */
#define _MAJOR_VERSION_ipc    10
#define _MINOR_VERSION_ipc   4
#define _MINOR_REVISION_ipc   0

/* ipc fuctions */
 int 	ipc_open(char *,int);
 void 	*ipc_map(int, int);
 void 	ipc_close(int, void *, int);
 int 	ipc_size(void);
 int 	ipc_sem_init(void);
 int ipc_sem_lock(int semid, SEMBUF *sb);
 int ipc_sem_free(int semid, SEMBUF *sb);
 int ipc_sem_id(int skey);

#endif