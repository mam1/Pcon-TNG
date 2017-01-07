#ifndef SYS_DAT_H_
#define SYS_DAT_H_

#include "typedefs.h"

/* system version */
#define _MAJOR_VERSION_system	10
#define _MINOR_VERSION_system	4
#define _MINOR_REVISION_system	0

FILE *sys_open(char *fname,_SYS_DAT *sdat);
void sys_load(FILE *f,_SYS_DAT *sdat);
int sys_save(FILE *f,_SYS_DAT *sdat);
int sys_comp( _CONFIG_DAT *config);
void sys_disp(_SYS_DAT *s);


#endif /* SYS_DAT_H_ */