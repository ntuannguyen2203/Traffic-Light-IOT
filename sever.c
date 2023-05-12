#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"
#include <stdint.h>
#include <mariadb/mysql.h>
//data rev from web
int red;
int yellow;
int green;
int station;
//data receive from pi
int ledReturn_1,ledReturn_2,timeReturn_1,timeReturn_2;
/*
Broker: broker.emqx.io
TCP Port: 1883 
*/
#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define CLIENTID    "publisher_demo"
#define SUB_TOPIC   "tx/20146376"
#define PUB_TOPIC   "rx/20146376"
#define QOS         2
char *server = "localhost";
char *user = "nguyen";
char *password = "p"; /* set me first */
char *database = "iot";
MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;
void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 2;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Message '%s' with delivery token %d delivered\n", payload, token);
    
}

int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    printf("Received message: %s\n", payload);
    sscanf(payload,"Return Data { Station 1: LED[ %d ][ %d ] || Station 2: LED[ %d ][ %d ] }",&ledReturn_1,&timeReturn_1,&ledReturn_2,&timeReturn_2);
    // conn = mysql_init(NULL);
    // if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL) 
    // {
    //     fprintf(stderr, "%s\n", mysql_error(conn));
    //     mysql_close(conn);
    //     exit(1);
    // }  
    char sql[200];
    sprintf(sql,"update returnled set led1=%d,time1=%d,led2=%d,time2=%d",ledReturn_1+1,timeReturn_1,ledReturn_2+1,timeReturn_2);
    //printf("Updated\n");
    mysql_query(conn,sql);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
} 

int main(int argc, char* argv[]) {
   
     conn = mysql_init(NULL);
    mysql_real_connect(conn,server,user,password,database,0,NULL,0);
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL); 

    int rc;  // return code
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    //listen for operation
    MQTTClient_subscribe(client, SUB_TOPIC, 2);
    
    // kiem tra cot isUpdated
    char sql[200];
    while(1) 
    {
        sprintf(sql, "select * from iotled");
        mysql_query(conn,sql);
        res = mysql_store_result(conn); 
        row = mysql_fetch_row(res); //row[0]-> red; row[1]->green
        // NOTES: row la bien dang chuoi ky tu
        if(atoi(row[4])==1){
            char msg [200];
            sprintf(msg,"Update config: Station %d: Red [ %d ] || Yellow [ %d ] || Green [ %d ]",
                                        atoi(row[0]),atoi(row[1]),atoi(row[2]),atoi(row[3]));
            publish(client, PUB_TOPIC, msg);
            char reset[30];
            sprintf(reset,"update iotled set send=0");
            mysql_query(conn,reset);
            printf("reset\n");
        }

        sleep(1);
    }
    mysql_close(conn);
    return rc;
}