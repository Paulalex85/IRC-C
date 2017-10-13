/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

//TYPES

typedef struct Client {
	struct Client *suiv;
	int id;
	char pseudo[30]; //30 carac max pour le pseudo
} Client;

typedef struct Message {
	struct Message *suiv;
	int id;
	int  id_client; //identifie l'user qui a envoyé le msg
	char message[256]; //longueur max de 256 carac
} Message;

typedef struct Channel {
	struct Channel *suiv;
	Client *listClient; // liste des membres du channel
	Message *listMessage; // les msg du channel
	char nom[30];
	int id;
	int nb_client; 
} Channel;

typedef struct Requete { // struct a echanger avec client
	int instruction;
	char text[256];
	int id;
} Requete;

//id des requetes
// 1 : pseudo user -> création server 
// 2 : creation channel
// 3 : join channel
// 4 : leave channel

void ajouter_channel(int id_user) {
	printf("Nom Channel? \n");

	Requete r;
	r.instruction = 2;
	r.id = id_user;
	scanf("%s", &r.text);
}

const char* creation_user(int *id_user) {
	printf("Pseudo? \n");

	Requete r;
	r.instruction = 1;
	r.id = 0;
	scanf("%s", &r.text);
	
	char pseudo[30];
	strcpy(pseudo, r.text); 
      
    /* envoi du message vers le serveur */
    if ((send(socket_descriptor, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
    }
     
    printf("message envoye au serveur. \n");
                
    /* lecture de la reponse en provenance du serveur */

	if ((recv(socket_descriptor, &r, sizeof(r),0)) > 0) {
		*id_user = r.id;
	}
	return pseudo;
}

int main(int argc, char **argv) {
  
    int 	socket_descriptor, 	/* descripteur de socket */
		longueur; 		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    servent *	ptr_service; 		/* info sur service */
    char 	buffer[256];
    char *	prog; 			/* nom du programme */
    char *	host; 			/* nom de la machine distante */
    char *	mesg; 			/* message envoyé */
	char	pseudo[30]; // pseudo de l'utilisateur

	Message *listMessage; // messages du channel rejoind
	Channel *listChannel; // liste des channels
	int id_user; //id de l'utilisateur 
     
    if (argc != 2) {
	perror("usage : client <adresse-serveur>");
	exit(1);
    }
   
    prog = argv[0];
    host = argv[1];
    
    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);
    
    if ((ptr_host = gethostbyname(host)) == NULL) {
	perror("erreur : impossible de trouver le serveur a partir de son adresse.");
	exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    
    /*-----------------------------------------------------------*/
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
	perror("erreur : impossible de recuperer le numero de port du service desire.");
	exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("erreur : impossible de creer la socket de connexion avec le serveur.");
	exit(1);
    }
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
    }
    
    printf("connexion etablie avec le serveur. \n");
    
    pseudo = creation_user(&id_user);
    
    printf("\nfin de la reception.\n");
    
    close(socket_descriptor);
    
    printf("connexion avec le serveur fermee, fin du programme.\n");
    
    exit(0);
    
}
