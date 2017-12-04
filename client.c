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
// 5 : get channels
// 6 : send message

int ajouter_channel(int id_user, int socket) { // retourne id channel
	Requete r;
	int found = 0;
	r.instruction = 2;
	r.id = id_user;
	printf("Nom Channel? \n");
	scanf("%s", &r.text);

	/* envoi du message vers le serveur */
  if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
  }

  printf("ajout envoye au serveur. \n");

	r.id = -1;
	/* lecture de la reponse en provenance du serveur */
	while (found == 0){
		if ((recv(socket, &r, sizeof(r),0)) > 0) {
			found = 1;
			if(r.id != -1){
				printf("Channel cree\n");
			}
			else {
				printf("probleme de creation du channel");
			}
		}
	}

	return r.id;
}

int is_in_list_channel(Channel c, Channel* listChannel) {
	Channel *aux;
	aux = listChannel;

	while (aux != NULL) {
		if(aux->id == c.id) {
			return 1;
		}

		aux = aux->suiv;
	}

	return 0;
}

void get_list_channel(Channel *listChannel, int socket) {
	Channel c;
	Requete r;
	r.instruction = 5;

	/* envoi du message vers le serveur */
	if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}

	while(recv(socket, &c, sizeof(c),0) > 0) {
		if (is_in_list_channel(c,listChannel) == 0) {
			Channel *new = (Channel*) malloc(sizeof(Channel)); // cr
			new->id = c.id;
			new->nb_client = c.nb_client;
			strcpy(new->nom, c.nom);

			new->suiv = listChannel;
			listChannel = new;
		}
	}
}

void afficher_channel(int id_user, int socket_descriptor, Channel* listChannel) {

	Requete r;
	r.instruction = 5;

	/* envoi du message vers le serveur */
 	if ((send(socket_descriptor, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}

  Channel *courant;
	courant = listChannel;
	printf("Voici les différents channels possibles.\n");
	printf("Tapez le numéro du channel que vous voulez rejoindre.\n");

	while(courant != NULL) {
		printf("test\n");
		printf("%s - ", courant->id);
		printf("%s \n", courant->nom);
		courant = courant->suiv;
	}
}

void envoi_message(int id_user,int socket_descriptor) {
	printf("Tapez votre message.\n");

	char message;
	scanf("%d", &message);

	Requete r;
	r.instruction = 6;
	// r.text = message;
	strcpy(r.text , message);

	if ((send(socket, &r, sizeof(r),0)) < 0) { // message pour finir la connection avec le server
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
  }

}

void fin_connection(int socket) {
	Requete r;
	r.instruction = 7;

	if ((send(socket, &r, sizeof(r),0)) < 0) { // message pour finir la connection avec le server
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
  }
}

const char* creation_user(int socket, int *id_user) {
	printf("Pseudo? \n");

	Requete r;
	r.instruction = 1;
	r.id = -1;
	scanf("%s", &r.text);

	char pseudo[30];
	strcpy(pseudo, r.text);

  /* envoi du message vers le serveur */
  if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
  }

  printf("pseudo envoye au serveur. \n");

  /* lecture de la reponse en provenance du serveur */
	while (r.id == -1){
		if ((recv(socket, &r, sizeof(r),0)) > 0) {
			*id_user = r.id;
		}
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
	const char * pseudo; // pseudo de l'utilisateur

	Message *listMessage = NULL; // messages du channel rejoind
	Channel *listChannel = NULL; // liste des channels
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

  pseudo = creation_user(socket_descriptor, &id_user);
	printf("id user : %d \n", id_user);

	printf("Action ?\n");
	printf("0 - Quitter\n");
	printf("1 - Ajouter channel\n");
	printf("2 - Voir les channels\n");

	int value_user;
	scanf("%d", &value_user);

	switch(value_user){
		case 0:
			fin_connection(socket_descriptor);
			break;
		case 1:
			if(ajouter_channel(id_user,socket_descriptor) == 1)
			{
				printf("ajouter suite\n");
			}
			break;
		case 2:
			afficher_channel(id_user, socket_descriptor, listChannel);
		break;
		default: break;
	}

  printf("\nfin de la reception.\n");

  close(socket_descriptor);

  printf("connexion avec le serveur fermee, fin du programme.\n");

  exit(0);
}
