
int parce(* char message){

	    int i;
    char* payloadptr;

payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }

}


