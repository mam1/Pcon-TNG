#ifndef SYS_DAT_H_
#define SYS_DAT_H_


FILE *sys_open(char *fname,_SYS_DAT2 *sdat);
void sys_load(char *fname,_SYS_DAT2 *sdat);
void sys_save(char *fname,_SYS_DAT2 *sdat);




#endif /* SYS_DAT_H_ */