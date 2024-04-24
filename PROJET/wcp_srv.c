/* Travail qui appartient à l'étudiant : DJEROUA Mustapha Hacène 12105968*/

/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <errno.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s repertoire_comptines\n"
			"serveur pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s comptines\n", nom_prog, nom_prog);
}
/** Retourne en cas de succès le descripteur de fichier d'une socket d'écoute
 *  attachée au port port et à toutes les adresses locales. */
int creer_configurer_sock_ecoute(uint16_t port);

/** Écrit dans le fichier de desripteur fd la liste des comptines présents dans
 *  le catalogue c comme spécifié par le protocole WCP, c'est-à-dire sous la
 *  forme de plusieurs lignes terminées par '\n' :
 *  chaque ligne commence par le numéro de la comptine (son indice dans le
 *  catalogue) commençant à 0, écrit en décimal, sur 6 caractères
 *  suivi d'un espace
 *  puis du titre de la comptine
 *  une ligne vide termine le message */
void envoyer_liste(int fd, struct catalogue *c);

/** Lit dans fd un entier sur 2 octets écrit en network byte order
 *  retourne : cet entier en boutisme machine. */
uint16_t recevoir_num_comptine(int fd);

/** Écrit dans fd la comptine numéro ic du catalogue c dont le fichier est situé
 *  dans le répertoire dirname comme spécifié par le protocole WCP, c'est-à-dire :
 *  chaque ligne du fichier de comptine est écrite avec son '\n' final, y
 *  compris son titre, deux lignes vides terminent le message */
void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic);

int main(int argc, char *argv[])
{
	/*if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}*/
	int sockfd = creer_configurer_sock_ecoute(PORT_WCP);

	printf("Serveur en attente de connexions\n");
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	struct catalogue * c = creer_catalogue("comptines");
	int client_sockfd = accept(sockfd,(struct sockaddr *)&client_addr,&client_addr_len);
	envoyer_liste(client_sockfd,c);
	printf("J'attends la reception du numéro\n :");
	uint16_t recv_num = recevoir_num_comptine(client_sockfd);
	envoyer_comptine(client_sockfd,"comptines",c,recv_num);
	liberer_catalogue(c);
	close(client_sockfd);
	return 0;
	
	}
		


int creer_configurer_sock_ecoute(uint16_t port)
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd == -1){
		perror("Erreur lors de la création de la socket");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serv_addr;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1){
		perror("Erreur lors de l'attachement de la socket au port");
		exit(EXIT_FAILURE);
	}
	if (listen(sockfd, 5) == -1){
		perror("Erreur lors de l'écoute sur la socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void envoyer_liste(int fd, struct catalogue *c)
{
	for ( int i = 0 ; i < c->nb ; i++){
		dprintf(fd,"%06d %s\n", i,c->tab[i]->titre);
	}
	dprintf(fd,"\n");
	return;
}

uint16_t recevoir_num_comptine(int fd)
{
	uint16_t num;
	read(fd,&num,sizeof(num));
	return ntohs(num);
}

void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic) {
    if (ic >= c->nb) {
        // Invalid comptine index
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", dirname, c->tab[ic]->nom_fichier);

    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Failed to open comptine file");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        dprintf(fd, "%s", line);
    }

    fclose(file);
}

