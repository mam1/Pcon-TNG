#ifndef MAIN_H_
#define MAIN_H_

#define VERSION			"99" 			//version number

#define STACK_A			30				//stack size for cog A
#define STACK_B			30				//stack size for cog B
#define STACK_C			30				//stack size for cog C

#define INPUT_BUFFER	128				//input buffer size
#define SERIAL_BUFFER	128
#define COMMANDS		11				//number of commands 

/* control block data structure */
typedef volatile struct 
{
    int		query_flag;	//!= 0, requests a counter update
    int     query_ctr;	//number of times cog has been queried 
    int		cog;		//number of the cog the code is runing on  	
} CONTROL_BLOCK;

/* BBBmon control block data structure */
typedef volatile struct
{
    int		query_flag;	//!= 0, requests a counter update
    int     query_ctr;	//number of times cog has been queried
    int		cog;		//number of the cog the code is runing on
} CONTROL_BLOCK_BBBmon;

#endif
