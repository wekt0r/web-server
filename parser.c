// Wiktor Garbarek 291963

#include<stdio.h>
#include<string.h>

#include "utils.h"

int parse_request(struct request *dst, char *request){
    char delim[3] = " \n\r";
    char *str, *token;
    int flag = NOTHING;
    for(str=request;;str = NULL){
        token = strtok(str, delim);
        if (token == NULL) return -flag;

        switch (flag){
            case GET: strcpy(dst->endpoint, token); break;
            case HOST: strcpy(dst->host, token); break;
            case CONNECTION: strcpy(dst->connection, token); break;
        }
        
        flag = NOTHING;
        if(strcmp(token, "GET") == 0) flag = GET;
        if(strcmp(token, "Host:") == 0) flag = HOST;
        if(strcmp(token, "Connection:") == 0) flag = CONNECTION;
    }
}