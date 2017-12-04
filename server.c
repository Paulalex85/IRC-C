/*----------------------------------------------
Serveur à lancer avant le client
gcc IRC-C/server.c -lpthread -o server
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */
#include <pthread.h>
#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

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


void nouveau_client(Client *list,int *nbclient, char pseudo[], int socket)
{
	Client *new = (Client*) malloc(sizeof(Client)); //crée new
	*nbclient = (*nbclient) +1; //incrémente l'id
	new->id = *nbclient; // assigne l'id
	strcpy(new->pseudo, pseudo); // copie le pseudo

	if(list == NULL) {
		new->suiv = NULL;
	}
	else{
		new->suiv = list; // on pointe le premier de la liste dans le suivant du nouveau
	}
	list = new; // on fait pointer le début de la liste sur le nouveau

	Requete r;
	r.instruction = 1;
	r.id = new->id;

	if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
  }
	printf("ajout de l'user : %s id: %d \n", pseudo, new->id);
}

void supprimer_client(Client *list,int id_client)
{
	Client *aux;
	Client *pre;
	if(list != NULL) {
		if(list->id == id_client) {
			aux = list;
			list= list->suiv;
			free(aux);
		}
		else {
			aux = list->suiv;
			pre = list;
			while(aux != NULL) {
				if(aux->id == id_client){
					pre->suiv = aux->suiv;
					free(aux);
					break;
				}
				else{
					pre = aux;
					aux = aux->suiv;
				}
			}
		}
	}
}

void creer_channel(Channel *list,int *nbchannel, char nom[], int socket)
{
	int id_new = -1;
	Channel *new = (Channel*) malloc(sizeof(Channel)); //crée new
	id_new = (*nbchannel) + 1; //incrémente l'id
	*nbchannel = id_new; //assigne a la valeur de programme principal

	new->id = id_new; // assigne l'id
	new->nb_client = 0;
	strcpy(new->nom, nom); // copie le nom
	printf("Le précédent channel a pour nom %s\n", list->nom);

	new->suiv = list; // on pointe le premier de la liste dans le suivant du nouveau
	list = new; // on fait pointer le début de la liste sur le nouveau
	printf("ajout du channel %s ok\n", list->nom);

	//envoie au client
	Requete r;
	r.id = new->id;
	if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au client.");
		exit(1);
  	}
	printf("ajout du channel : %s id: %d \n", new->nom, new->id);
}

int ajout_client_channel(Channel *channel, Client *list, int id_client) // retourne 1 ou 0 si fait ou non
{
	int fait = 0;
	Client *move = NULL;
	Client *aux;
	Client *pre;

	if(list != NULL) {
		if(list->id == id_client) {
			move = list;//transfert
			list = list->suiv;//enleve le client de la list générale
		}
		else{
			pre = list;
			aux = list->suiv;
			while(aux != NULL){
				if(aux->id == id_client){
					move = aux;//transfert
					pre->suiv = aux->suiv; //enleve de la liste
					break;
				}
				else {
					pre = aux;
					aux = aux->suiv;
				}
			}
		}

		if(move != NULL) {
			fait = 1;
			aux = (*channel).listClient; //prend la tete de la liste du channel
			move->suiv = aux; //affecte la liste apres le client a ajouter
			(*channel).listClient = move;
		}
	}

	return fait;
}

void supprimer_channel(Channel *list, int* nbchannel, int id_channel)
{
	Channel *aux;
	Channel *pre;

	if(list != NULL) {
		if(list->id == id_channel) {
			aux = list;
			list= list->suiv;
			free(aux);
		}
		else {
			aux = list->suiv;
			pre = list;
			while(aux != NULL) {
				if(aux->id == id_channel){
					pre->suiv = aux->suiv;
					free(aux);
				}
				else{
					pre = aux;
					aux = aux->suiv;
				}
			}
		}
	}
}

