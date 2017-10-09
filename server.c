/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
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
	int id;
	Client *client; //identifie l'user qui a envoyé le msg
	char message[256]; //longueur max de 256 carac
} Message;

typedef struct Channel {
	struct Channel *suiv;
	Client *listMembre; //les membres du channel
	Message *listMessage; // les msg du channel
	char nom[30];
	int id;
} Channel;

typedef struct Requete { // struct a echanger avec client
	int instruction;
	char text[256];
	int id;
} Requete;
	
	
void nouveau_client(Client *list,int *nbclient, char pseudo[])
{
	Client *new = (Client*) malloc(sizeof(Client)); //crée new
	*nbclient = nbclient++; //incrémente l'id
	new->id = *nbclient; // assigne l'id
	strcpy(new->pseudo, pseudo); // copie le pseudo
	if(list == NULL) {
		new->suiv = NULL;
	else{
		new->suiv = list; // on pointe le premier de la liste dans le suivant du nouveau
	}
	list = new; // on fait pointer le début de la liste sur le nouveau
	printf("ajout de l'user ok\n");
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
				}
				else{
					pre = aux;
					aux = aux->suiv;
				}
			}
		}
	}
}

int creer_channel(Channel *list,int *nbchannel char nom[]) //retourne l'id du channel
{
	int id_new = -1;
	Channel *new = (Channel*) malloc(sizeof(Channel)); //crée new
	id_new = (*nbchannel) + 1; //incrémente l'id
	*nbchannel = id_new; //assigne a la valeur de programme principal
	new->id = id_new; // assigne l'id
	strcpy(new->nom, nom); // copie le nom
	new->suiv = list; // on pointe le premier de la liste dans le suivant du nouveau
	list = new; // on fait pointer le début de la liste sur le nouveau
	printf("ajout de l'user ok\n");
	return id_new;
}

void supprimer_channel(Channel *list,int id_channel)
{
	Client *aux;
	Client *pre; 
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

void supprimer_message(Message *list)//supprime le premier de la liste 
{
	Message *aux = list;
	list = list->suiv;
	free(aux); 
}

/*------------------------------------------------------*/
void gestion_message (int sock, Client *listClient, int *nbclient) {

	Requete r;
	if(recv(sock, &r, sizeof(r),0) <=0) // assign la requete a r
		return;

	switch(r.instruction) {
		case 1:
			nouveau_client(listClient, nbclient, r.text);
			break;
		default:
			return;
	}
        
    return;
}
/*------------------------------------------------------*/

/*------------------------------------------------------*/
main(int argc, char **argv) {
  
    int 		socket_descriptor, 		/* descripteur de socket */
			nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    
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
			accept(socket_descriptor, 
			       (sockaddr*)(&adresse_client_courant),
			       &longueur_adresse_courante))
			 < 0) {
			perror("erreur : impossible d'accepter la connexion avec le client.");
			exit(1);
		}
		
		/* traitement du message */
		printf("reception d'un message.\n");

		gestion_message(nouv_socket_descriptor, listClient, &nb_clients);
						
		close(nouv_socket_descriptor);
		
    }
    
}

