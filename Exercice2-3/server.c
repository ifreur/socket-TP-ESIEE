#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>


int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Incorrect number of input ..\n");
        exit(1);
    }

    char msg[2048];
    int socket1;
    int port_rec = atoi(argv[1]);


    socket1 = socket(AF_INET,SOCK_STREAM,0);
    int opt = 1;
    setsockopt(socket1,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));
    if(socket1 == 1){
        printf("Error on socket : End of prgm %s",strerror(errno));
        exit(1);
    }

    struct sockaddr_in ad1;
    ad1.sin_family = AF_INET;
    ad1.sin_port = htons(port_rec);
    ad1.sin_addr.s_addr = INADDR_ANY;
        
    int res = bind(socket1,(struct sockaddr*) &ad1,sizeof(ad1));
    if(res != 0){
        printf("Problem on bind : %s\n",strerror(errno));
        exit(1);
    }

    listen(socket1,1);

    struct sockaddr_in ad2;
    ad2.sin_family = AF_INET;
    ad2.sin_port = 0; 
    ad1.sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(ad2);

    int acc = accept(socket1, (struct sockaddr*) &ad2,&len);

    read(acc,msg,sizeof(msg));
    printf("%s",msg);

    close(socket1);

    return 0;
}