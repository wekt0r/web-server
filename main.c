// Wiktor Garbarek 291963

#define _DEFAULT_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

#include "utils.h"

void parse_args(int argc, char *argv[], uint16_t *port, char directory[][4096]){
    if (argc != 3){
        printf("Some arguments are missing - usage %s PORT DIRECTORY\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    *port = atoi(argv[PORT_I]);
    
    struct stat checker;
    if (stat(argv[DIRECTORY_I], &checker) == -1 || !S_ISDIR(checker.st_mode)){
        printf("%s does not exist or is not a directory\n", argv[DIRECTORY_I]);
        exit(EXIT_FAILURE);
    }

    if (sprintf((char *) directory, "%s", argv[DIRECTORY_I]) <= 0){
        printf("Invalid directory name\n");
        exit(EXIT_FAILURE);
    }
}

int init_socket(uint16_t port){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        handle_error("socket");
    }

    struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port); 
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind (sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		handle_error("bind");
	}
    
    if (listen(sockfd, 64) < 0){
        handle_error("listen");
    }

    return sockfd;
}
int accept_connection(int sockfd){
    struct sockaddr_in client_address;
    socklen_t len = sizeof(client_address);
    int conn_sockfd = accept(sockfd, (struct sockaddr *) &client_address, &len);
    if (conn_sockfd < 0){ 
        handle_error("accept");
    }
    char ip_address[20];
    inet_ntop (AF_INET, &client_address.sin_addr, ip_address, sizeof(ip_address));
    printf("New client %s:%d\n", ip_address, ntohs(client_address.sin_port));
    return conn_sockfd;
}

void set_timeout(struct timeval *timeout){
    timeout->tv_sec = 1;
    timeout->tv_usec = 0;
}

int is_still_alive(char *connection, struct timeval *timeout){
    if(strcmp(connection, "close") == 0){
        return 0;
    } else {
        set_timeout(timeout);
        return 1;
    }
}
int select_wrapper(int conn_sockfd, struct timeval *timeout){
    fd_set descriptors;
	FD_ZERO (&descriptors);
	FD_SET (conn_sockfd, &descriptors);
    int keep_alive = select(conn_sockfd+1, &descriptors, NULL, NULL, timeout);
    if (keep_alive < 0){
	    handle_error("select");
    }
    return keep_alive;
}

static int sockfd;

void close_socket(){
    printf("Stopping server\n");
    if (close(sockfd) < 0){
            handle_error("close");
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]){
    uint16_t port;
    char directory[4096];
    
    parse_args(argc, argv, &port, &directory);
    
    sockfd = init_socket(port);
    
    struct sigaction act;
    act.sa_handler = close_socket;
    sigaction(SIGINT, &act, NULL);

    for(;;){
        int conn_sockfd = accept_connection(sockfd);
        struct timeval timeout;
        set_timeout(&timeout);
        
        int keep_alive = 1;
        while(keep_alive){
            keep_alive = select_wrapper(conn_sockfd, &timeout);
            if (keep_alive == 0){
                break;
            }
            char buffer[IP_MAXPACKET];
            bzero(buffer, IP_MAXPACKET);
            int bytes_received = recv(conn_sockfd, buffer, IP_MAXPACKET, MSG_DONTWAIT);
            if(bytes_received > 0){
                struct request request;
                bzero(&request, sizeof(request));
                parse_request(&request, buffer);
                printf("Parsed request:\n\t%s\n\t%s\n\t%s\n", request.endpoint, request.host, request.connection);
                                
                struct response response;
                prepare_response(&response, &request, directory);
                respond(&response, conn_sockfd);
                printf("Responded with code %d\n", response.code);
                keep_alive = is_still_alive(request.connection, &timeout);
            }

            if (bytes_received == 0){
                keep_alive = 0;
            } else if (bytes_received < 0 && errno != EWOULDBLOCK){
                handle_error("recv");
            }

        }
        if (close(conn_sockfd) < 0){
            handle_error("close");
        }
    }
    close_socket();
    return 0;
}  
