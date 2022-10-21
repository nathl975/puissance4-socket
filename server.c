#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <netdb.h>
#include <string.h>

#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;


int main(int argc, char const *argv[])
{
    int 	    socket_descriptor, 		    /* descripteur de socket */
			    nouv_socket_descriptor,     /* [nouveau] descripteur de socket */
			    longueur_adresse_courante;  /* longueur d'adresse courante d'un client */
    sockaddr_in server_info, 		        /* structure d'adresse locale*/
			    client_info;                /* adresse client courant */
    hostent*	ptr_hote; 			        /* les infos recuperees sur la machine hote */
    servent*	ptr_service; 			    /* les infos recuperees sur le service de la machine */
    char 	    machine[TAILLE_MAX_NOM+1];  /* nom de la machine locale */
    
    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
        return -1;
    }
    
    /* initialisation de la structure server_info avec les infos recuperees */			
    
    /* copie de ptr_hote vers server_info */
    bcopy((char*)ptr_hote->h_addr, (char*)&server_info.sin_addr, ptr_hote->h_length);
    server_info.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    server_info.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */
    
    /* utiliser un nouveau numero de port */
    server_info.sin_port = htons(5000);
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(server_info.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
        return -1;
    }

    /* association du socket socket_descriptor à la structure d'adresse server_info */
    if ((bind(socket_descriptor, (sockaddr*)(&server_info), sizeof(server_info))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		return -1;
    }
    
    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);
    // socket info

    // create socket

    // bind

    // listen

    // server
    return 0;
}
