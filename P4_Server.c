/* programme Serveur
   Alloue un socket
   puis en boucle
  1) attend une nouvelle connexion du client
    2) envoie un message court au client
  3) ferme la connexion
   fin boucle

   Syntaxe :  serveur [port]
    port numéro de port à utiliser (optionnel, par défaut PROTOPORT)
*/

#ifdef WIN32 /* si vous êtes sous Windows */

#include <winsock2.h>

#elif defined (linux) /* si vous êtes sous Linux */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else /* sinon vous êtes sur une plateforme non supportée */

#error not defined for this platform

#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define PROTOPORT 5000
#define QLEN 10
#define MTU 1450

typedef struct
{
  int CoupGagnant;
  int CoupJoueur;
  int Socket;
  int EtatJeu;
  int JoueurID;
  int * Plateau;
  int * HauteurPlateau;

  pthread_t IDThread;

  pthread_mutex_t MutexJoueur;
  pthread_cond_t CondBegin;
  pthread_cond_t CondEnd;
}
Joueur;

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

void Display(int Plateau[][6])
{
  printf("\n-----------------------------\n");
  printf("   1     2     3     4     5     6     7\n\n");
  for(int y=5;y>-1;y--)
  {
    for(int x=0;x<7;x++)
    {
      //printf("%d, %d ",x,y);
      // printf("%c",177+Plateau[x][y]);
      // printf("%c",177+Plateau[x][y]);
      printf("%d    ",Plateau[x][y]);
      // if(Plateau[x][y]==-1){
      //         yellow();
      //       }
      //       if(Plateau[x][y]==1){
      //         red();
      //       }
      //       reset();
    }
    printf("\n\n");
   }
  
    // for (int i = 0; i < 6; i++)
    // {
    //     printf("| ");
    //     for (int j = 0; j < 7; j++)
    //     {
    //         //Plateau[i][j] = yellow();
    //         //printf("%c", Plateau[i][j]);
    //         if(Plateau[i][j]==-1){
    //           yellow();
    //         }
    //         if(Plateau[i][j]==1){
    //           red();
    //         }
    //         reset();
    //         printf(" | ");
    //     }
    //     printf("\n-----------------------------\n");
    //}
}

int EvalPlateau(int Plateau[][6])
{
  int S=0;

  // Gestion de la verticale
  for(int x=0;x<7;x++)
  {
    for(int y=0;y<3;y++)
    {
      S=0;
      S=Plateau[x][y+0]+Plateau[x][y+1]+Plateau[x][y+2]+Plateau[x][y+3];

      if(S*S==16)
      {
        return 1;
      }
    }
  }

  // Gestion de la verticale
  for(int x=0;x<4;x++)
  {
    for(int y=0;y<6;y++)
    {
      S=0;
      S=Plateau[x+0][y]+Plateau[x+1][y]+Plateau[x+2][y]+Plateau[x+3][y];

      if(S*S==16)
      {
        return 1;
      }
    }
  }

  // Gestion de la diagonale LU inverse
  for(int x=0;x<4;x++)
  {
    for(int y=3;y<6;y++)
    {
      S=0;
      S=Plateau[x+0][y+0]+Plateau[x+1][y+1]+Plateau[x+2][y+2]+Plateau[x+3][y+3];

      if(S*S==16)
      {
        return 1;
      }
    }
  }

  // Gestion de la diagonale LU
  for(int x=0;x<4;x++)
  {
    for(int y=0;y<3;y++)
    {
      S=0;
      S=Plateau[x+0][y-0]+Plateau[x+1][y-1]+Plateau[x+2][y-2]+Plateau[x+3][y-3];

      if(S*S==16)
      {
        return 1;
      }
    }
  }
  return 0;
}

