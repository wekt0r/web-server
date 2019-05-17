// Wiktor Garbarek 291963

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define RESPONSE_HEADER "HTTP/1.1 %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n"

// ARGS PARSER CONSTANTS
#define PORT_I 1
#define DIRECTORY_I 2

// REQUEST PARSER CONSTANTS
#define NOTHING 0
#define GET 1
#define HOST 2
#define CONNECTION 3

// HTTP CONSTANTS
#define OK 200
#define MOVED 301
#define FORBIDDEN 403
#define NOT_FOUND 404
#define NOT_IMPLEMENTED 501

#define FORBIDDEN_SITE "<center> <h1> 403 FORBIDDEN </h1> </center>"
#define NOT_FOUND_SITE "<center> <h1> 404 NOT FOUND </h1> </center>"
#define NOT_IMPL_SITE "<center> <h1> 501 NOT IMPLEMENTED </h1> </center>"


struct request {
    char endpoint[4096];
    char host[256]; 
    char connection[64];
};

struct response{
    int code;
    char full_path[4096];
};

int parse_request(struct request *dst, char *request);
void prepare_response(struct response *response, struct request *request, char *directory);
void respond(struct response *response, int conn_sockfd);