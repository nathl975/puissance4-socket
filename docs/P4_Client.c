/* programme Client
   Alloue un socket, le connecter au serveur, et afficher les messages reçus

	 Syntaxe :  client [ hôte [port]]
		hôte nom de l'ordinateur sur le quel s'exécute le serveur (optionnel par défaut localhost)
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
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <windows.h>

#define PROTOPORT 5000
#define MTU 1450
#define LGMAX 100

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
   printf("\t 1    2    3    4    5    6    7   \n\n");
   for(int y=5;y>-1;y--){
      printf("\t");
      for(int x=0;x<7;x++)
      {
         // printf("%c",177+Plateau[x][y]);
         // printf("%c",177+Plateau[x][y]);
         printf("%d    ",Plateau[x][y]);
         //  if(Plateau[x][y]==-1){
         //      printf("-1");yellow();
         //    }
         //    if(Plateau[x][y]==1){
         //      printf("1");red();
         //    }
         //    reset();
      }
      printf("\n\n");
   }
}

void main(int argc,char **argv)

{  
   struct hostent *ptrh;  // pointeur table d'hotes
   struct protoent *ptrp; // pointeur table de protocoles
   struct sockaddr_in sad; // structure adresse IP adresse du serveur à soliciter
   SOCKET sd;	// descriptuer de socket
   int port;	// numéro de port
   char * host; //pointeru nom d'hôte
   int a,nbchartotal=0;
   int n; //nombre de caractères lus
   char BufIN[MTU]; //tampon pour les données du serveur
   char BufOUT[MTU]; //tampon pour les données du serveur
   char localhost[LGMAX];

   #ifdef WIN32 //specifique Windows
      WSADATA wsa;
      int err = WSAStartup(MAKEWORD(2, 2), &wsa);
      if(err < 0)
      {
         puts("WSAStartup failed !");
         exit(EXIT_FAILURE);
      }
   #endif

   localhost[0]='\0';
   strncat(localhost,"localhost",LGMAX);

   memset((char*)&sad,0,sizeof(sad)); // effacer struct sockaddr_in voir si utile
   memset((char *)&BufIN,0,sizeof(BufIN)); // initialisation du tampon de reception
   memset((char *)&BufOUT,0,sizeof(BufOUT)); // initialisation du tampon d emission
   sad.sin_family =AF_INET; // définir famille internet

   if (argc>2)  //si numero de port spécifié au niveau de la ligne de commande
   {
	   port = atoi(argv[2]);	//conversion de la chaine de caractère en binaire
   }
   else  //protocole par défaut
   {
      port=PROTOPORT;
   }
   if (port>0)
   {
      sad.sin_port=htons((u_short)port);
   }
   else
   {
	   fprintf(stdout, "Not valid Port Number :%s \n",argv[2]);
	   exit(1);
   }

   if(argc>1)
   {
	   host=argv[1];
   }
   else
   {
      host=localhost;
   }

   /* convertir le nom d'hote en adressse IP et insérer cette dernière dans
   la structure d'adresse de socket */

   ptrh=gethostbyname(host);  // fonction qui retourne l'adresse IP de la machine dont le nom est host

   if ( ((char*)ptrh)==NULL)
   {
      fprintf(stdout," Invalid Host :%s\n",host);
      exit(1);
   }

   memcpy(&sad.sin_addr,ptrh->h_addr,ptrh->h_length); //copie de l'adresse IP dasn la sad

   /* Convertir le nom de protocole de transport TCP/IP en n° de protocole */

   if ( ((int)(ptrp=getprotobyname("tcp")))==0)
   {
      fprintf(stdout,"TCP Protocol  Error ");
      exit(1);
   }


   /* Créer un Socket */

   sd =socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
   if (sd<0)
   {
      fprintf(stdout,"Error (socket)\n");
      exit(1);
   }

   /* Connecter le socket au serveur spécifié */

   if (a=connect(sd, (struct sockaddr *)&sad, sizeof(sad)) <0)
   {
      fprintf(stdout,"Error (connect) \n");
   }



   /////////////////////////////////
   // <<< P A R T I E   J E U >>> //
   /////////////////////////////////



   nbchartotal=0;
   int n_envoi=0;
   char str_envoi[100];
   int fin=0;
   char EndW[]="ENDW"; // END WINNER
   char EndL[]="ENDL"; // END LOOSER
   char BegT[]="BEGT"; // BEGIN TURN
   char EndT[]="ENDT"; // END TURN
   char ErrO[]="ERRO"; // ERROR OVERFLOW
   char ErrR[]="ERRR"; // ERROR IN RANGE

   int Plateau[7][6]={0}; // PLATEAU DE JEU
   int CoupValide=0;      // VALIDITE DU COUP


   while(fin!=1)
   {
      // On initialise les variables de réception réseau
      n=0;
      memset(BufIN, 0, MTU);


      printf("Veuillez attendre votre tour\n");

      // On attend le signal de début lancé par le réseau
      while(strcmp(BufIN, BegT)!=0 && strcmp(BufIN, EndW)!=0 && strcmp(BufIN, EndL)!=0)
      {
         n=0;
         //Sleep(100);
         n=recv(sd,BufIN,MTU,0);
         BufIN[n]='\0';
      }

      // Si on a reçu l'odre de continuer la partie, on copie le plateau donné par le serveur
      // Ci dessous l'exemple d'un ordre reçu (taille = 5 si FLAG = ERRO/ERRR, sinon taille=42+5 car il y a une mise à jour du plateau)
      // -------------------------------------------------------------
      // | F | L | A | G | \0 | C1 | C2 | C3 | ... | C40 | C41 | C42 |
      // -------------------------------------------------------------
      // C1 à C42 sont les 42 cases du plateau 


      // Si FLAG = BEGIN TURN
      if(strcmp(BufIN, BegT)==0)
      {
         printf("SIZEOF PLATEAU : %d\n",sizeof(Plateau));
         for(int x=0;x<42;x++)
         {
            Plateau[x/6][x%6]=BufIN[x+5];
         }
         Display(Plateau);
      }


      // Si FLAG = END WINNER/LOOSER
      if(strcmp(BufIN, EndL)==0 || strcmp(BufIN, EndW)==0)
      {
         // On court-circuite des portions de code
         goto HELL;
      }

      // On rend le coup invalide par défaut
      CoupValide=0;

      // Tant que le coup n'est pas valide 
      while(CoupValide!=1)
      {
         // On entre un essai
         fflush(stdin);
         memset(str_envoi, 0, sizeof(str_envoi));
         printf("Veuillez entrer un chiffre : ");
         gets(str_envoi);

         // On envoie l'essai
         n_envoi=strlen(str_envoi);
         send(sd,str_envoi,n_envoi+1,0);

         // On attend la réponse
         n=0;
         n=recv(sd,BufIN,MTU,0);

         // Si la réponse est autre que BEGIN TURN ou non nulle
         if (n>0 && strcmp(BufIN, BegT)!=0)
         {
            // Balise du court-circuitage en cas de fin de partie
            HELL:

            BufIN[n]='\0';

            //Si, fonction de s'il on a gagné ou perdu
            if(strcmp(BufIN, EndW)==0)
            {
               for(int x=0;x<42;x++)
               {
                  Plateau[x/6][x%6]=BufIN[x+5];
               }
               // On affiche l'état final
               Display(Plateau);

               printf("Bravo, vous avez gagne ! \n");
               fin=1;
               CoupValide=1;
            }
            if(strcmp(BufIN, EndL)==0)
            {
               for(int x=0;x<42;x++)
               {
                  Plateau[x/6][x%6]=BufIN[x+5];
               }
               // On affiche l'état final
               Display(Plateau);

               printf("Helas, vous avez perdu !\n");
               fin=1;
               CoupValide=1;
            }

            //S'il y a eu erreur lors de la validité du coup
            if(strcmp(BufIN, ErrO)==0)
            {
               printf("Vous ne pouvez plus ajouter de jeton sur cette colonne !\n");
            }
            if(strcmp(BufIN, ErrR)==0)
            {
               printf("La colonne que vous avez indique n'existe pas !\n");
            }

            // Si le coup est validé
            if((strcmp(BufIN, EndT)==0))
            {
               printf("Coup enregistre, fin du tour\n\n");
               CoupValide=1;


               // On écrase notre plateau avec celui fourni par le serveur
               for(int x=0;x<42;x++)
               {
                  Plateau[x/6][x%6]=BufIN[x+5];
               }
               // On affiche l'état final
               Display(Plateau);
            }
         }
	   }
   }

   closesocket(sd); //fermeture de la socket

   printf("FIN DE LA PARTIE\n");

#ifdef WIN32
   WSACleanup();
#endif
   exit(0);
}
