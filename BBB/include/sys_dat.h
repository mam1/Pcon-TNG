#ifndef SYS_DAT_H_
#define SYS_DAT_H_

#include "typedefs.h"


FILE *sys_open(char *fname,_SYS_DAT2 *sdat);
void sys_load(FILE *f,_SYS_DAT2 *sdat);
int sys_save(FILE *f,_SYS_DAT2 *sdat);
int sys_comp(_SYS_DAT2 *sdat, _CONFIG_DAT *config);
void sys_disp(_SYS_DAT2 *s);


#endif /* SYS_DAT_H_ */