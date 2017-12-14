/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>

//POUR MAC
//#include <sys/types.h>
// POUR LINUX - décommenter selon l'OS
#include <linux/types.h> 	/* pour les sockets */

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

Channel *listChannel;
Channel channel_info;


int ajouter_channel(int socket) { // retourne id channel
	Requete r;
	int found = 0;
	r.instruction = 2;
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

int is_in_list_channel(Channel* c) {
	Channel *aux;
	aux = listChannel;

	while (aux != NULL) {
		if(aux->id == c->id) {
			return 1;
		}

		aux = aux->suiv;
	}

	return 0;
}

/*void get_list_channel(int socket, int id_user) {
	Channel *c = (Channel*) malloc(sizeof(Channel));;
	Requete r;
	r.instruction = 5;

	//envoi du message vers le serveur 
	if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}

	printf("**************************************\n");
	printf("********** Channels obtenus **********\n");
	printf("**************************************\n");

	int encore = 1;
	while(encore == 1 && recv(socket, c, sizeof(*c),0) > 0) {
		if (is_in_list_channel(c) == 0) {
			Channel *new = (Channel*) malloc(sizeof(Channel)); // cr
			new->id = c->id;
			new->nb_client = c->nb_client;
			strcpy(new->nom, c->nom);

			printf("NOM : %s ", new->nom);
			printf(" ID : %d\n", new->id);

			if(new->id == 1) {
				encore = -1;
			}

			new->suiv = listChannel;
			listChannel = new;
		}
	}

	printf("Tapez l'id du channel que vous voulez rejoindre\n");
	int channelVoulu;
	scanf("%d", &channelVoulu);
	printf("Channel %d rejoint !\n", channelVoulu);

	// On  cherche le channel choisis dans le chainage
	int trouve = -1;
	Channel *courant = (Channel*) malloc(sizeof(Channel)); //crée new
	courant = listChannel;
	while(trouve == -1 && courant->suiv != NULL) {
		printf("test while : %s \n", courant->nom);
		if (channelVoulu == courant->id) {
			trouve = 1;
			channelChoisis = courant;
		} else {
			courant = courant->suiv;
		}
	}

	// L'id du channel
	r.id = courant->id;
	// L'instruction pour que le serveur sache quoi faire
	r.instruction = 8;
	// L'id du client, on le convertis d'int to string
  	sprintf(r.text, "%d", id_user);

  	//r.text = id_user +'0';
	printf("%s - %d - %d \n", r.text, r.instruction, r.id);

	// On demande à rejoindre le channel
	if ((send(socket, &r, sizeof(r),0)) < 0) { // message pour finir la connection avec le server
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}

	// On envoie le channel qu'on veut rejoindre
	if ((send(socket, channelChoisis, sizeof(*channelChoisis),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}

	printf("id : %d - nom: %s nbclients: %d \n", channelChoisis->id, channelChoisis->nom, channelChoisis->nb_client);

	if ((recv(socket, channelChoisis, sizeof(*channelChoisis),0)) > 0) {
		printf("Channel reçu ! \n");
	}
	printf("id : %d - nom: %s nbclients: %d \n", channelChoisis->id, channelChoisis->nom, channelChoisis->nb_client);

	envoi_message(id_user, courant->id, socket);
}

void envoi_message(int id_user, int id_channel, int socket_descriptor) {
	printf("**************************************\n");
	printf("**** Bienvenue dans le channel %d ****\n", id_channel);
	printf("**************************************\n");
	printf("* Tapez 'q:' pour quitter le channel\n");

	Client *listClient = (Client*) malloc(sizeof(Client)); //crée new
	listClient = channelChoisis->listClient;
	printf("Membres du channel\n");
	printf("Nombre de membres : %d\n", channelChoisis->nb_client);
	while (listClient != NULL) {
		printf("dans le while \n");
		printf("%s\n", listClient->pseudo);
		listClient = listClient->suiv;
	}

	int veutEcrire = 1;
	char message[256] = "";

	Requete r;
	r.instruction = 6;
	r.id = id_channel;

	Message *messageRecu;
	while(veutEcrire == 1) {

		r.instruction = 6;
		scanf("%s\n", &message);
		printf("le messsage entré est %s\n", message);

		if (strcmp(message, "q:") == 0) {
        printf("Au revoir !");
				veutEcrire = -1;
				//fin_connection(socket_descriptor);
    }  else {
				strcpy(r.text, message);
				// On envoie le message au serveur pour qu'il puisse l'envoyer à tous les membres
				if ((send(socket_descriptor, &r, sizeof(r), 0)) > 0) {
					printf("Message envoyé\n");
					strcpy(r.text, "chaine vidé");
					printf("%s\n", r.text);
				}

				if ((recv(socket_descriptor, &r, sizeof(r),0)) > 0) {
					printf("Message reçu ! %s\n", r.text);
				}
    }
	}
}*/

void fin_connection(int socket) {
	Requete r;
	r.instruction = 7;

	if ((send(socket, &r, sizeof(r),0)) < 0) { // message pour finir la connection avec le server
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
    }
}

int mode_channel(int socket){
	Requete r;
	int fin = 0;
	//Demande tout les messages deja existant
	r.instruction = 9;
	r.id = channel_info.id;
	int nb_mess;
	int last_id_mess;
	
	Message newMess;
	
	printf("**************************************\n");
	printf("**** Bienvenue dans le channel %s %d****\n", channel_info.nom, channel_info.id);
	printf("**************************************\n");
	printf("* Tapez 'q:' pour quitter le channel\n");
	
	
	while(fin == 0){
		//demande le dernier message
		r.id = 9;
		if ((send(socket, &r, sizeof(r),0)) < 0) {
			perror("erreur : impossible d'ecrire le message destine au serveur.");
			exit(1);
		}
		//recoit le dernier 
		if ((recv(socket, &newMess, sizeof(newMess),0)) > 0){
			if(newMess.id != -1 && newMess.id != last_id_mess){
				printf("%s\n", newMess.message);
				last_id_mess = newMess.id;
			}
		}
		
		
		scanf("%s", &r.text);
		if (strcmp(r.text, "q:") == 0) { //quitte le channel 
			return 1; //retourne a la selection des channels 
		}
		else { // envoie message
			r.instruction = 6;
			//le texte et l'id channel est deja renseigné
			if ((send(socket, &r, sizeof(r),0)) < 0) {
				perror("erreur : impossible d'ecrire le message destine au serveur.");
				exit(1);
			}
		}	
	}
	return 0;
}

int mode_selection_channel(int socket){
	int fin = 0;
	Channel newChan;
	int user_action = -3; 
	Requete r;
	//Demande tout les channels deja existant
	r.instruction = 5;
	int nb_chan;
	int last_id_chan;
	
	if ((send(socket, &r, sizeof(r),0)) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}
	
	if((recv(socket, &newChan, sizeof(newChan),0)) > 0 ){
		nb_chan = newChan.nb_client;
	}
	
	//recupère du serveur
	for(int i = 0; i < nb_chan; i++){
		if((recv(socket, &newChan, sizeof(newChan),0)) > 0 ){
			printf("%d - %s\n", newChan.id, newChan.nom);
			last_id_chan = newChan.id;
		}
	}
	
	printf("Entrer l'id du channel pour le rejoindre \n");
	printf("Entrer -2 pour ajouter un channel\n");
	printf("Entrer -1 pour quitter \n");
	
	while(fin == 0){
		//demande le dernier channel ajouté
		r.instruction = 4;
		if ((send(socket, &r, sizeof(r),0)) < 0) { 
			perror("erreur : impossible d'ecrire le message destine au serveur.");
		}
		//recois la reponse
		if ((recv(socket, &newChan, sizeof(newChan),0)) > 0) {
			if(newChan.id != -1){
				printf("%d - %s\n", newChan.id, newChan.nom);
			}
		}
	
		scanf("%d", &user_action);
		if(user_action > -3) {
			if (user_action == -1) { // quitte l'app
				fin_connection(socket);
				exit(0);
			}else if(user_action == -2){ // ajoute un channel
				ajouter_channel(socket);
			}
			else{ //join un channel
				r.instruction = 8;
				r.id = user_action;
				if ((send(socket, &r, sizeof(r),0)) < 0) { 
					perror("erreur : impossible d'ecrire le message destine au serveur.");
				}
				
				//assigne dans une variable globale pour acces plus tard 
				int trouve = 0;
				while(trouve == 0 ){
					if ((recv(socket, &channel_info, sizeof(channel_info),0)) > 0) {
						trouve = 1;
						if(channel_info.id != -1){
							return 2; // permet d'aller dans l'autre mode après 
						}
						else {
							printf("Pas de channel avec cet id \n");
						}
					}
				}
			}
		}
		
	}
	return 0;
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
	//Channel *listChannel = NULL; // liste des channels
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
	
	//lance la boucle
	int action = mode_selection_channel(socket_descriptor);
	while(action != 0){
		switch(action){
			case 1 : action = mode_selection_channel(socket_descriptor);break;
			case 2 : action = mode_channel(socket_descriptor);break;
			default : action = 0;
		}
	}
	
	printf("\nfin de la reception.\n");

	close(socket_descriptor);

	printf("connexion avec le serveur fermee, fin du programme.\n");

	exit(0);
}
