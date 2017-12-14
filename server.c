/*----------------------------------------------
Serveur à lancer avant le client
gcc IRC-C/server.c -lpthread -o server
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>

//POUR MAC
//#include <sys/types.h>
// POUR LINUX - décommenter selon l'OS
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
	int socket;
} Client;

typedef struct Message {
	struct Message *suiv;
	int id;
	int  id_client; //identifie l'user qui a envoyé le msg
	char message[256]; //longueur max de 256 carac
} Message;

typedef struct Channel {
	struct Channel *suiv;
	Client *list_client; // liste des membres du channel
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

//variables globales ftw
Client *listClient;
Channel *listChannel;
int nb_clients, nb_channels, nb_messages;

Client* get_client_by_socket(int sock){
	Client* current = listClient;
	while(current != NULL){
		if(current->socket == sock){
			return current;
		}
		else{
			current = current->suiv;
		}
	}
	return NULL;
}

Channel* get_channel_by_id(int id){
	Channel* current = listChannel;
	while(current != NULL){
		if(current->id == id){
			return current;
		}else{
			current= current->suiv;
		}
	}
	return NULL;
}


void nouveau_client(char pseudo[], int socket)
{
	Client *new = (Client*) malloc(sizeof(Client)); //crée new
	nb_clients = nb_clients +1; //incrémente l'id
	new->id = nb_clients; // assigne l'id
	new->socket = socket;
	strcpy(new->pseudo, pseudo); // copie le pseudo

	if(listClient == NULL) {
		new->suiv = NULL;
	}
	else{
		new->suiv = listClient; // on pointe le premier de la liste dans le suivant du nouveau
	}
	listClient = new; // on fait pointer le début de la liste sur le nouveau

	Requete r;
	r.instruction = 1;
	r.id = new->id;

	if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
  }
	printf("ajout de l'user : %s id: %d \n", pseudo, new->id);
}

//Quand deconnection
void supprimer_client(int id_client)
{
	Client *aux;
	Client *pre;
	if(listClient != NULL) {
		if(listClient->id == id_client) {
			aux = listClient;
			listClient= listClient->suiv;
			nb_clients--;
			free(aux);
		}
		else {
			aux = listClient->suiv;
			pre = listClient;
			while(aux != NULL) {
				if(aux->id == id_client){
					pre->suiv = aux->suiv;
					nb_clients--;
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

void creer_channel(char nom[], int socket)
{
	int id_new = -1;
	Channel *new = (Channel*) malloc(sizeof(Channel)); //crée new
	id_new = nb_channels + 1; //incrémente l'id
	nb_channels = id_new; //assigne a la valeur de programme principal

	new->id = id_new; // assigne l'id
	new->nb_client = 0;
	strcpy(new->nom, nom); // copie le nom
	new->suiv = NULL;

	//si y a pas de channel on le met directement 
	if(listChannel == NULL){
		listChannel = new;
	}
	else
	{
		new->suiv = listChannel; // on pointe le premier de la liste dans le suivant du nouveau
		listChannel = new;
	}

	//envoie au client
	Requete r;
	r.id = new->id;
	if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au client.");
		exit(1);
  	}
	printf("ajout du channel : %s id: %d \n", new->nom, new->id);
}

int ajout_client_channel(int id_client) // retourne 1 ou 0 si fait ou non
{
	//TODO voir INCREMENTATION
	int fait = 0;
	Client *move = NULL;
	Client *aux;
	Client *pre;
	//on boucle tout les clients 
	if(listClient != NULL) {
		if(listClient->id == id_client) { //si on a le meme id
			move = listClient;//transfert
			listClient = listClient->suiv;//enleve le client de la list générale
		}
		else{
			pre = listClient;
			aux = listClient->suiv;
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
			aux = listChannel->list_client; //prend la tete de la liste du channel
			move->suiv = aux; //affecte la liste apres le client a ajouter
			listChannel->list_client = move;
			listChannel->nb_client++;
		}
	}

	return fait;
}

void supprimer_channel(int id_channel)
{
	Channel *aux;
	Channel *pre;

	if(listChannel != NULL) {
		if(listChannel->id == id_channel) {
			aux = listChannel;
			listChannel= listChannel->suiv;
			nb_channels--;
			free(aux);
		}
		else {
			aux = listChannel->suiv;
			pre = listChannel;
			while(aux != NULL) {
				if(aux->id == id_channel){
					pre->suiv = aux->suiv;
					nb_channels--;
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

void send_channels(int socket_descriptor) {
	Channel *courant = listChannel;
	Channel infos_chan;
	infos_chan.nb_client = nb_channels;
	printf("il y a %d channels", nb_channels);
	
	//permet de savoir combien de channel sont existant
	if ((send(socket_descriptor, &infos_chan, sizeof(infos_chan),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au client.");
		exit(1);
	}

	printf("**************************************\n");
	printf("********* Channels existants *********\n");
	printf("**************************************\n");
	while(courant != NULL){
		//envoie au client
		printf("L'id du channel est %d\n", courant->id);
		printf("Le nom du channel est %s\n", courant->nom);

		if ((send(socket_descriptor, courant, sizeof(*courant),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au client.");
			exit(1);
	  	}
		courant = courant->suiv;
	}
}

void rejoindre_channel(int sock, int id_channel) {
	Client* courant_cli = get_client_by_socket(sock);
 
 	Channel *courant_chan = listChannel;
 	// Ajouter le client au channel VOULU
 	while (courant_chan != NULL) {
 		if(courant_chan->id == id_channel) {//trouve
 		
 			//Supprime le client de la liste principale
 			Client* aux = listClient;
 			Client* pre;
 			if( aux->socket == courant_cli->socket){//verifie le premier
 				listClient = courant_cli->suiv;
 			}
 			else{
 				pre = aux;
 				aux = aux->suiv;
 				while(aux != NULL){
 					if(aux->socket == courant_cli->socket){//trouve
 						pre->suiv = aux->suiv; //saute de la liste
 						break;
 					}
 					else {
 						pre = aux;
 						aux = aux->suiv;
 					}
 				}
 			}
 			
 			//ajoute le client au channel
 			Client* list_client_chan = courant_chan->list_client;
 			if(list_client_chan ==NULL){
 				courant_chan->list_client = courant_cli;
 			}else{//insertion tete
 				courant_cli->suiv = courant_chan->list_client;
 				courant_chan->list_client = courant_cli;
 			}
 			courant_chan->nb_client++;
 			printf("%s a rejoint le channel %s\n",courant_cli->pseudo, courant_chan->nom);
 			break;
 		} else {
 			courant_chan = courant_chan->suiv;
 		}
 	}
	
	if(courant_chan == NULL){
		Channel channel_vide;
		channel_vide.id = -1;
		if ((send(sock, &channel_vide, sizeof(channel_vide),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au client.");
			exit(1);
		}
	}
	else{
		//envoie les infos du channel au client
		if ((send(sock, courant_chan, sizeof(*courant_chan),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au client.");
			exit(1);
		}
	}
}

void envoyer_list_message(int id_channel , int socket) {
	int nb_message = 0;
	Channel* current = listChannel;
	Message* mess;
	Message message_infos;
	Message* aux;
//on cherche le channel
	/*current = get_channel_by_id(id_channel);
	mess = current->listMessage;*/
	//une fois cela fait on parcour la liste pour avoir le nombre de message dans la liste
	/*while(mess != NULL){
		nb_message++;
		mess = mess->suiv;
	}*/
	
	//printf("test1");
	//envoie le nombre de message a recevoir au client
	/*message_infos.id = nb_message;
	if ((send(socket, &message_infos, sizeof(message_infos),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au client.");
		exit(1);
	}
	printf("test2");
	mess = current->listMessage;
	int compteur_aux;
	//on remonte la liste et envoie tout les messages
	while(nb_message > 0){
		compteur_aux = 1;
		while (compteur_aux != nb_message){
			compteur_aux++;
			mess = mess->suiv;
		}
		if ((send(socket,mess, sizeof(*mess),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au client.");
			exit(1);
		}
		mess = current->listMessage;
		nb_message--;
	}*/
			

}

