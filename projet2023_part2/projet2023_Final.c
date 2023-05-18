#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char **argv){
	if (argc < 2){
		printf("Aucun script shell");
		exit(-1);
	}
	for (int script_num = 1; script_num < argc; script_num++){
		struct stat sts;
		char *sc;
		int num_pid;
		int fw[2];
		printf("Fichier #%d: %s\n", script_num, argv[script_num]);
		int fd = open(argv[script_num], O_RDONLY); // ouvre le fichier shell
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
		//printf("Premiere sous-chiane = %s\n", sc); // Ligne #!/bin/bash du fichier shell
		while ((sc=strtok(NULL, "\n")) != NULL){
			//printf("Sous-chaine suivante = %s\n", sc); // commande et param
			tab = realloc(tab, sizeof(char *) * (place + 1)); // alloue un espace memoire pour l'adresse de la commande et param
			tab[place] = sc; // copie la commande et param dans un espace de tab
			place++;
		}
		for (int i = 0; i < place; i++){
			char buf;
			printf("Commande#%d: %s\n", i+1, tab[i]);
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
					com = realloc(com, sizeof(char *) * (num + 1)); // alloue un espace memoire pour l'adresse de la commande et param
					com[num] = sc2; // copie la commande et param dans un espace different chacun de com
					num++;
					sc2 = strtok(NULL, " ");
				}
				com[num] = 0;
				close(1); // close stdout
				dup2(fw[1], 1); // send stdout to the writing side of the pipe
				
				//int execvp(const char *argv[1], char *const argv2[]); - param de execvp
				if (execvp(com[0], com) == -1){ // com[0] = commande, com = commande et param
					printf("Erreur execvp");
					exit(-5);
				}
				close(fw[1]); // Le lecteur verra EOF
				wait(NULL); // attend la lecture du pere
				exit(EXIT_SUCCESS);
			}
			else {
				close(fw[1]); //close writing end of the pipe
				while (read(fw[0], &buf, 1) > 0){
				printf("%s", &buf); //print every line in the pipe;
			}
			printf("\n");
			close(fw[0]); //end reading end of pipe
			}
		}
		close(fd);
	}
}
