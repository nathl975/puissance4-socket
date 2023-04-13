#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"

#define TAILLE_MAX_NOM 256
#define PLAYER_ONE 0
#define PLAYER_TWO 1
#define YELLOW_PIECE 'J'
#define RED_PIECE 'R'
#define VICTOIRE 1
#define EGALITE 2

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

typedef struct {
    int     id;
    char    couleur;
    int     socket;
} Joueur;

typedef struct {
    int     id,
            joueurCourant;
    char    grille[6][7];
    Joueur  joueurs[2];
} Partie;

/* fonctions utilisées pour afficher les jetons de la bonne couleur */
void red()
{
    printf("\033[1;31m");
}

void yellow()
{
    printf("\033[1;33m");
}

/* affiche la grille de jeu */
void printGrille(char grille[][7])
{
    printf("\n-----------------------------\n");
    for (int i = 0; i < 6; i++)
    {
        printf("| ");
        for (int j = 0; j < 7; j++)
        {
            grille[i][j] == YELLOW_PIECE ? yellow() : red();
            printf("%c", grille[i][j]);
            printf("\033[0m"); // reset couleur
            printf(" | ");
        }
        printf("\n-----------------------------\n");
    }
}

/* affiche les informations des joueurs de la partie */
void printJoueurs(Partie *partie) {
    for (int i = 0 ; i < 2 ; i++) {
        printf("Joueur %d \n", i+1);
        printf("id : %d \n", partie->joueurs[i].id);
        printf("socket : %d \n", partie->joueurs[i].socket);
        printf("couleur : %c \n", partie->joueurs[i].couleur);
    }
}

// initialise un tableau[6][7] avec des ' '
void initialiserGrille(char grille[][7])
{
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            grille[i][j] = ' ';
        }
    }
}

// vérifie si le joueur a gagné
int verifierVictoire(char grille[6][7])
{
    if (grille[0][0] != ' ' && grille[0][1] != ' ' &&
        grille[0][2] != ' ' && grille[0][3] != ' ' &&
        grille[0][4] != ' ' && grille[0][5] != ' ' &&
        grille[0][6] != ' ')
        {
            printf("\nLe plateau est plein, il y a égalité\n");
            return EGALITE;
        }
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0 ; j < 7 ; j++)
            if (   (grille[i][j] == grille[i][j + 1] &&
                    grille[i][j] == grille[i][j + 2] &&
                    grille[i][j] == grille[i][j + 3] &&
                    grille[i][j] != ' ') // Vérifie en ligne
                ||
                    (grille[i][j] == grille[i + 1][j] &&
                     grille[i][j] == grille[i + 2][j] &&
                     grille[i][j] == grille[i + 3][j] &&
                     grille[i][j] != ' ') // vérifie en colonne
                ||
                    (grille[i][j] == grille[i + 1][j + 1] &&
                     grille[i][j] == grille[i + 2][j + 2] &&
                     grille[i][j] == grille[i + 3][j + 3] &&
                     grille[i][j] != ' ') // vérifie en diagonale droite
                ||
                    (grille[i][j] == grille[i + 1][j - 1] &&
                     grille[i][j] == grille[i + 2][j - 2] &&
                     grille[i][j] == grille[i + 3][j - 3] &&
                     grille[i][j] != ' ') // vérifie en diagonale gauche
                    )
                return VICTOIRE;
    }
    return 0;
}

void fermerConnexionsClients(Partie *partie) {
    for (int joueur = 0 ; joueur < 2 ; joueur++) {
        close(partie->joueurs[joueur].socket);
    }
}

void renvoi (int sock) {

    char buffer[BUFFER_LEN];
    long longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0)
    	return;
    
    printf("message lu : %s \n", buffer);
    
    buffer[0] = 'R';
    buffer[1] = 'E';
    buffer[longueur] = '#';
    buffer[longueur+1] ='\0';
    
    printf("message apres traitement : %s \n", buffer);
    
    printf("renvoi du message traite.\n");

    /* mise en attente du programme pour simuler un délai de transmission */
    sleep(3);
    
    write(sock,buffer,strlen(buffer)+1);
    
    printf("message envoyé. \n");
}

