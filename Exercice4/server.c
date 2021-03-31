#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>

pid_t pid;

// Permet de fermer le fils et le père au moment d'un ctrl+c
void end_server(int foo){
    kill(pid,SIGKILL);
    exit(0);
}

// Permet d'écrire dans les logs.
void ecrire_log(char namefile[128],char ip[32]){
    FILE* logg = NULL;
    logg = fopen("log_file","a");
    time_t mytime;
    struct tm *local_time;
    time(&mytime);
    local_time = localtime(&mytime);
    fprintf(logg,"IP: %s Date :%d/%d/%d à %d:%d:%d Fichier : %s\n",ip,local_time->tm_mday,local_time->tm_mon+1,local_time->tm_year + 1900,local_time->tm_hour,local_time->tm_min,local_time->tm_sec,namefile);
    fclose(logg);
}

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Incorrect number of input ..\n");
        exit(1);
    }

    int socket1;

    int port_rec = atoi(argv[1]);
    int port_log = atoi(argv[2]);

    char request_body[500];

    char reply[2048];
    char reply_patern[77]="HTTP/1.1 %s\r\ncontent-length: %d\n\rcontent-type: text/html; charset UTF-8 \r\n%s";

    char defaultPage[32] = "index.html";
    char accepted[16] = "200 OK";
    char refused[16] = "400 Bad request";

    FILE* fichier = NULL;

    int res;
    
//Mise en double thread pour pouvoir gerer deux comportements sur deux ports.
pid = fork();

    //Création de socket.
    socket1 = socket(AF_INET,SOCK_STREAM,0);
    int opt = 1;
        //Gestion des erreurs pour réutiliser l'adresse en cas de coupure brusque.
    setsockopt(socket1,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int)); 
        //Gestion erreur. 
    if(socket1 == 1){
        printf("Error on socket : %s\n",strerror(errno));
        exit(1);
    }

    //Création adresse de reception. 
    struct sockaddr_in ad1;
    ad1.sin_family = AF_INET;
        //On distingue les deux cas selon notre pid (different depuis le fork).
    if(pid == 0)
        ad1.sin_port = htons(port_log);
    else
        ad1.sin_port = htons(port_rec);
    ad1.sin_addr.s_addr = INADDR_ANY;
    
    //Bind avec gestion d'erreur.
    res = bind(socket1,(struct sockaddr*) &ad1,sizeof(ad1));
    if(res != 0){
        printf("Problem on bind : %s\n",strerror(errno));
        exit(1);
    }

    // Création de l'adresse pour contenir la source.
    struct sockaddr_in ad2;
    ad2.sin_family = AF_INET;
    ad2.sin_port = 0; 
    ad1.sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(ad2);

while (1){

    //Ecoute du port.
    listen(socket1,1);

    //Acceptation du packet.
    int acc = accept(socket1, (struct sockaddr*) &ad2,&len);

    //Lecture du packet 
    read(acc,request_body,sizeof(request_body));

    char namefile[128];

    //Nous distinguons deux cas : 
    // 1) Le cas où nous sommes sur le port de reception et le format de requete est bon.
    // 2) Le cas où nous sommes sur le port de log (Format ignoré)
    if( sscanf(request_body,"GET /%s HTTP/",namefile) != EOF || pid == 0) {
        //Si nous sommes sur le port de log, nous renvoyons juste le fichier log_file
        if(pid == 0){
            fichier = fopen("log_file","r");
            memcpy(namefile,"log_file",sizeof(char)*9);
        }
        //Dans l'autre cas, nous cherchons à savoir le nom du fichier demandé lors de la requete.
        else{
            if (request_body[5] == ' '){
                namefile[0] = ' ';
                namefile[1] = '\0';
            }                          
            fichier = fopen(namefile,"r");
            if(fichier == NULL){
                printf("Fichier non trouve \n");
                fichier = fopen(defaultPage,"r");
                memcpy(namefile,defaultPage,sizeof(defaultPage));
            }
        }

        //récupération de l'adesse ip sous forme de string pour l'écrire dans les logs.
        char adresseip[32] ;
        inet_ntop(AF_INET,&(ad2.sin_addr),adresseip,32);
        ecrire_log(namefile,adresseip);
        
        //Création de la reponse sous format HTTP. 
        struct stat mystat;
        lstat(namefile,&mystat);
        char buff[(int) mystat.st_size];
            //Lecture du fichier.
        fread(buff,sizeof(char),sizeof(buff),fichier);
            //Création du body avec un patern prédefini


        sprintf(reply,reply_patern,accepted,strlen(buff),buff);
        fclose(fichier);
    }
    else {
    //Dans le cas où notre requete est mauvaise, on renvoie une erreur 400.
    sprintf(reply,reply_patern,refused,0,"");
    }
    //Encoire de notre reponse.
    write(acc,reply,sizeof(reply));

}
    //fermeture de notre socket. 
    close(socket1);

    return 0;
}