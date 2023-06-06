/*
 * Fichier définissant les variables de configuration et
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#ifndef M1_RESEAUX_PUISSANCE4_CONFIG_H
#define M1_RESEAUX_PUISSANCE4_CONFIG_H

/* Paramètres du socket */
#define BUFFER_LEN 256
#define DELAY_MS 300

/* Paramètres de la grille */
#define N_LIGNES 6
#define N_COL 7

/* Paramètres des joueurs */
#define YELLOW_PIECE 'J'
#define RED_PIECE 'R'

#define PLAYER_ONE 0
#define PLAYER_TWO 1

/* Évènements */
#define EVENT_JOUEUR_JAUNE "JAUN"
#define EVENT_JOUEUR_ROUGE "ROUG"

#define EVENT_DEBUT_PARTIE "DEBP"
#define EVENT_TOUR_JOUEUR "TOUR"

#define EVENT_VICTOIRE "VICT"
#define EVENT_DEFAITE "DEFT"
#define EVENT_EGALITE "EGAL"

#define EVENT_FIN_PARTIE "FINP"


void writeSocket(int sd, char* message) {
    if ((send(sd, message, strlen(message), 0)) < 0) {
        perror("erreur: envoi sur le socket impossible");
        exit(EXIT_FAILURE);
    }
    /* nombre de millisecondes * 1000 pour convertir en microsecondes
     * utilisé pour être sûr que le buffer ne réécrit pas avant que la valeur soit lue par le récepteur
     */
    usleep(DELAY_MS * 1000);
}

char* readSocket(int sd, char * buffer) {
    if(read(sd, buffer, sizeof(buffer)) < 0) {
        perror("erreur: réception sur le socket impossible");
        exit(EXIT_FAILURE);
    }

    return buffer;
}

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

// initialise un tableau[6][7] avec des ' '
void initialiserGrille(char grille[][N_COL])
{
    for (int i = 0; i < N_LIGNES; i++)
    {
        for (int j = 0; j < N_COL; j++)
        {
            grille[i][j] = ' ';
        }
    }
}

// Place le pion dans la grille
void placerPiece(char grille[][N_COL], int colonne, char piece)
{
    if (grille[5][colonne] == ' ')
    {
        grille[5][colonne] = piece;
    }
    else
    {
        for (int i = 0; i < N_LIGNES && grille[i][colonne] == ' ' ; i++)
        {
            if (grille[i + 1][colonne] != ' ')
            {
                grille[i][colonne] = piece;
            }
        }
    }
}

#endif //M1_RESEAUX_PUISSANCE4_CONFIG_H
