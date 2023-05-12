#include <stdio.h>
#include <wiringPi.h>
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"
#include <stdint.h>
#include <stdlib.h>
#define LED_red_1 23//red
#define LED_yellow_1 24 //yellow
#define LED_green_1 25//green

#define LED_red_2 27//red
#define LED_yellow_2 28 //yellow
#define LED_green_2 29//green
int countStation_1=0;
int countStation_2=0;
int countSystem=0;
int stateStation_1=-1;
int stateStation_2=-1;
int timeless_1;
int timeless_2;
int temp=0;
int temp_2=0;
int color=0;
int color_2=0;
int timeLed[2][3]={{2,2,2},//current station 1   RYG
                  {2,2,2}};//current station 2   RYG

__uint8_t led[2][3]={{LED_red_1,LED_yellow_1,LED_green_1},
                    {LED_red_2,LED_yellow_2,LED_green_2}};
#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define CLIENTID    "subscriber_demo"
#define SUB_TOPIC   "rx/20146376"
#define PUB_TOPIC   "tx/20146376"
#define QOS         2
void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 2;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    //printf("Message '%s' with delivery token %d delivered\n", payload, token);
}

int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    printf("Received message: %s\n", payload);
    //receive update config
    int station,red,yellow,green;
    printf("scan-+-+-+-+-+-\n");
    sscanf(payload,"Update config: Station %d: Red [ %d ] || Yellow [ %d ] || Green [ %d ]",&station,&red,&yellow,&green);
    printf("done\n");
    if(station==1)
    {
        timeLed[0][0]=red;
        timeLed[0][1]=yellow;
        timeLed[0][2]=green;
    }
    if(station==2)
    {
        timeLed[1][0]=red;
        timeLed[1][1]=yellow;
        timeLed[1][2]=green;
    }
    printf("update\n");
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void noStation(void)
{
    printf("Please choose station\n"); 
    fflush(stdout);   
}
int main()
{
    wiringPiSetup () ; // note the setup method chosen
    //pin mode
    for (int i = 0; i < 2; i++)
    {   
        for(int j=0;j<3;j++)
        {
            //define IO
            pinMode(led[i][j],OUTPUT);
            //set led off
            digitalWrite(led[i][j],0);
        }
    }
    
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    //listen for operation
    MQTTClient_subscribe(client, SUB_TOPIC, 2);
    
   int count=0;
   printf("Start\n");
   stateStation_1=0;
   stateStation_2=0;
    while(1) 
    {  

        if(stateStation_1!=-1)
        {
            color=stateStation_1;
            timeless_1=timeLed[0][color];
            
            if(timeless_1-temp>0)//xu li delay
            {  
                digitalWrite(led[0][color],1);
                if(countStation_1%2000==0)
                {
                    temp+=1;
                    //printf("\nS1 Timeless: %d seconds || Color: %d\n",timeless_1-temp,color);
    
                }
                if(temp==timeless_1)
                {
                    timeless_1=0;
                }
            }
            else{

                digitalWrite(led[0][color],0); 
                temp=0;
                switch (stateStation_1)
                {
                case 0:
                    stateStation_1=1;
                    break;
                case 1:
                    stateStation_1=2;
                    break;
                case 2:
                    stateStation_1=0;
                    break; 
                default:
                    break;
                }

            }
            
        }
        //STATE STATION 2
        if(stateStation_2!=-1)
        {
            color_2=stateStation_2;
            timeless_2=timeLed[1][color_2];
            
            if(timeless_2-temp_2>0)//xu li delay
            {  
                digitalWrite(led[1][color_2],1);
                if(countStation_2%2000==0)
                {
                    temp_2+=1;
                    //printf("\nS2 Timeless: %d seconds || Color: %d\n",timeless_2-temp_2,color_2);
    
                }
                if(temp_2==timeless_2)
                {
                    timeless_2=0;
                }
            }
            else{

                digitalWrite(led[1][color_2],0); 
                temp_2=0;
                switch (stateStation_2)
                {
                case 0:
                    stateStation_2=1;
                    break;
                case 1:
                    stateStation_2=2;
                    break;
                case 2:
                    stateStation_2=0;
                    break; 
                default:
                    break;
                }

            }
            
        }

        if(countSystem%2000==0)
        {
            //send to sever
            count++;
           
            int time_1=timeless_1-temp;
            int time_2=timeless_2-temp_2;
            if(time_1<0)
            {
                time_1=0;
            }
            if(time_2<0)
            {
                time_2=0;
            }
            printf("|___________________||____________________||__________|\n");
            printf("|Station 1: %d [ %d ] || Station 2: %d [ %d ] || Times: %d||\n",stateStation_1,time_1,stateStation_2,time_2,count);
            //sendata to sever-> database
            char msg[40];
            sprintf(msg,"Return Data { Station 1: LED[ %d ][ %d ] || Station 2: LED[ %d ][ %d ] }",stateStation_1,time_1+1,stateStation_2,time_2+1);
            publish(client,PUB_TOPIC,msg);
        }     
        countSystem++;
        countStation_1++;
        countStation_2++;
        delay(1);

                    

    }
    return 0;

}

