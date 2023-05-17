#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	struct stat sts;
	int fd;
	char *sc1;
	char *sc2;
	int pipes[10];
	int ipipe = 0;
	int num_pid;
	
	if (argc<2) {
		printf("Il faut donner au moins un nom de script shell.\n");
		exit(-1); // Assert Error => assert(argc<2);
	}
	
	fd = open(argv[1], O_RDONLY); // Ouverture du premier script shell
	if (fd < 0) {
	 printf("Erreur à l'ouverture du script '%s'\n", argv[1]);
	 exit(-2);
	} // Error open
	
	if (fstat(fd, &sts) != 0) { // Récupération des statistiques du script
		printf("Error fstat\n");
		exit(-3);
	} // Error fstat
	
	char buf[sts.st_ino]; // sts.st_ino = nombre d'inodes du fichier
	read(fd, buf, sts.st_ino); // Lecture du fichier script
	sc1 = strtok(buf, "\n");
		
	while ((sc1 = strtok(NULL, "\n")) != NULL) { // Séparation du script à chaque passage à la ligne
		char *argv2[10];
		
		if (pipe(pipes + ipipe) == -1) {
			printf("Error pipe\n");
			exit(-4);
		} // Error pipe
		
		num_pid = fork();
		if (num_pid == -1) {
			printf("Error fork\n");
			exit(-5);
		} // Error fork
		
		if (num_pid == 0) {
			int i = 0;
			
			close(pipes[ipipe]);
			dup2(pipes[ipipe + 1],1);
			
			// printf("%s\n", sc1);
			sc2 = strtok(sc1, " ");
			argv2[i] = sc2;
			while ((sc2 = strtok(NULL, " ")) != NULL) {
				i++;
				argv2[i] = sc2;
			}
			argv2[i+1] = 0;
			
			if (execvp(argv2[0], argv2) == -1) { // Exécute la commande (argv2[0]) avec ses paramètres et arguments
				printf("Error execvp\n");
				exit(-6);
			}; // Error execvp
			exit(0);
		}
		
		ipipe += 2;
	}
	
	if (num_pid != 0) {
		for (int compteur = 0; compteur < sizeof(pipes); compteur+=2){
			char buf2;
			close(pipes[compteur+1]);
			while (read(pipes[compteur], &buf2, 1) > 0) { // Lit le pipe dans une variable, caractère par caractère.
				printf("%s",&buf2); // Affiche chacun des caractères lu
			}
			close(pipes[compteur]);
		}
		for (int compteur = ipipe; compteur < sizeof(pipes); compteur++) {
			close(pipes[compteur]);
		}
		exit(0);
	}
	
	
	close(fd); // Fermeture du script
	exit(0);
}
