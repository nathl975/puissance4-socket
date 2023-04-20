/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#include "config.h"

#define HOST "localhost"

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

char* colonneEvents[7] = {"COL1", "COL2", "COL3", "COL4", "COL5", "COL6", "COL7"};

int demanderColonne(char grille[][N_COL]) {
    int colonne = -1;

    while (colonne < 1 || colonne > N_COL) {
        printf("Entrez le numéro de colonne (entre 1 et %d) : ", N_COL);
        if (scanf("%d", &colonne) != 1) {
            // En cas d'erreur de saisie (non numérique)
            colonne = -1;
            while (getchar() != '\n');  // Vider le buffer d'entrée
        }

        if (grille[0][colonne-1] != ' ') {
            printf("La colonne %d est déjà pleine, veuillez en choisir une autre.\n", colonne);
            colonne = -1;
        } else if (colonne < 1 || colonne > N_COL) {
            printf("Le numéro de colonne entré n'est pas valide (compris entre 1 et %d).\n", colonne);
            colonne = -1;
        }
    }

    return colonne - 1;
}

int main(int argc, char* argv[]) {
    int 	    socket_descriptor; 	/* descripteur de socket */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		    /* info sur une machine hôte */
    // servent *	ptr_service; 		/* info sur service */
    char 	    buffer[BUFFER_LEN];
    char*       port = argv[1];     /* port utilisé pour la connexion */

    /////////////////////////////////////////////////////////////
    // <<<          INITIALISATION SOCKET CLIENT           >>> //
    /////////////////////////////////////////////////////////////

    if ((ptr_host = gethostbyname(HOST)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }
    
    /* copie caractère par caractère des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET;

    adresse_locale.sin_port = htons(strtol(port, NULL, 10));

    printf("numéro de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    
    /* creation du socket */
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

    /////////////////////////////////////////////////////////////
    // <<<        GESTION DES ÉVÉNEMENTS DE PARTIE         >>> //
    /////////////////////////////////////////////////////////////

    char* event = "";

    char grille[N_LIGNES][N_COL];
    char piece;
    char pieceAutreJoueur; // la pièce du joueur adverse pour l'affichage
    while (strcmp(event, EVENT_FIN_PARTIE)) {
        event = readSocket(socket_descriptor, buffer);

//        printf("Event : %s\n", event);

        if (strcmp(event, EVENT_DEBUT_PARTIE) == 0) {
            initialiserGrille(grille);

            printf("Un autre joueur a été trouvé, la partie va commencer.\n");
        } else if (strcmp(event, EVENT_JOUEUR_JAUNE) == 0) {
            piece = 'J';
            pieceAutreJoueur = 'R';

            printf("Vous jouez les jaune.\n");
        } else if (strcmp(event, EVENT_JOUEUR_ROUGE) == 0) {
            piece = 'R';
            pieceAutreJoueur = 'J';

            printf("Vous jouez les rouge.\n");
            printf("Votre adversaire commence...\n");
        } else if (strcmp(event, EVENT_TOUR_JOUEUR) == 0) {
            printf("C'est votre tour.\n");
            printGrille(grille);

            int colonne = demanderColonne(grille);
            placerPiece(grille, colonne, piece);
            printGrille(grille);

            writeSocket(socket_descriptor, colonneEvents[colonne]);
            printf("A votre adversaire de jouer...\n");
        } else if (event[3] > '0' && event[3] <= '7') {
            printf("Le joueur adverse a joué la colonne %c\n", event[3]);

            int colonne = event[3] - 49;
            placerPiece(grille, colonne, pieceAutreJoueur);
        } else if (strcmp(event, EVENT_VICTOIRE) == 0) {
            printf("Félicitations, vous avez gagné !\n");
        } else if (strcmp(event, EVENT_DEFAITE) == 0) {
            printGrille(grille);
            printf("Dommage, votre adversaire a été meilleur...\n");
        } else if (strcmp(event, EVENT_EGALITE) == 0) {
            printf("Il y a égalité, bien joué à vous deux.\n");
        } else if (strcmp(event, EVENT_FIN_PARTIE) == 0) {
            printf("Fin de la partie, déconnexion en cours...\n");

            close(socket_descriptor);
        } else {
            printf("Erreur évènementielle, fin de la partie.\nErreur %s.\n", event);

            return -1;
        }
    }

    return 0;
}