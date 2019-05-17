// Wiktor Garbarek 291963

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>

#include "utils.h"

void create_path(char *dst, struct request *request, char *directory){
    char *port_pos = strchr(request->host, ':'); 
    if (port_pos != NULL){
        *port_pos = '\0';
    }
    if (directory[strlen(directory) - 1] == '/'){
        directory[strlen(directory) - 1] = '\0';
    }

    printf("File under directory >%s/%s%s< will be checked\n", directory, request->host, request->endpoint);
    sprintf(dst, "%s/%s%s", directory, request->host, request->endpoint);
}

int check_path(char *path){
    if (strstr(path, "..") != NULL) return FORBIDDEN;
    struct stat checker;
    if (stat(path, &checker) == -1) return NOT_FOUND;
    if (S_ISDIR(checker.st_mode)) return MOVED;
    return OK; 
    
}

char *get_content_type(char *dir){
    char *file_extension = strrchr(dir, '.'); // strrchr returns last occurence of '.'
    if (strcmp(file_extension, ".txt" ) == 0) return "text/plain";
    if (strcmp(file_extension, ".html") == 0) return "text/html";
    if (strcmp(file_extension, ".css" ) == 0) return "text/css";
    if (strcmp(file_extension, ".jpg" ) == 0) return "image/jpg";
    if (strcmp(file_extension, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(file_extension, ".png" ) == 0) return "image/png";
    if (strcmp(file_extension, ".pdf" ) == 0) return "application/pdf";
    return "application/octet-stream";
}

void prepare_response(struct response *response, struct request *request, char *directory){
    if (!strlen(request->endpoint) || !strlen(request->host) || !strlen(request->connection)){
        response->code = NOT_IMPLEMENTED;
        return;
    }
    char full_path[4096];
    create_path(full_path, request, directory);
    int code = check_path(full_path);
    if (code == MOVED){
        strcat(full_path, (full_path[strlen(full_path) - 1] == '/') ? "index.html" : "/index.html");
        code = check_path(full_path);
        code = (code == 200)? 301: code;
    }   
    strcpy(response->full_path, full_path);
    response->code = code;
}
void read_file_to_buffer(char *buffer, char* file_path, size_t file_size){
    int f = open(file_path, O_RDONLY);
    if (read(f, buffer, file_size) < 0){
        handle_error("read");
    }
    close(f);
}

void respond(struct response *response, int conn_sockfd){
    char *response_buffer;
    struct stat file;
    size_t size;
    if (response->code == OK || response->code == MOVED){
        stat(response->full_path, &file);
        response_buffer = malloc(128 + (size_t) file.st_size);
        if (response->code == OK){
            sprintf(response_buffer, RESPONSE_HEADER, 
                    "200 OK", (size_t) file.st_size, get_content_type(response->full_path));
        }
        if (response->code == MOVED){
           sprintf(response_buffer, RESPONSE_HEADER, 
                   "301 Moved Permanently", (size_t) file.st_size, get_content_type(response->full_path));
        }
        size = strlen(response_buffer);
        
        read_file_to_buffer(response_buffer + strlen(response_buffer), response->full_path, (size_t) file.st_size); 
        size += (size_t) file.st_size;
    } else {
        response_buffer = malloc(512);
        if (response->code == FORBIDDEN){
            sprintf(response_buffer, RESPONSE_HEADER FORBIDDEN_SITE, 
                    "403 Forbidden", strlen(FORBIDDEN_SITE), "text/html");
        }
        if (response->code == NOT_FOUND){
            sprintf(response_buffer, RESPONSE_HEADER NOT_FOUND_SITE, 
                    "404 Not Found", strlen(NOT_FOUND_SITE), "text/html");
        }
        if (response->code == NOT_IMPLEMENTED){
            sprintf(response_buffer, RESPONSE_HEADER NOT_IMPL_SITE, 
                    "501 Not Implemented", strlen(NOT_IMPL_SITE), "text/html");
        }
        size = strlen(response_buffer);
    }
    if (send(conn_sockfd, response_buffer, size, 0) < 0){
        free(response_buffer);
        handle_error("send");
    }
    free(response_buffer);
}
