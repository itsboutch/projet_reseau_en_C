/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <sys/types.h>
//#include <winsock2.h>
#include <sys/socket.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321

void usage(char *nom_prog)
{
    fprintf(stderr, "Usage: %s addr_ipv4\n"
            "client pour WCP (Wikicomptine Protocol)\n"
            "Exemple: %s 208.97.177.124\n", nom_prog, nom_prog);
}

/** Retourne (en cas de succès) le descripteur de fichier d'une socket
 *  TCP/IPv4 connectée au processus écoutant sur port sur la machine d'adresse
 *  addr_ipv4 */
int creer_connecter_sock(char *addr_ipv4, uint16_t port);

/** Lit la liste numérotée des comptines dans le descripteur fd et les affiche
 *  sur le terminal.
 *  retourne : le nombre de comptines disponibles */
uint16_t recevoir_liste_comptines(int fd);

/** Demande à l'utilisateur un nombre entre 0 (compris) et nc (non compris)
 *  et retourne la valeur saisie. */
uint16_t saisir_num_comptine(uint16_t nb_comptines);

/** Écrit l'entier ic dans le fichier de descripteur fd en network byte order */
void envoyer_num_comptine(int fd, uint16_t nc);

/** Affiche la comptine arrivant dans fd sur le terminal */
void afficher_comptine(int fd);

int main(int argc, char *argv[])
{
    /*if (argc != 2) {
        usage(argv[0]);
        return 1;
    }*/
    int sockfd = creer_connecter_sock(argv[1],PORT_WCP);
    printf(" OOHHH un client s'est connecté :\n ");
    uint16_t recv,recv1 = 0;
    recv = recevoir_liste_comptines(sockfd);
    recv1 = saisir_num_comptine(recv);
    envoyer_num_comptine(sockfd,recv1);
    afficher_comptine(sockfd);
    close(sockfd);
    return 0;
}

int creer_connecter_sock(char *addr_ipv4, uint16_t port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd == -1){
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if  (inet_pton(AF_INET,addr_ipv4,&(serv_addr.sin_addr)) <=0){
        perror("Erreur lors de la conversion de l'adresse IP");
        exit(EXIT_FAILURE);
    }
    if  (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1){
        perror("Erreur lors de la connexion à la socket distante");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

uint16_t recevoir_liste_comptines(int fd)
{   
    char buffer[1024];
    uint16_t nb_comptines = 0;
    int read = 0;
    while(1) {
        read = read_until_nl(fd, buffer);
        if (read == 0) {
            break;
        }
        char* titre = malloc(sizeof(char) * (read + 1));
        if (titre == NULL) {
            perror("erreur du malloc du titre de recevoir liste comptines");
            exit(EXIT_FAILURE);
        } 
        int num_titre;
        sscanf(buffer, "%d %[^\n]", &num_titre, titre);
        printf("%d %s\n", num_titre, titre);
        nb_comptines++;
        
        free(titre);
    }
    return nb_comptines;
}

uint16_t saisir_num_comptine(uint16_t nb_comptines)
{
    uint16_t num;
    printf("Veuillez saisir le numéro de la comptine que vous voulez entre 0 et %d :\n ", nb_comptines);
    scanf("%hu",&num);
  return num;
}

void envoyer_num_comptine(int fd, uint16_t nc)
{
    uint16_t num = htons(nc);
    write(fd,&num,sizeof(num));
}

void afficher_comptine(int fd) {
    char line[1024];
    int num_read = 0;
    char ch;
    int i = 0;

    while ((num_read = read(fd, &ch, 1))) {
        if (ch == '\n') {
            line[i] = '\0';
            printf("%s\n", line);
            i = 0;
        } else {
            line[i++] = ch;
        }
    }

    if (i > 0) {
        line[i] = '\0';
        printf("%s\n", line);
    }
}