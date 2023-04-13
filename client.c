/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

#define HOST "localhost"

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

void writeSocket(int sd, char * message) {
    if ((write(sd, message, strlen(message))) < 0) {
        perror("erreur : impossible d'écrire le message destine au serveur.");
        exit(1);
    }
}

void readSocket(int sd, char * buffer) {
    int longueur;
    if((longueur = read(sd, buffer, sizeof(buffer))) > 0) {
        //printf("long : %d", longueur);
        printf("réponse du serveur : \n");
        write(STDOUT_FILENO,buffer,longueur); // écriture dans la sortie standard
    } else {
        write(STDOUT_FILENO, "erreur", 7);
    }
}

int main(int argc, char **argv) {
  
    int 	    socket_descriptor, 	/* descripteur de socket */
                longueur; 		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    servent *	ptr_service; 		/* info sur service */
    char 	    buffer[256];
    char        mesg[80]; 			/* message envoyé */
     
    /*if (argc != 3) {
	perror("usage : client <adresse-serveur> <message-a-transmettre>");
	exit(1);
    }*/

    
    if ((ptr_host = gethostbyname(HOST)) == NULL) {
	perror("erreur : impossible de trouver le serveur a partir de son adresse.");
	exit(1);
    }
    
    /* copie character par character des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    
    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
	perror("erreur : impossible de recuperer le numero de port du service desire.");
	exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    */
    /*-----------------------------------------------------------*/
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(PORT);
    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de créer la socket de connexion avec le serveur.");
        exit(1);
    }
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }
    
    printf("connexion établie avec le serveur. \n");
    
    //printf("envoi d'un message au serveur. \n");
      
    /* envoi du message vers le serveur */
    while (strcmp(mesg, "q")) {
        printf("Veuillez saisir un message Elouan : \n");
        scanf("%s", mesg);

        writeSocket(socket_descriptor, mesg);

        printf("message envoyé au serveur. \n");

        /* lecture de la réponse en provenance du serveur */
        readSocket(socket_descriptor, buffer);
    }
    
    printf("\nfin de la reception.\n");
    
    close(socket_descriptor);
    
    printf("connexion avec le serveur fermee, fin du programme.\n");
    
    exit(0);
    
}