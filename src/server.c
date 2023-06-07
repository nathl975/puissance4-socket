#include <netdb.h>
#include <pthread.h>

#include "config.h"

#define TAILLE_MAX_NOM 256
#define NB_CLIENTS 4

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

long debug = 0; /* définit si on affiche ou non la progression de la partie */

typedef struct {
    int id;
    char piece;
    int socket;
} Joueur;

typedef struct {
    int id,
            joueurCourant;
    char grille[N_LIGNES][N_COL];
    Joueur joueurs[2];
    pthread_t thread;
} Partie;

/* affiche les informations des joueurs de la partie */
void printJoueurs(Partie *partie) {
    printf("===================\n");
    for (int i = 0; i < 2; i++) {
        printf("Joueur %d \n", i + 1);
        printf("id : %d \n", partie->joueurs[i].id);
        printf("socket : %d \n", partie->joueurs[i].socket);
        printf("piece : %c \n", partie->joueurs[i].piece);
        printf("===================\n");
    }
}

void fermerConnexionsClients(Partie *partie) {
    for (int joueur = 0; joueur < 2; joueur++) {
        close(partie->joueurs[joueur].socket);
    }
}

void debutPartie(Partie *partie) {
    for (int i = 0; i < 2; ++i) {
        writeSocket(partie->joueurs[i].socket, EVENT_DEBUT_PARTIE);
    }
    writeSocket(partie->joueurs[0].socket, EVENT_JOUEUR_JAUNE);
    writeSocket(partie->joueurs[1].socket, EVENT_JOUEUR_ROUGE);
}

void finPartie(Partie *partie) {
    for (int i = 0; i < 2; ++i) {
        writeSocket(partie->joueurs[i].socket, EVENT_FIN_PARTIE);
    }
    printf("Fin de la partie %d\n", partie->id);
}

void *jouerPartie(void *data) {
    Partie *partie = (Partie *) data;

    if (debug) {
        printf("thread: %lu\n", partie->thread);
        printf("socket joueur 1: %d\n", partie->joueurs[0].socket);
        printf("socket joueur 2: %d\n", partie->joueurs[1].socket);
    }

    debutPartie(partie);

    int result;
    int cptTour = 1;
    /* la partie se déroule tant qu'il n'y a pas d'égalité ni de victoire d'un des joueurs */
    while ((result = verifierVictoire(partie->grille)) == 0) {
        char buffer[BUFFER_LEN];

        if (debug) {
            printf("Partie %d, Tour %d : \n", partie->id, cptTour);
            printf("Partie %d, Joueur : %c\n", partie->id, partie->joueurs[partie->joueurCourant].piece);
        }

        int joueurCourant = partie->joueurCourant;
        writeSocket(partie->joueurs[partie->joueurCourant].socket, EVENT_TOUR_JOUEUR);

        char *colonne = readSocket(partie->joueurs[joueurCourant].socket, buffer);
        int colonneChoisie = colonne[3] - 49;

        if (debug) {
            printf("Partie %d: Choix du joueur %c : %s \n", partie->id, partie->joueurs[joueurCourant].piece, colonne);
            printf("Partie %d: Colonne choisie : %d \n", partie->id, colonneChoisie);
        }

        placerPiece(partie->grille, colonneChoisie, partie->joueurs[joueurCourant].piece);
        if (debug) {
            printf("Partie %d: Grille :\n", partie->id);
            printGrille(partie->grille);
        }

        partie->joueurCourant = !joueurCourant;
        joueurCourant = partie->joueurCourant;

        writeSocket(partie->joueurs[joueurCourant].socket, buffer);

        cptTour++;
    }

    /* On gère ici les différents cas de fin de partie :
     * 1 : un des 2 joueurs a gagné
     * 2 : le tableau est plein, il y a donc égalité
     * default : d'après la fonction verifierVictoire et la condition du while, toute autre valeur est supposée comme une erreur
     */
    switch (result) {
        case 1:
            if (debug)
                printf("Partie %d: victoire du joueur %c.\n", partie->id, partie->joueurs[!partie->joueurCourant].piece);
            writeSocket(partie->joueurs[!partie->joueurCourant].socket, EVENT_VICTOIRE);
            writeSocket(partie->joueurs[partie->joueurCourant].socket, EVENT_DEFAITE);
            break;
        case 2:
            if (debug)
                printf("Partie %d: égalité.\n", partie->id);
            writeSocket(partie->joueurs[!partie->joueurCourant].socket, EVENT_EGALITE);
            writeSocket(partie->joueurs[partie->joueurCourant].socket, EVENT_EGALITE);
            break;
        default:
            if (debug)
                printf("Partie %d: erreur lors de la partie.\n", partie->id);
            break;
    }

    finPartie(partie);
    fermerConnexionsClients(partie);

    return NULL;
}