// Définition de la fonction associée aux THREADS joueurs
static void * LaunchPlayer (void * Ressources)
{

  int marque=1;
  int CoupValide=0;
  int n_recu=0;
  char BufIN[MTU]={0};
  char BufOUT[MTU]={0};

  // On fait le lien avec les ressources partagées, ici la structure joueur
  Joueur* Player = Ressources;

  // On indique qu'on débute
  printf("THREAD %d : START \n",Player->JoueurID);

  // Les deux joueurs étant des clones, il faut les distinguer sur le plateau
  // L'un autre pour marque 1 et l'autre -1
  if(Player->JoueurID%2==0)
  {
    marque=-1;
  }

  // Début du Jeu
  while(1)
  {
    // Attente du signal de début
    pthread_mutex_lock (& Player->MutexJoueur);
    pthread_cond_wait (& Player->CondBegin, & Player->MutexJoueur);
    // On indique qu'on débute
    printf("THREAD %d : START CYCLE\n", Player->JoueurID);

    // Si la partie tourne encore 
    if(Player->EtatJeu==0)
    {
      // On dégèle l'interface du client
      sprintf(BufOUT,"BEGT");
      for(int x=0;x<42;x++)
      {
        // On copie l'état du plateau après le FLAG
        *(BufOUT+5+x)=Player->Plateau[x];
      }
      send(Player->Socket,BufOUT,47,0);
      printf("THREAD %d : BEGT SENT\n", Player->JoueurID);
    }

    // Si le joueur a perdu
    if(Player->EtatJeu==-1)
    {
      sprintf(BufOUT,"ENDL");
      for(int x=0;x<42;x++)
      {
        // On copie l'état du plateau après le FLAG
        *(BufOUT+5+x)=Player->Plateau[x];
      }
      send(Player->Socket,BufOUT,47,0);

      printf("THREAD %d : EXIT LOOSER \n", Player->JoueurID);
      pthread_cond_signal (&Player->CondEnd);
      pthread_mutex_unlock (&Player->MutexJoueur);
      closesocket(Player->Socket);
      return NULL;
    }

    // Si le joueur a gagné
    if(Player->EtatJeu==1) // On indique au client qu'il gagné
    {
      sprintf(BufOUT,"ENDW");
      for(int x=0;x<42;x++)
      {
        // On copie l'état du plateau après le FLAG
        *(BufOUT+5+x)=Player->Plateau[x];
      }
      send(Player->Socket,BufOUT,47,0);

      printf("THREAD %d : EXIT WINNER \n", Player->JoueurID);
      pthread_cond_signal (&Player->CondEnd);
      pthread_mutex_unlock (&Player->MutexJoueur);
      closesocket(Player->Socket);
      return NULL;
    }

    CoupValide=0;

    // Tant que le coup reçu n'est pas valide
    while(CoupValide!=1)
    {
      // On reçoit le coup du joueur qui est un entier appartenant à [1;7] et qui indique quelle colonne il joue
      n_recu=0;
      n_recu=recv(Player->Socket,BufIN,MTU,0);
      if(n_recu>0)
      {
        Player->CoupJoueur = atoi(BufIN);
      }

      // Si la colonne existe bien
      if(Player->CoupJoueur>0 && Player->CoupJoueur<8)
      {
        printf("THREAD %d : INPUT IN RANGE \n", Player->JoueurID);

        // Et si elle n'est pas déjà pleine
        if(Player->HauteurPlateau[Player->CoupJoueur-1]<6)
        {
          // Alors on valide le coup
          CoupValide=1;
          printf("THREAD %d : INPUT OK \n", Player->JoueurID);
        }
        // Autrement
        else
        {
          // On indique l'erreur
          printf("THREAD %d : INPUT STUCK \n", Player->JoueurID);
          sprintf(BufOUT,"ERRO"); // Erreur de dépassement
          send(Player->Socket,BufOUT,strlen(BufOUT),0);
        }
      }

      // Si la colonne n'existe pas 
      if(Player->CoupJoueur<=0 || Player->CoupJoueur>=8)
      {
        // On indique l'erreur
        printf("THREAD %d : INPUT OUT OF RANGE \n", Player->JoueurID);
        sprintf(BufOUT,"ERRR"); // Erreur dans l'intervalle
        send(Player->Socket,BufOUT,strlen(BufOUT),0);
      }
    }

    // Ici le coup est valide
    // Alors on l'inscrit sur le plateau
    Player->Plateau[(Player->CoupJoueur-1)*6+Player->HauteurPlateau[Player->CoupJoueur-1]]=marque;
    // On incrémente la place occupée dans cette colonne
    Player->HauteurPlateau[Player->CoupJoueur-1]++;

    printf("THREAD %d : ENDT SENT\n", Player->JoueurID);

    // On informe de la fin du tour
    sprintf(BufOUT,"ENDT");
    for(int x=0;x<42;x++)
    {
      // On envoie une copie du tableau
      *(BufOUT+5+x)=Player->Plateau[x];
    }
    send(Player->Socket,BufOUT,47,0);

    // Envoi du signal de fin
    pthread_cond_signal (&Player->CondEnd);
    pthread_mutex_unlock (&Player->MutexJoueur);
  }

  return NULL;
}

