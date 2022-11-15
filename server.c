#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

// initialise un tableau[6][7] avec des ' '
void initGrid(char grid[][7])
{
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            grid[i][j] = ' ';
        }
    }
}

// vérifie si le joueur a gagné
int hasWon(char grid[6][7])
{
    if (grid[0][0] != ' ' && grid[0][1] != ' ' &&
        grid[0][2] != ' ' && grid[0][3] != ' ' &&
        grid[0][4] != ' ' && grid[0][5] != ' ' &&
        grid[0][6] != ' ')
        {
            printf("\nLe plateau est plein, il y a égalité\n");
            return -1;
        }
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            if (grid[i][j] == grid[i][j + 1] &&
                grid[i][j] == grid[i][j + 2] &&
                grid[i][j] == grid[i][j + 3] &&
                grid[i][j] != ' ') // Vérifie en ligne
                return 1;

            else if (grid[i][j] == grid[i + 1][j] &&
                     grid[i][j] == grid[i + 2][j] &&
                     grid[i][j] == grid[i + 3][j] &&
                     grid[i][j] != ' ') // vérifie en colonne
                return 1;

            else if (grid[i][j] == grid[i + 1][j + 1] &&
                     grid[i][j] == grid[i + 2][j + 2] &&
                     grid[i][j] == grid[i + 3][j + 3] &&
                     grid[i][j] != ' ') // vérifie en diagonale droite
                return 1;

            else if (grid[i][j] == grid[i + 1][j - 1] &&
                     grid[i][j] == grid[i + 2][j - 2] &&
                     grid[i][j] == grid[i + 3][j - 3] &&
                     grid[i][j] != ' ') // vérifie en diagonale gauche
                return 1;
        }
    }
    return 0;
}

void startGame() {

}

void clientAwait() {

}

void renvoi (int sock) {

    char buffer[256];
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0)
    	return;
    
    printf("message lu : %s \n", buffer);
    
    buffer[0] = 'R';
    buffer[1] = 'E';
    buffer[longueur] = '#';
    buffer[longueur+1] ='\0';
    
    printf("message apres traitement : %s \n", buffer);
    
    printf("renvoi du message traite.\n");

    /* mise en attente du prgramme pour simuler un delai de transmission */
    sleep(3);
    
    write(sock,buffer,strlen(buffer)+1);
    
    printf("message envoye. \n");
        
    return;
    
}

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
    
    // server_info
    /* copie de ptr_hote vers server_info */
    bcopy((char*)ptr_hote->h_addr, (char*)&server_info.sin_addr, ptr_hote->h_length);
    server_info.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    server_info.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */
    
    /* utiliser un nouveau numero de port */
    server_info.sin_port = htons(5000);
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(server_info.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    // socket
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
        return -1;
    }

    // bind
    /* association du socket socket_descriptor à la structure d'adresse server_info */
    if ((bind(socket_descriptor, (sockaddr*)(&server_info), sizeof(server_info))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		return -1;
    }
    
    // listen
    /* initialisation de la file d'écoute */
    listen(socket_descriptor,5);

    // traitement serveur
    /* attente des connexions et traitement des donnees recues */

    char grid[6][7];
    initGrid(grid);

    int clients[2];
    int cptClient = 0;

    while(!hasWon(grid) || hasWon(grid) == -1) {
    
		longueur_adresse_courante = sizeof(client_info);
		
		/* adresse_client_courant sera renseigné par accept via les infos du connect */
		if ((nouv_socket_descriptor = 
			accept(socket_descriptor, 
			       (sockaddr*)(&client_info),
			       &longueur_adresse_courante))
			 < 0) {
			perror("erreur : impossible d'accepter la connexion avec le client.");
			exit(1);
		} else {
            cptClient += 1;
        }

        if(cptClient == 2) {
            startGame();
        } else {
            clientAwait();
        }
		
		/* traitement du message */
		printf("reception d'un message.\n");
		
		renvoi(nouv_socket_descriptor);
    }
    close(nouv_socket_descriptor);

    return 0;
}
