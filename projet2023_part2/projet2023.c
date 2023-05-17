#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char **argv){
	struct stat sts;
	char *sc;
	int num_pid;
	int fw[2];
	if (argc < 2){
		printf("Aucun script shell");
		exit(-1);
	}
	int fd = open(argv[1], O_RDONLY); // ouvre le fichier shell
	if (fd < 0){
		printf("Erreur a l'ouverture du fichier");
		exit(-2);
	}
	if (fstat(fd, &sts) != 0) {
		printf("Erreur a l'execution de fstat");
		exit(-3);
	}
	//printf("%ld", sts.st_ino); Nombre de inode
	
	char **tab = NULL;
	int place = 0;
	
	char buff[sts.st_ino];
	read(fd, buff, sts.st_ino); // lit le contenue du fichier shell
	sc = strtok(buff, "\n"); // cree des sous-chaine en separant le texte ou il y a des \n
	//printf("Premiere sous-chiane = %s\n", sc); - Ligne #!/bin/bash du fichier shell
	while ((sc=strtok(NULL, "\n")) != NULL){
		//printf("Sous-chaine suivante = %s\n", sc); // commande et param
		tab = realloc(tab, sizeof(char *) * (place + 1)); 
		tab[place] = sc; // copie la commande et param dans tab
		place++;
	}
	for (int i = 0; i < place; i++){
		printf("Commande: %s\n", tab[i]);
		if (pipe(fw) == -1) { // check if pipe creation failed
			printf("Pipe Failed");
			exit(-4);
		}
		num_pid = fork();
		if (num_pid == -1) exit(-2); // on arrete le programme quand fork a une erreur
		if (num_pid == 0) { // processus fils
			close(fw[0]); // close reading end of pipe
			char **com = NULL;
			int num = 0;
			char *sc2;
			sc2 = strtok(tab[i], " ");
			while (sc2 != NULL){
				com = realloc(com, sizeof(char *) * (num + 1));
				com[num] = sc2;
				num++;
				sc2 = strtok(NULL, " ");
			}
			for (int e = 0; e < num; e++){
				printf("%s\n", com[e]);
			}
			exit(-2);
		}
		else {
			close(fw[1]);
		}
	}
	close(fd);
}
