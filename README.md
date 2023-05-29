# M1-Reseaux-Puissance4
[Nathan LETOURNEL](https://www.github.com/nathl975), Elouan PRIME - M1 MIAGE Alternant

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
Pour jouer, il faut lancer le serveur sur un port non-utilisé (ex: 5000). Ensuite, le serveur va attendre que 2 clients se connectent pour lancer la partie.
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
Nous nous en sommes donc inspiré pour comprendre comment faire fonctionner le jeu.
* [puissance4.c](docs/puissance4.c)

Ce dernier fichier est un exemple de _Puissance 4_ que j'ai rédigé à l'IUT et que nous avons utilisé pour créer cette version multijoueur. Ce fichier est compilable et jouable, mais ne s'exécute que dans un seul Terminal.