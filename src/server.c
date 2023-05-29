#include <netdb.h>
#include <pthread.h>

#include "config.h"

#define TAILLE_MAX_NOM 256
#define VICTOIRE 1
#define EGALITE 2
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

// vérifie si le joueur a gagné
int verifierVictoire(char grille[N_LIGNES][N_COL]) {
    // Vérifier les lignes
    for (int i = 0; i < N_LIGNES; i++) {
        for (int j = 0; j < N_COL - 3; j++) {
            char piece = grille[i][j];
            if (piece != ' ' && grille[i][j + 1] == piece &&
                grille[i][j + 2] == piece && grille[i][j + 3] == piece) {
                return VICTOIRE;
            }
        }
    }

    // Vérifier les colonnes
    for (int i = 0; i < N_LIGNES - 3; i++) {
        for (int j = 0; j < N_COL; j++) {
            char piece = grille[i][j];
            if (piece != ' ' && grille[i + 1][j] == piece &&
                grille[i + 2][j] == piece && grille[i + 3][j] == piece) {
                return VICTOIRE;
            }
        }
    }

    // Vérifier les diagonales ascendantes
    for (int i = 3; i < N_LIGNES; i++) {
        for (int j = 0; j < N_COL - 3; j++) {
            char piece = grille[i][j];
            if (piece != ' ' && grille[i - 1][j + 1] == piece &&
                grille[i - 2][j + 2] == piece && grille[i - 3][j + 3] == piece) {
                return VICTOIRE;
            }
        }
    }

    // Vérifier les diagonales descendantes
    for (int i = 0; i < N_LIGNES - 3; i++) {
        for (int j = 0; j < N_COL - 3; j++) {
            char piece = grille[i][j];
            if (piece != ' ' && grille[i + 1][j + 1] == piece &&
                grille[i + 2][j + 2] == piece && grille[i + 3][j + 3] == piece) {
                return VICTOIRE;
            }
        }
    }

    // Vérifier l'égalité : il y a égalité quand la grille est pleine et qu'aucun joueur n'a remporté le match
    if (grille[0][0] != ' ' && grille[0][1] != ' ' &&
        grille[0][2] != ' ' && grille[0][3] != ' ' &&
        grille[0][4] != ' ' && grille[0][5] != ' ' &&
        grille[0][6] != ' ') {
        printf("\nLe plateau est plein, il y a égalité\n");
        return EGALITE;
    }

    return 0;
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
            printf("Tour %d : \n", cptTour);
            printf("Joueur : %c\n", partie->joueurs[partie->joueurCourant].piece);
        }

        int joueurCourant = partie->joueurCourant;
        writeSocket(partie->joueurs[partie->joueurCourant].socket, EVENT_TOUR_JOUEUR);

        char *colonne = readSocket(partie->joueurs[joueurCourant].socket, buffer);
        int colonneChoisie = colonne[3] - 49;

//        printf("Choix du joueur %c : %s \n", partie->joueurs[joueurCourant].piece, colonne);
//        printf("Colonne choisie : %d \n", colonneChoisie);

        placerPiece(partie->grille, colonneChoisie, partie->joueurs[joueurCourant].piece);
        if (debug)
            printGrille(partie->grille);

        partie->joueurCourant = !joueurCourant;
        joueurCourant = partie->joueurCourant;

        writeSocket(partie->joueurs[joueurCourant].socket, buffer);

        cptTour++;
    }

    /* On gère ici les différents cas de fin de partie :
     * 1 : un des 2 joueurs a gagné
     * 2 : le tableau est plein, il y a donc égalité
     * default : d'après la fonction verifierVictoire et la condition du while, toute autre valeur est supposée comme une erreur */
    switch (result) {
        case 1:
            printf("Victoire du joueur %c.\n", partie->joueurs[!partie->joueurCourant].piece);
            writeSocket(partie->joueurs[!partie->joueurCourant].socket, EVENT_VICTOIRE);
            writeSocket(partie->joueurs[partie->joueurCourant].socket, EVENT_DEFAITE);
            break;
        case 2:
            printf("Il y a égalité.\n");
            writeSocket(partie->joueurs[!partie->joueurCourant].socket, EVENT_EGALITE);
            writeSocket(partie->joueurs[partie->joueurCourant].socket, EVENT_EGALITE);
            break;
        default:
            printf("Erreur lors de la partie.\n");
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
        } else if (debug) {
            printf("Un nouveau joueur vient de se connecter\n");
        }

        /* on initialise un nouveau joueur
         * Premier joueur connecté :
         * id = 0
         * couleur = jaune
         */
        Joueur joueur;
        joueur.socket = current_socket;
        joueur.id = nbConnexions % 2 == 0 ? 0 : 1;
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

            if (debug)
                printf("Les 2 joueurs ont été trouvés, la partie peut commencer.\n");
            initialiserGrille(partie->grille);

            if (debug) {
                printGrille(partie->grille);
                printJoueurs(partie);
            }

            /////////////////////////////////////////////////////////////
            // <<<               DÉROULEMENT PARTIE                >>> //
            /////////////////////////////////////////////////////////////

            printf("Début de la partie %d\n", partieIdCpt);
            pthread_create(&partie->thread, NULL, jouerPartie, (void *) partie);
            if (debug)
                printf("thread créé\n");
            pthread_detach(partie->thread);
            if (debug)
                printf("thread détaché\n");
        }
    }
}