int main()
{
    int 	        server_socket, 		    /* descripteur de socket serveur */
                    partieIdCpt;

	unsigned int    longueur_adresse_courante;  /* longueur d'adresse courante d'un client */
    sockaddr_in     server_info, 		        /* structure d'adresse locale*/
			        client_info;                /* adresse client courant */
//    servent*	    ptr_service; 			    /* les infos récupérées sur le service de la machine */
    char 	        machine[TAILLE_MAX_NOM+1];  /* nom de la machine locale */

    /////////////////////////////////////////////////////////////
    // <<<          INITIALISATION SOCKET SERVEUR          >>> //
    /////////////////////////////////////////////////////////////

    gethostname(machine,TAILLE_MAX_NOM);		/* récupération du nom de la machine */
    
    /* initialisation de la structure server_info avec les infos récupérées */
    
    // server_info
    /* copie de ptr_hôte vers server_info */
    server_info.sin_family		= AF_INET; 	                /* ou AF_INET */
    server_info.sin_addr.s_addr	= INADDR_ANY; 			    /* ou AF_INET */
    server_info.sin_port        = htons(PORT);
    
    printf("numéro de port pour la connexion au serveur : %d \n",
		   ntohs(server_info.sin_port) /* ntohs(ptr_service->s_port) */);

    // socket
    /* creation du socket */
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de créer la socket de connexion avec le client.");
        return -1;
    }

    // bind
    /* association du socket descriptor server_socket à la structure d'adresse server_info */
    if ((bind(server_socket, (sockaddr*)(&server_info), sizeof(server_info))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		return -1;
    }
    
    // listen
    /* initialisation de la file d'écoute */
    listen(server_socket, 5);

    // traitement serveur
    /* attente des connexions et traitement des donnees recues */

    /////////////////////////////////////////////////////////////
    // <<<              INITIALISATION PARTIE              >>> //
    /////////////////////////////////////////////////////////////

    Partie *partie = malloc(sizeof (Partie));
    partieIdCpt = 0;

    /* Initialisation des joueurs */
    // on attend que 2 joueurs se connectent avant de démarrer la partie
    for (int i = 0 ; i < 2 ; i++) {
        longueur_adresse_courante = sizeof(client_info);

        /* le socket client sera renseigné par accept via les infos du connect */
        if( (partie->joueurs[i].socket = accept(server_socket,(sockaddr*)(&client_info),&longueur_adresse_courante)) < 1)
        {
            fprintf(stdout,"accept failed %d \n",partie->joueurs[i].socket);
            return -1;
        }

        /* On initialise les données de partie */
        partie->id = partieIdCpt;
        partie->joueurCourant = PLAYER_ONE;

        /* on initialise les joueurs */
        partie->joueurs[i].id = i == 0 ? PLAYER_ONE : PLAYER_TWO;
        partie->joueurs[i].couleur = i == 0 ? YELLOW_PIECE : RED_PIECE;

        partieIdCpt++;
    }

    printf("Les 2 joueurs ont été trouvés, la partie peut commencer.\n");
    initialiserGrille(partie->grille);

    printGrille(partie->grille);
    printJoueurs(partie);

    /////////////////////////////////////////////////////////////
    // <<<               DÉROULEMENT PARTIE                >>> //
    /////////////////////////////////////////////////////////////

    // la partie se déroule tant qu'il n'y a pas d'égalité ni de victoire d'un des joueurs
    int result;
    while ((result = verifierVictoire(partie->grille)) == 0) {
		
		/* traitement du message */
		printf("Début de la partie.\n");

		renvoi(partie->joueurs[0].socket);

    }

    /* On gère ici les différents cas de fin de partie :
     * 1 : un des 2 joueurs a gagné
     * 2 : le tableau est plein, il y a donc égalité
     * default : d'après la fonction verifierVictoire et la condition du while, toute autre valeur est supposée comme une erreur */
    switch (result) {
        case 1:
            printf("Victoire du joueur %d.\n", partie->joueurs[partie->joueurCourant].couleur);
            break;
        case 2:
            printf("Il y a égalité.\n");
            break;
        default:
            printf("Erreur lors de la partie.\n");
            break;
    }

    printGrille(partie->grille);
    fermerConnexionsClients(partie);
    return 0;
}
