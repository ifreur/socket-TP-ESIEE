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

    if(argc != 4){
        printf("Incorrect number of input ..\n");
        exit(1);
    }

    char  dns_name[256] ;
    strcpy(dns_name,argv[1]);
    int port_dest = atoi(argv[2]);
    char msg[50];
    strcpy(msg,argv[3]);


    int pid = getpid();
    int pid_ext;
    int socket1;



    socket1 = socket(AF_INET,SOCK_DGRAM,0);
    if(socket1 == 1){
        printf("Error on socket : End of prgm %s",strerror(errno));
        exit(1);
    }

    struct sockaddr_in ad1;
    ad1.sin_family = AF_INET;
    ad1.sin_port = 0;
    ad1.sin_addr.s_addr = INADDR_ANY;

    int res = bind(socket1,(struct sockaddr*) &ad1,sizeof(ad1));
    if(res != 0){
        printf("Problem on bind : %s\n",strerror(errno));
        exit(1);
    }

    struct hostent *hp = gethostbyname(dns_name);

    struct sockaddr_in ad2;
    ad2.sin_family = AF_INET;
    ad2.sin_port = htons(port_dest);
    memcpy(&ad2.sin_addr.s_addr,hp->h_addr_list[0],hp->h_length);
    socklen_t len = sizeof(ad2);
    
    sendto(socket1,&pid,sizeof(pid),0,(struct sockaddr*) &ad2,sizeof(ad2));
    sendto(socket1,&msg,sizeof(msg),0,(struct sockaddr*) &ad2,sizeof(ad2));

    recvfrom(socket1,&pid_ext,sizeof(pid_ext),0,(struct sockaddr*) &ad2, &len  );
    recvfrom(socket1,&msg,sizeof(msg),0,(struct sockaddr*) &ad2, &len  );

    printf("PID : %d \n",pid_ext);    
    printf("Message : %s \n",msg);    


    close(socket1);

    return 0;
}