void send_channels(Channel *list, int* nbchannel, int socket_descriptor) {
	printf("test\n");
	Channel *courant = (Channel*) malloc(sizeof(Channel)); //crée new
	courant = list;
	/*
	if (nbchannel > 0) {
		Requete r;
		r.id = nbchannel;
		if ((send(socket_descriptor, &r, sizeof(r),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au client.");
			exit(1);
  		}
	}
	*/
	if(courant != NULL) {
		//envoie au client
		printf("test dans le courant\n");
		printf("L'id du channel est %d\n", courant->id);
		printf("Le nom du channel est %s\n", courant->nom);
		printf("L'id du channel est %d\n", list->id);
		printf("Le nom du channel est %s\n", list->nom);

		Requete r;
		r.id = courant->id;
		if ((send(socket_descriptor, &r, sizeof(r),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au client.");
			exit(1);
	  	}

	  	courant = courant->suiv;
	}

}

void ajouter_message(Channel *list, int clientId, char contenu, int socket ) {

	Message* newMessage = (Message*) malloc(sizeof(Message));

	Message* listMessage = list->listMessage;
	int lastId =listMessage->id;
	newMessage->id = lastId + 1;

	strcpy(newMessage->message, contenu);
	//newMessage->message = contenu;
	newMessage->id_client = clientId;
	listMessage->suiv = newMessage; // Le nouveau message devient le dernier de la list
	newMessage->suiv = NULL;
}

void supprimer_message(Message *list) //supprime le premier de la liste
{
	Message *aux = list;
	list = list->suiv;
	free(aux);
}

/*------------------------------------------------------*/
void *gestion_message (void * arg) {
//int sock, Client *listClient, int *nbclient, Channel *listChannel, int *nbchannel
	void** realData = (void**)arg;
	void* p1 = realData[0];
	void* p2 = realData[1];
	void* p3 = realData[2];
	void* p4 = realData[3];
	void* p5 = realData[4];

	Requete r;

	int sock = *(int*)p1;
	Client *listClient = (Client*)p2;
	int *nbclient = (int*)p3;
	Channel *listChannel = (Channel*)p4;
	int *nbchannel = (int*)p5;

	int i = 1;
	while (i == 1) {
		if(recv(sock, &r, sizeof(r),0) > 0) { // assign la requete a
			printf("Requete de type %d \n", r.instruction);

			switch(r.instruction) {
				case 1:
					nouveau_client(listClient, nbclient, r.text, sock);
					break;
				case 2:
					creer_channel(listChannel, nbchannel, r.text, sock);
					break;
				case 5:
					send_channels(listChannel, nbchannel, sock);
					 break;
				case 6:
				 	ajouter_message(listChannel, r.id, r.text, sock);
				case 7: //Id d'instruction pour fermer la connection
					close(sock);
					pthread_exit(NULL);
					i = 0;
					break;
				default:
					i = 0;
					break;
			}
		}
	}
}
/*------------------------------------------------------*/

/*------------------------------------------------------*/
int main(int argc, char **argv) {

	int socket_descriptor,		/* descripteur de socket */
		nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
		longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */

	sockaddr_in adresse_locale, 		/* structure d'adresse locale*/
		adresse_client_courant; 	/* adresse client courant */

	hostent* ptr_hote; 			/* les infos recuperees sur la machine hote */
	servent* ptr_service; 			/* les infos recuperees sur le service de la machine */
	char machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */

	gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */

	Client *listClient = (Client*) malloc(sizeof(Client));
	Channel *listChannel = (Channel*) malloc(sizeof(Channel));

	int nb_messages = 0;
	int nb_channels = 0;
	int nb_clients = 0;

  /* recuperation de la structure d'adresse en utilisant le nom */
  if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
  }

  /* initialisation de la structure adresse_locale avec les infos recuperees */

  /* copie de ptr_hote vers adresse_locale */
  bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
  adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
  adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */

  /*-----------------------------------------------------------*/
  /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
  /* Utilisation de la solution 1 car application d'un service deja existant */

  if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
		perror("erreur : impossible de recuperer le numero de port du service desire.");
		exit(1);
  }

  adresse_locale.sin_port = htons(ptr_service->s_port);
  printf("numero de port pour la connexion au serveur : %d \n",
	   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);

  /* creation de la socket */
  if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
  }

  /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
  if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
  }

  /* initialisation de la file d'ecoute */
  listen(socket_descriptor,5);

  /* attente des connexions et traitement des donnees recues */
  for(;;) {
		longueur_adresse_courante = sizeof(adresse_client_courant);

		/* adresse_client_courant sera renseigné par accept via les infos du connect */
		if ((nouv_socket_descriptor =
			accept(
				socket_descriptor,
			  (sockaddr*)(&adresse_client_courant),
			  &longueur_adresse_courante)
			)
			 < 0)
		{
			perror("erreur : impossible d'accepter la connexion avec le client.");
			exit(1);
		}

		/* traitement du message */
		printf("reception d'un message.\n");

		pthread_t thread;
		void *ptr[5];
		ptr[0] = &nouv_socket_descriptor;
		ptr[1] = listClient;
		ptr[2] = &nb_clients;
		ptr[3] = listChannel;
		ptr[4] = &nb_channels;

		if (pthread_create(&thread, NULL, gestion_message, ptr) == -1){
			perror("pthread_create");
		}
		//gestion_message(nouv_socket_descriptor, listClient, &nb_clients,listChannel, &nb_channels);
		//close(nouv_socket_descriptor);
  }
}
