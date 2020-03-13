#ifndef WATCHER_H_
#define WATCHER_H_

/* Dcon version info */
#define _MAJOR_VERSION_watcher-d		0
#define _MINOR_VERSION_watcher-d 		0
#define _MINOR_REVISION_watcher-d		0

#define ADDRESS     "tcp://192.168.254.221:1883"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "258Thomas/#"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000

#include "shared.h"


int count(void);
int dispevars(void);

void logit(char *);
void delivered(void *, MQTTClient_deliveryToken);
int msgarrvd(void *, char *, int, MQTTClient_message *);
void connlost(void *, char *);


#endif /* WATCHER_H_ */