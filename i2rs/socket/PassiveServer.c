#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define LISTEN_SIZE 20
#define PORT 8888

int start_server(int port, int type){
    int ss = socket(AF_INET, type, 0);
    if(ss < 0){
        printf("create socket error\n");
        return -1;
    }

    //struct sockaddr_in server_addr;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
 
    if(bind(ss, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("bind error\n");
        return -1;
    }
    //TCP
    if(SOCK_STREAM == type){

        if(listen(ss, LISTEN_SIZE) < 0){
            printf("listen error\n");
            return -1;
        }
        printf("tcp server start\n");
    }
    else
        printf("udp server start\n");
    return ss;
}

int create_tcp_server(int port){
    start_server(port, SOCK_STREAM);
}

int create_udp_server(int port){
    start_server(port, SOCK_DGRAM);
}
