#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char *argv[]){

    int socket1;
    int port_dest = atoi(argv[1]);
    
    char  dns_name[256] ;
    memcpy(dns_name,argv[2], sizeof(dns_name));

    char page[100] ;
    memcpy(page,argv[3], sizeof(page));

    char request[1024];
    char *request_patern = "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n";
    sprintf(request,request_patern,page,dns_name);

    socket1 = socket(AF_INET,SOCK_STREAM,0);
    if(socket1 == 1){
        printf("Error on socket : End of prgm %s",strerror(errno));
        exit(1);
    }
    struct hostent *hp = gethostbyname(dns_name);

    struct sockaddr_in ad2;
    ad2.sin_family = AF_INET;
    ad2.sin_port = htons(port_dest);
    memcpy(&ad2.sin_addr.s_addr,hp->h_addr_list[0],hp->h_length);

    connect(socket1,(struct sockaddr*) &ad2,sizeof(ad2));

    int check = strlen(request);
    int tmp;

    tmp = write(socket1,request,check);
    if(tmp < 0)
        exit(-1);

    char response[2000];
    read(socket1,response,sizeof(response));

    printf("%s\n",response);

    close(socket1);

    return 0;
}