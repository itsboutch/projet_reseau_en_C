/* Travail qui appartient à l'étudiant : DJEROUA Mustapha Hacène 12105968*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "comptine_utils.h"

int read_until_nl(int fd, char *buf)
{
	{
	int nbytes_read = 0;
	char c;
	while ( read(fd,&c,1) == 1){
		buf[nbytes_read] = c;
		nbytes_read++;

		if ( c == '\n'){
			break;
		}
	}
	return nbytes_read -1;

	}

}

int est_nom_fichier_comptine(char *nom_fich)
{
	int len = strlen(nom_fich);
	char * extension = ".cpt";
	int ext_len = strlen(extension);
	if ( ext_len >= len)
		return 0;
	else {
		char * file_ext = nom_fich + (len - ext_len);
		return (strcmp(file_ext,extension) == 0);
	}
}

struct comptine *init_cpt_depuis_fichier(const char *dir_name, const char *base_name)
{
	int fd;
	char c = '/';
	char full_name[1024];
	snprintf(full_name, 1024, "%s%c%s", dir_name, c, base_name);
	char *buf = malloc(sizeof(char) * 256);
	if (buf == NULL) {
		perror("erreur lors du malloc du buffer");
		exit(EXIT_FAILURE);
	}
	if ((fd = open(full_name, O_RDONLY)) < 0) {
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}
	struct comptine *cpt = malloc(sizeof(struct comptine));
	if (cpt == NULL) {
		perror("erreur lors du malloc de la comptine");
		exit(EXIT_FAILURE);
	}
	int nom = (int)strlen(base_name);
	cpt->nom_fichier = malloc(sizeof(char) * (nom + 1));
	if (cpt->nom_fichier == NULL) {
		perror("erreur lors du malloc du nom de fichier de la comptine");
		exit(EXIT_FAILURE);
	}
	int length = read_until_nl(fd, buf);
	cpt->titre = malloc(sizeof(char) * (length + 1));
	if (cpt->titre == NULL) {
		perror("erreur lors du malloc du titre de la comptine");
		exit(EXIT_FAILURE);
	}
	strncpy(cpt->nom_fichier, base_name, nom);
	strncpy(cpt->titre, buf, length);
	cpt->nom_fichier[nom] = '\0';
	cpt->titre[length] = '\0';
	close(fd);
	free(buf);
	return cpt;
}

void liberer_comptine(struct comptine *cpt)
{
	free(cpt->titre);
	free(cpt->nom_fichier);
	free(cpt);
}

 struct catalogue *creer_catalogue(const char *dir_name)
{
	int is_file = 0;
	struct catalogue *cat = malloc(sizeof(struct catalogue));
	if (cat == NULL){
		perror("erreur lors du malloc du catalogue");
		exit(EXIT_FAILURE);
	} 
	cat->nb = 0;
	char *nom_fich = malloc(sizeof(char) * 256);
	if (nom_fich == NULL){
		perror("erreur lors du malloc du nom de fichier du catalogue");
		exit(EXIT_FAILURE);
	} 
	DIR *dir;
	struct dirent* entree;
	dir = opendir(dir_name);
	if (dir == NULL) {
		perror("Failed to open directory");
		free(nom_fich);
		free(cat); 
		return NULL;
	}

	while ((entree = readdir(dir)) != NULL){
		strcpy(nom_fich, entree->d_name);
		is_file = est_nom_fichier_comptine(nom_fich);
		if (is_file == 1){
			cat->nb++;
		}
	}

	int nb_comptines = cat->nb;
	printf("%d",cat->nb);	
	cat->tab = malloc(sizeof(struct comptine *) * nb_comptines);
	if (cat->tab == NULL){
		perror("erreur lors du malloc du tableau des comptines");
		free(nom_fich); 
		free(cat); 
		exit(EXIT_FAILURE);
	}
	rewinddir(dir);
	int index = 0;
	while ((entree = readdir(dir)) != NULL){
		strcpy(nom_fich, entree->d_name);
		is_file = est_nom_fichier_comptine(nom_fich);
		if (is_file == 1){
			struct comptine *cpt = init_cpt_depuis_fichier(dir_name, nom_fich);
			if (cpt != NULL){
				cat->tab[index] = cpt;
				index++;
			}
		}
	}	

	closedir(dir);
	free(nom_fich);
	return cat;
}
	
void liberer_catalogue(struct catalogue *c)
{
	if (c == NULL){
		return;
	}

	for (int i = 0; i < c->nb; i++){
		struct comptine *cpt = c->tab[i];
		liberer_comptine(cpt);
	}

	free(c->tab);
	free(c);
}
