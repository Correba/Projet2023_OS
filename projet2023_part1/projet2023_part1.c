#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	int pipefd[2]; // Notre pipe : pipe[0] => read ; pipe[1] => write
	int num_pid; // Notre fork
	char *argv2[argc]; //tableau de la commande et de ses paramètres et arguments
	char buf;
	
	if (argc<2) {
		printf("Il faut donner au moins une commande ainsi que les options et/ou paramètres de celle-ci si nécessaire.\n");
		exit(-1); // Assert Error => assert(argc<2);
	}
	
	if (pipe(pipefd) == -1) {
		printf("Error pipe\n");
		exit(-2);
	} // Error pipe
	
	num_pid = fork();
	if (num_pid == -1) {
		printf("Error fork\n");
		exit(-3);
	} // Error fork
	
	// --- PROCESSUS FILS ---
	
	if (num_pid == 0) {
		close(pipefd[0]); // Ferme le pipe de lecture (Inutile)
		
		close(1); // Ferme le canal stdout
		dup2(pipefd[1],1); //Redirige le stdout vers le pipe d'écriture
		
		for (int i = 1; i < argc; i++) {
			argv2[i-1] = argv[i];
		} // Créer le tableau de la commande et de ses paramètres et arguments
		argv2[argc-1] = 0; // execvp a besoin d'un argument 0 dans son tableau argv pour terminer l'éxecution.
		
		if (execvp(argv2[0], argv2) == -1) { // Exécute la commande (argv2[0]) avec ses paramètres et arguments
			printf("Error execvp\n");
			exit(-4);
		}; // Error execvp
		
		wait(NULL); // Attend la lecture du père
		exit(0);
	}
	
	// --- PROCESSUS PERE ---
	
	else {
		close(pipefd[1]); // Ferme le pipe d'écriture (Inutile)
		
		while (read(pipefd[0], &buf, 1) > 0) // Lit le pipe dans une variable, caractère par caractère.
			printf("%s",&buf); // Affiche chacun des caractères lu
		printf("\n");
		
		exit(0);
	}
}
