
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#define ADDRESS     "tcp://192.168.254.221:1883"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
volatile MQTTClient_deliveryToken deliveredtoken;
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);
    do
    {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}


// #include <stdio.h>
// #include <stdlib.h>
// #include <signal.h>
// #include <memory.h>
// #include <sys/time.h>

// #include "MQTTClient.h"

// volatile int toStop = 0;

// void usage(char** argv)
// {
//     printf("Usage: %s topic channel_token\n", argv[0]);
//     exit(-1);
// }

// void cfinish(int sig)
// {
//     signal(SIGINT, NULL);
//     toStop = 1;
// }

// void messageArrived(MessageData* md)
// {
//     MQTTMessage* message = md->message;

//     printf("%.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
//     printf("%.*s\n", (int)message->payloadlen, (char*)message->payload);
// }


// int main(int argc, char** argv)
// {
//     int rc = 0;
//     unsigned char buf[100];
//     unsigned char readbuf[100];

//     if (argc < 3)
//         usage(argv);

//     char* host = "192.168.254.221";
//     int port = 1883;
//     char* topic = argv[1];
//     char* clientid = "myid";
//     char* username = argv[2];

//     Network n;
//     Client c;

//     signal(SIGINT, cfinish);
//     signal(SIGTERM, cfinish);

//     NewNetwork(&n);
//     ConnectNetwork(&n, host, port);
//     MQTTClient(&c, &n, 1000, buf, 100, readbuf, 100);

//     MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
//     data.willFlag = 0;
//     data.MQTTVersion = 3;
//     data.clientID.cstring = clientid;
//     data.username.cstring = username;
//     //data.password.cstring = opts.password;

//     data.keepAliveInterval = 10;
//     data.cleansession = 1;
//     printf("Connecting to %s %d\n", host, port);

//     rc = MQTTConnect(&c, &data);
//     printf("Connected %d\n", rc);

//     printf("Subscribing to %s\n", topic);
//     rc = MQTTSubscribe(&c, topic, QOS1, messageArrived);
//     printf("Subscribed %d\n", rc);

//     while (!toStop)
//     {
//         MQTTYield(&c, 1000);
//     }

//     printf("Stopping\n");

//     MQTTDisconnect(&c);
//     n.disconnect(&n);

//     return 0;
// }