#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const char PLAYER_ONE = 'J';
const char PLAYER_TWO = 'R';

void red()
{
    printf("\033[1;31m");
}

void yellow()
{
    printf("\033[1;33m");
}

void reset()
{
    printf("\033[0m");
}

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

// affiche la grille de jeu
void displayGrid(char grid[][7])
{
    printf("\n-----------------------------\n");
    for (int i = 0; i < 6; i++)
    {
        printf("| ");
        for (int j = 0; j < 7; j++)
        {
            grid[i][j] == PLAYER_ONE ? yellow() : red();
            printf("%c", grid[i][j]);
            reset();
            printf(" | ");
        }
        printf("\n-----------------------------\n");
    }
}

// Demande une colonne à l'utilisateur
int askColumn()
{
    int column;
    printf("Entrez la colonne voulue : ");
    scanf("%d", &column);
    return column - 1;
}

// demande la colonne à l'utilisateur et vérifie qu'elle est bien dans la grille
int chooseColumn(char grid[][7])
{
    int column = askColumn();
    bool verif = false;
    while (!verif)
    {
        if (column >= 0 && column <= 6 && grid[0][column] == ' ')
        {
            verif = true;
        }
        else
        {
            printf("\nValeur erronnee, entrer a nouveau !\n");
            column = askColumn();
        }
    }
    return column;
}

// Retourne le pion du joueur
char choosePiece(int cpt)
{
    return cpt % 2 ? PLAYER_TWO : PLAYER_ONE;
}

// Place le pion dans la grille
int placePiece(char grid[][7], int uneColonne, int cpt)
{
    if (grid[5][uneColonne] == ' ')
    {
        grid[5][uneColonne] = choosePiece(cpt);
    }
    else
    {
        for (int i = 0; i < 6 && grid[i][uneColonne] == ' '; i++)
        {
            if (grid[i + 1][uneColonne] != ' ')
            {
                grid[i][uneColonne] = choosePiece(cpt);
            }
        }
    }
    return cpt + 1;
}

// vérifie si le joueur a gagné
bool hasWon(char grid[][7])
{
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            if (grid[i][j] == grid[i][j + 1] &&
                grid[i][j] == grid[i][j + 2] &&
                grid[i][j] == grid[i][j + 3] &&
                grid[i][j] != ' ') // Verifie en ligne
                return true;

            else if (grid[i][j] == grid[i + 1][j] &&
                     grid[i][j] == grid[i + 2][j] &&
                     grid[i][j] == grid[i + 3][j] &&
                     grid[i][j] != ' ') // verifie en colonne
                return true;

            else if (grid[i][j] == grid[i + 1][j + 1] &&
                     grid[i][j] == grid[i + 2][j + 2] &&
                     grid[i][j] == grid[i + 3][j + 3] &&
                     grid[i][j] != ' ') // verifie en diagonale droite
                return true;

            else if (grid[i][j] == grid[i + 1][j - 1] &&
                     grid[i][j] == grid[i + 2][j - 2] &&
                     grid[i][j] == grid[i + 3][j - 3] &&
                     grid[i][j] != ' ') // verifie en diagonale gauche
                return true;
        }
    }
    return false;
}

int main()
{
    char grid[6][7];
    int column;
    int cpt = 0;
    char currentPiece;

    initGrid(grid);

    while (!hasWon(grid))
    {
        currentPiece = choosePiece(cpt);
        printf("\nAu joueur %c de jouer :\n", currentPiece);

        column = chooseColumn(grid);

        cpt = placePiece(grid, column, cpt);
        displayGrid(grid);

        if (grid[0][0] != ' ' && grid[0][1] != ' ' &&
            grid[0][2] != ' ' && grid[0][3] != ' ' &&
            grid[0][4] != ' ' && grid[0][5] != ' ' &&
            grid[0][6] != ' ')
        {
            printf("\nLe plateau est plein, il y a egalite\n");
            return 0;
        }
    }

    printf("\nBravo, le joueur %c a gagne !\n", choosePiece(cpt + 1)); // + 1 pour sauter le tour du joueur courant et récupérer le gagnant
    return 0;
}
