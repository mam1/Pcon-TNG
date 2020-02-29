#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "build_token_q.h"


void main(int argc, char* argv[])
{
	char 			topic[] = "258Thomas/shop/sensor";

	build_token_q(topic, (int)strlen(topic));

	return;

	}