// Main
void main(int argc, char ** argv)
{

  struct protoent *ptrp;   // pointeur sur la table de protocoles
  struct sockaddr_in sad;  // structure pour adresse serveur
  struct sockaddr_in cad;  // structure pour adresse client
  int sd;                  // descripteurs de socket
  int port;                // numero de port
  int alen;
  int error;
  int visits=0;
  char  BufIN[MTU];
  char  BufOUT[MTU];

  int End=0;
  int Plateau[7][6]={0};
  int HauteurPlateau[7]={0};
  int Somme=0;



  /////////////////////////////////////////////////////////////
  // <<< P A R T I E   I N I T I A T I O N   R E S E A U >>> //
  /////////////////////////////////////////////////////////////



  printf("RESEAU :");
  #ifdef WIN32 //specifique Windows
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(err < 0)
    {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
    }
  #endif

  memset((char*)&sad,0,sizeof(sad)); //effacer struct sockaddr_in sad
  sad.sin_family=AF_INET;       //définir famille internet pour la socket du serveur

   /* rechercher l'argument PORT de la la ligne de commande s'il est spécifié
       sinon prendre PROTOPORT */

  if (argc>1)  //si numero de port spécifié au niveau de la ligne de commande
  {
    port = atoi(argv[1]);  //conversion de la chaine de caractère en binaire
  }
  else  //protocole par défaut
  {
    port=PROTOPORT;
  }


  if (port>0)
  {
    sad.sin_port=htons((u_short)port);
  }  //fonction qui inverse les octets
  else
  {
    printf("Invalid PORT Number :%s \n",argv[1]);
    exit(1);
  }


    /* Convertir le nom de protocole de transport TCP/IP en n° de protocole */

  if ( ((int)(ptrp=getprotobyname("tcp")))==0)   //fonction qui retourne les informations relatives au protocoel TCP
  {
    printf("TCP Protocol ERROR");
    exit(1);
  }

  sd =socket(PF_INET, SOCK_STREAM, ptrp->p_proto); // création de la socket d'écoute

  if (sd<0)
  {
    printf("LISTENING Socket Creation Problem\n");
    exit(1);
  }

    /* Lier l'adresse locale à la socket */

  if (bind(sd, (struct sockaddr *)&sad, sizeof(sad))<0)
  {
    fprintf(stdout,"Error (bind) \n");
    exit(1);
  }

   /* spécifier la taille de la file d'attente des requêtes */

  if (listen(sd,QLEN)<0)           // création de la file d'attente associé à la socket d'écoute
  {
    fprintf(stdout,"Error (listen)\n");
    exit(1);
  }

   /* Boucle principale (accepter les requêtes et les traiter) */

  alen=sizeof(cad);

  int n_recu=0;
  // Vestige de l'implémentation du Juste Prix
  int prix=0;




  /////////////////////////////////
  // <<< P A R T I E   J E U >>> //
  /////////////////////////////////

  //-------------------------//
  // - P R E P A R A T I O N //
  //-------------------------// 

  printf("Debut partie jeu\n");

  int x=0;

  // Tableau de pointeurs vers structure
  Joueur* ListeJoueur[2];

  // Boucle de préparation des clients, on note pour acquis le fait qu'il n'y ait que deux clients
  // Pour chaque joueur :
  for(x=0;x<2;x++)
  {
    // On alloue la place pour passer l'argument aux threads
    ListeJoueur[x]=malloc(sizeof(Joueur));

    // On créé la socket de communication client
    if( (ListeJoueur[x]->Socket=accept(sd, (struct sockaddr *)&cad, &alen)) < 1)
    {
      fprintf(stdout,"accept failed %d \n",ListeJoueur[x]->Socket);
      exit(1);
    }

    // Autre vestige du Juste Prix
    ListeJoueur[x]->CoupGagnant=prix;

    ListeJoueur[x]->EtatJeu=0;
    ListeJoueur[x]->JoueurID=x+1;
    // On oriente les pointeurs vers les bonnes variables partagées
    // Ici le plateau sera partagé par les deux joueurs (sous forme de thread) et le main
    ListeJoueur[x]->Plateau=Plateau;
    ListeJoueur[x]->HauteurPlateau=HauteurPlateau;

    // Initialisation des MUTEX et des CONDITIONS
    ListeJoueur[x]->MutexJoueur=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    ListeJoueur[x]->CondBegin=(pthread_cond_t)PTHREAD_COND_INITIALIZER;
    ListeJoueur[x]->CondEnd=(pthread_cond_t)PTHREAD_COND_INITIALIZER;

    // On lance finalement le thread du client N° x+1
    ListeJoueur[x]->IDThread=pthread_create(&ListeJoueur[x]->IDThread, NULL, LaunchPlayer, (void*) ListeJoueur[x]);
  }

  //---------//
  // - J E U //
  //---------//

  // On lance la partie
  while(1)
  {
    for(x=0;x<2;x++)
    {

      printf("MAIN : TOUR JOUEUR %d",x+1);
      // Début de la phase de synchronisation Main-Thread

      pthread_mutex_lock(&ListeJoueur[x]->MutexJoueur);
      pthread_cond_signal(&ListeJoueur[x]->CondBegin);

      // Pendant ce temps le MAIN est gelé en attendant que le THREAD du joueur x+1 lui redonne la main

      pthread_cond_wait(&ListeJoueur[x]->CondEnd, &ListeJoueur[x]->MutexJoueur);
      pthread_mutex_unlock(&ListeJoueur[x]->MutexJoueur);

      printf(" : %d \n", ListeJoueur[x]->CoupJoueur);

      // Fin de la phase de synchronisation Main-Thread

      // On affiche le tableau
      Display(Plateau);

      // Si la partie est terminée
      if(EvalPlateau(Plateau))
      {
        // On change l'état du jeu aux deux THREADS

        // Ce joueur a gagné
        ListeJoueur[x]->EtatJeu=1;
        // Celui-ci a perdu
        ListeJoueur[(x+1)%2]->EtatJeu=-1;
        // On sort du While
        goto EXIT;
      }

      // En cas de match nul, on vide le tableau et on continue
      Somme=0;
      for(int k=0;k<7;k++)
      {
        Somme+=HauteurPlateau[k];
      }
      
      if(Somme==42)
      {
        memset(HauteurPlateau, 0, 7);
        memset(Plateau, 0, 42);
      }

    }
  }

  EXIT:

  // On redonne une fois la main à chacun des THREADS afin qu'il indiquent à leur client l'état du jeu
  for(x=0;x<2;x++)
  {
      printf("FIN : %d\n",x);
      pthread_mutex_lock(&ListeJoueur[x]->MutexJoueur);
      pthread_cond_signal(&ListeJoueur[x]->CondBegin);

      // Le THREAD indique au client qu'il a perdu/gagné

      pthread_cond_wait(&ListeJoueur[x]->CondEnd, &ListeJoueur[x]->MutexJoueur);
      pthread_mutex_unlock(&ListeJoueur[x]->MutexJoueur);
  }
  printf("Fin de la partie, le joueur %d est gagnant\n",x);

#ifdef WIN32
   WSACleanup();
#endif
}

