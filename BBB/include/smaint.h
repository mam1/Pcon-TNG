#ifndef SMAINT_H_
#define SMAINT_

int s_load(int id, _CMD_FSM_CB *cb);
int s_sort(_SEN_DAT *f);
int s_save(_SEN_DAT *f[_NUMBER_OF_SENSORS], _CMD_FSM_CB *cb);





#endif /* SMAINT_H_ */