int main(int argc, char *argv[]) {
    int server_socket,            /* descripteur de socket serveur */
    partieIdCpt;

    unsigned int longueur_adresse_courante;  /* longueur d'adresse courante d'un client */
    sockaddr_in server_info,                /* structure d'adresse locale*/
    client_info;                /* adresse client courant */
    char machine[TAILLE_MAX_NOM + 1];  /* nom de la machine locale */

    if (argv[1] == NULL) {
        printf("Veuillez renseigner le port d'écoute. Fin du programme.\n");
        exit(EXIT_FAILURE);
    }
    char *port = argv[1];             /* port utilisé pour le serveur */

    if (argc == 3) {
        debug = strtol(argv[2], NULL, 10);
    }

    /////////////////////////////////////////////////////////////
    // <<<          INITIALISATION SOCKET SERVEUR          >>> //
    /////////////////////////////////////////////////////////////

    if (argc != 2 && argc != 3) {
        printf("Le nombre d'arguments passés est invalide.");
        exit(EXIT_FAILURE);
    }

    gethostname(machine, TAILLE_MAX_NOM);        /* récupération du nom de la machine */

    /* initialisation de la structure server_info avec les infos récupérées */

    // server_info
    /* copie de ptr_hôte vers server_info */
    server_info.sin_family = AF_INET;                    /* ou AF_INET */
    server_info.sin_addr.s_addr = INADDR_ANY;                /* ou AF_INET */
    server_info.sin_port = htons(strtol(port, NULL, 10));

    printf("numéro de port pour la connexion au serveur : %d \n", ntohs(server_info.sin_port));

    // socket
    /* creation du socket */
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur: impossible de créer la socket de connexion avec le client");
        return -1;
    }

    // bind
    /* association du socket descriptor server_socket à la structure d'adresse server_info */
    if ((bind(server_socket, (sockaddr *) (&server_info), sizeof(server_info))) < 0) {
        perror("erreur: impossible de lier la socket a l'adresse de connexion");
        return -1;
    }

    // listen
    /* initialisation de la file d'écoute */
    listen(server_socket, NB_CLIENTS);

    // traitement serveur
    /* attente des connexions et traitement des donnees recues */

    /////////////////////////////////////////////////////////////
    // <<<              INITIALISATION PARTIE              >>> //
    /////////////////////////////////////////////////////////////

    partieIdCpt = 0;

    Joueur *joueurs = malloc(NB_CLIENTS * sizeof(Joueur));

    /* Initialisation des joueurs */
    // on attend que les joueurs se connectent avant de les rediriger 2 par 2 vers un thread
    int nbConnexions = 0;
    while (1) {
        longueur_adresse_courante = sizeof(client_info);
        int current_socket;
        /* le socket client sera renseigné par accept via les infos du connect */
        if ((current_socket = accept(server_socket, (sockaddr *) (&client_info), &longueur_adresse_courante)) < 1) {
            printf("accept failed %d \n", current_socket);
            return -1;
        } else {
            printf("Connexion joueur\n");
        }

        /* on initialise un nouveau joueur
         * Premier joueur connecté :
         * id = 0
         * couleur = jaune
         */
        Joueur joueur;
        joueur.socket = current_socket;
        joueur.id = nbConnexions % 2 == 0 ? PLAYER_ONE : PLAYER_TWO;
        joueur.piece = nbConnexions % 2 == 0 ? YELLOW_PIECE : RED_PIECE;
        joueurs[nbConnexions] = joueur;

        nbConnexions++;

        if (debug)
            printf("connexions: %d\n", nbConnexions);

        if (nbConnexions % 2 == 0) {
            Partie *partie = malloc(sizeof(Partie));
            partie->id = partieIdCpt;
            partie->joueurs[0] = joueurs[nbConnexions - 2];
            partie->joueurs[1] = joueurs[nbConnexions - 1];
            /* On part du principe que le premier joueur connecté commence */
            partie->joueurCourant = partie->joueurs[0].id;
            partieIdCpt++;

            initialiserGrille(partie->grille);

            if (debug) {
                printGrille(partie->grille);
                printJoueurs(partie);
            }

            /////////////////////////////////////////////////////////////
            // <<<               DÉROULEMENT PARTIE                >>> //
            /////////////////////////////////////////////////////////////

            printf("Début de la partie %d\n", partie->id);
            pthread_create(&partie->thread, NULL, jouerPartie, (void *) partie);
            if (debug)
                printf("Partie %d: thread créé\n", partie->id);
            pthread_detach(partie->thread);
            if (debug)
                printf("Partie %d: thread détaché\n", partie->id);
        }
    }
}
