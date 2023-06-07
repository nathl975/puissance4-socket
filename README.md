# M1-Reseaux-Puissance4
[Nathan LETOURNEL](https://www.github.com/nathl975), [Elouan PRIME](https://www.github.com/ElouanP) - M1 MIAGE Alternant

## Description
Ce projet est un jeu _Puissance 4_ programmé en C utilisant l'architecture client-serveur.
Il utilise les sockets pour interagir entre les clients et gérer les évènements de partie.

## Prérequis
Ce jeu a été conçu pour fonctionner sur Linux, utilisant les librairies POSIX. 
De plus, il nécessite que le compilateur GCC soit installé (ou équivalent) afin de compiler les fichiers sources du projet.

## Installation
Pour commencer à utiliser le projet, il faut tout d'abord télécharger les sources. Si vous disposez d'une clé SSH valide, 
vous pouvez utiliser la commande suivante.
```shell
git clone git@github.com:nathl975/puissance4-socket.git
```
Alternativement, vous pouvez cloner les sources du projet en HTTPS avec la commande suivante.
```shell
git clone https://github.com/nathl975/puissance4-socket.git
```
Il faut maintenant compiler les sources du projet, pour le client et le serveur en utilisant la commande suivante.
```shell
gcc src/client.c -o client ; gcc -pthread src/server.c -o server
```
Une fois que le client et le serveur sont compilés, vous pouvez maintenant jouer.

## Utilisation
Pour jouer, il faut lancer le serveur sur un port non-utilisé (ex: 5000). Ensuite, le serveur va attendre que 2 clients se connectent pour lancer une première partie.
Ensuite le lancement s'effectue par paires de 2 joueurs, le premier attend toujours qu'un deuxième se connecte, une fois celà la partie se lance automatiquement.
```shell
./server 5000
```
Pour connecter un client, lancer le programme client en spécifiant le même port que celui du serveur.
```shell
./client 5000
```
Une fois la partie lancée, le jeu se déroule comme _Puissance 4_.

À noter qu'il existe une fonction de debug au programme, en ajoutant 1 en 3e argument à l'exécution.
Celle-ci permet d'avoir davantage d'informations sur l'exécution des parties.
```shell
./server 5000 1
```
```shell
./client 5000 1
```

## Arborescence
## src
Ce répertoire contient les sources du projet, notamment les fichiers à compiler pour l'exécution
* [server.c](src/server.c)
* [client.c](src/client.c)

Il contient également un fichier de headers qui contient tous les éléments partagés par les 2 fichiers comme les constantes de préprocesseur ou les fonctions d'utilité aux 2 fichiers.
* [config.h](src/config.h)
## docs
Ce répertoire contient des fichiers de documentation ou d'exemple que nous avons pu trouver.
* [P4_Server.c](docs/P4_Server.c)
* [P4_Client.c](docs/P4_Client.c)

Ces 2 fichiers sont des exemples que nous avons trouvés sur [GitHub](https://github.com/nikanorivanovitch/Puissance-4) d'un jeu de _Puissance 4_ rédigé de manière similaire à nos consignes.
Nous nous en sommes donc inspiré pour comprendre comment faire fonctionner le jeu. Notre fonctionnement reste tout de même largement différent du leur, notamment sur l'exécution des threads,
nous gérons un thread pour 2 joueurs et ainsi un par partie là où leur gestion se fait par joueur et donc nécessite des sémaphores pour communiquer.
* [puissance4.c](docs/puissance4.c)

Ce dernier fichier est un exemple de _Puissance 4_ qui a été rédigé à l'IUT par Nathan et que nous avons utilisé pour créer cette version multijoueur.
Ce fichier est compilable et jouable, mais ne s'exécute que dans un seul Terminal. Il nous a été utile pour récupérer la logique du jeu _Puissance 4_ (victoire, égalité...).