void get_last_message(int id_channel , int sock){
	Channel *current = get_channel_by_id(id_channel);
	Message mess;
	if(current->listMessage == NULL )
	{
		mess.id = -1;
	}
	else
	{
		mess.id = current->listMessage->id;
		strcpy(mess.message,current->listMessage->message);
	}
	
	if ((send(sock, &mess, sizeof(mess),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au client.");
		exit(1);
  	}
}

void ajouter_message(int id_channel, char contenu[], int socket) {
	
	Channel * current_chan = get_channel_by_id(id_channel);
	Message* newMessage = (Message*) malloc(sizeof(Message));
	//copie des infos dans le message
	strcpy(newMessage->message, contenu);
	nb_messages++;
	newMessage->id = nb_messages;
	
	//ajout du message dans la liste
	Message* list_message = current_chan->listMessage;
	if(list_message == NULL){
		current_chan->listMessage = newMessage;
	}else{
		newMessage->suiv = current_chan->listMessage;
		current_chan->listMessage = newMessage;
	}
	
	//envoie a tout les membres du channel
	Client * client_to_send = current_chan->list_client;
	while(client_to_send != NULL){
		if ((send(client_to_send->socket, newMessage, sizeof(*newMessage),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au client.");
			exit(1);
	  	}
	  	client_to_send = client_to_send->suiv;
 	}
}

void get_last_channel(int last_channel, int sock) {
	Channel chan;
	if(listChannel == NULL || listChannel->id == last_channel)
	{
		chan.id = -1;
	}
	else
	{
		chan.id = listChannel->id;
		strcpy(chan.nom,listChannel->nom);
	}
	
	if ((send(sock, &chan, sizeof(chan),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au client.");
		exit(1);
  	}
}

void supprimer_message(Message *list) //supprime le premier de la liste
{
	Message *aux = list;
	list = list->suiv;
	free(aux);
}

/*------------------------------------------------------*/
void *gestion_message (void * arg) {
	//int sock
	void** realData = (void**)arg;
	void* p1 = realData[0];

	Requete r;

	int sock = *(int*)p1;

	int i = 1;
	while (i == 1) {
		if(recv(sock, &r, sizeof(r),0) > 0) { // assign la requete a r
			printf("Requete de type %d \n", r.instruction);

			switch(r.instruction) {
				case 1:
					nouveau_client(r.text, sock);
					break;
				case 2:
					creer_channel(r.text, sock);
					break;
				case 4:
					get_last_channel(r.id,sock);
					break;
				case 5:
					send_channels(sock);
					 break;
				case 6:
				 	ajouter_message(r.id, r.text, sock);
					break;
				case 7: //Id d'instruction pour fermer la connection
					close(sock);
					pthread_exit(NULL);
					i = 0;
					break;
				case 8:
					rejoindre_channel(sock, r.id);
					break;
				case 9:
					get_last_message(r.id, sock);
					break;
				case 10:
					envoyer_list_message(r.id,sock);
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

	//init variables globales
	nb_channels = 0;
	nb_clients = 0;
	nb_messages = 0;

  /* recuperation de la structure d'adresse en utilisant le nom */
	if ((ptr_hote = gethostbyname("127.0.0.1")) == NULL) {
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
		void *ptr[1];
		ptr[0] = &nouv_socket_descriptor;

		if (pthread_create(&thread, NULL, gestion_message, ptr) == -1){
			perror("pthread_create");
		}
		//gestion_message(nouv_socket_descriptor, listClient, &nb_clients,listChannel, &nb_channels);
		//close(nouv_socket_descriptor);
  }
}
