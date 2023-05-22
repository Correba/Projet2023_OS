#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
// #include <sys/wait.h> // wait utilisé en commentaires
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char **argv){

	if (argc < 2){
		printf("Aucun script shell\n");
		exit(-1);
	} // Vérifie s'il y a au moins un script shell en paramètre
	
	for (int script_num = 1; script_num < argc; script_num++){ // Parcoure chaque script shell
		struct stat sts; // la structure de lecture d'un fichier
		int file;          // le fichier script shell
		char **lines = NULL;      // le tableau des commandes du script shell
		int line_pos;       // la position d'espace mémoire des lignes du script shell
		char *line;        // une ligne du script shell
		
		printf("Fichier #%d: %s\n", script_num, argv[script_num]); // le numéro et le nom du script shell
		
		file = open(argv[script_num], O_RDONLY); // Ouvre le fichier script shell en lecture.
		if (file < 0){
			printf("Erreur d'ouverture du fichier\n"");
			exit(-2);
		} // Vérifie l'erreur à l'ouverture du fichier script shell.
		
		if (fstat(file, &sts) < 0) {
			printf("Erreur d'exécution de fstat\n"");
			exit(-3);
		} // Vérifie l'erreur à l'éxecution de la lecture statistique du fichier sript shell.
		
		char script[sts.st_ino]; // la chaine de caractères de la taille du nombre d'inodes du script shell
		
		read(file, script, sts.st_ino); // Lit le contenu du fichier script shell dans le buffer.
		close(file); // Ferme le fichier script shell.
		line = strtok(script, "\n"); // Lit la 1ère ligne du fichier script shell
		
		line_pos = 0; // Initialise la position d'espace mémoire du script shell à 0 octet.
		while ((line=strtok(NULL, "\n")) != NULL){ // Lit puis parcoure les commandes du script shell.
			lines = realloc(lines, sizeof(char *) * (line_pos + 1)); // Alloue un espace mémoire propre à l'adresse de la commande du script shell.
			lines[line_pos] = line; // Stocke la commande.
			line_pos++; // Incrémente la position de l'espace mémoire.
		}
		
		// --- EXECUTION DES COMMANDES ---
		for (int i = 0; i < line_pos; i++){ // Parcoure chaque commande du script shell.
			int pipefd[2];   // le pipe avec son entrée de lecture (0) et d'écriture (1)
			int num_pid; // le numéro de processus fils (0) ou père (1)
			
			printf("Fichier #%d: Commande #%d: %s\n", script_num, i+1, lines[i]); // le numéro du script shell, le numéro et le nom de la commande
			
			if (pipe(pipefd) == -1) {
				printf("Erreur de création du pipe\n"");
				exit(-4);
			} // Vérifie l'erreur à la création du pipe.
			
			num_pid = fork(); // Génère un processus fils.
			if (num_pid == -1) {
				printf("Erreur de création du processus fils\n"");
				exit(-5);
			} // Vérifie l'erreur à la création du processus fils.
			
			// --- PROCESSUS FILS ---
			if (num_pid == 0) { 
				char **coms = NULL; // le tableau de la commande suivi de ses paramètres
				int com_pos;    // la position d'espace mémoire de la commande et de ses paramètres
				char *com;  // la commande ou un de ses paramètres
				
				close(pipefd[0]); // Ferme le pipe de lecture inutile au processus fils.
				
				com = strtok(lines[i], " "); // Lit la commande.
				com_pos = 0; // Initialise la position d'espace mémoire de la commande ou d'un de ses paramètres à 0 octet.
				while (com != NULL){ // Parcoure la commande et ses paramètres.
					coms = realloc(coms, sizeof(char *) * (com_pos + 1)); // Alloue un espace mémoire propre à l'adresse de la commande ou de son paramètre.
					coms[com_pos] = com; // Stocke la commande ou l'un de ses paramètres.
					com = strtok(NULL, " "); // Lit le paramètre suivant de la commande
					com_pos++; // Incrémente la position de l'espace mémoire.
				}
				coms[com_pos] = 0; // Termine le tbleau par un 0, ce qui est nécessaire à l'éxecution de la commande.
				
				close(1); // Ferme le canal de sortie du terminal (stdout).
				if (dup2(pipefd[1], 1) == -1) { // Dévie le pipe d'écriture comme canal de sortie à la place de celui du terminal.
					printf("Erreur d'exécution de dup2\n"");
					exit(-6);
				} // Vérifie l'erreur au changement du canal de sortie
				
				//int execvp(const char *argv[1], char *const argv2[]); définition des paramètres de la fonction 'execvp'
				if (execvp(coms[0], coms) == -1){ // Exécute la commande (com[0]) avec ses paramètres.
					printf("Erreur d'exécution d'execvp\n"");
					exit(-7);
				} // Vérifie l'erreur à l'éxecution de la commande.
				
				/* PAS NECESSAIRE : execvp tue le processus fils à l'éxecution.
				close(fw[1]); // Ferme le pipe d'écriture.
				wait(NULL); // Attend la lecture du processus père.
				exit(0); // Termine le processus fils avec succés.
				*/
			} 
			
			// --- PROCESSUS PERE ---
			else {
				char results; // les résultats de l'éxécution des commandes du script shell
			
				close(pipefd[1]); // Ferme le pipe d'écriture inutile au processus père.
				
				while (read(pipefd[0], &results, 1) > 0){ // Lit chaque caractère des résultats de l'éxécution des commandes du script shell.
					printf("%s", &results); // Affiche chacun de ces caractères.
				}
				printf("\n"); // Termine l'affichage des résultats.
				
				close(pipefd[0]); // Ferme le pipe de lecture.
			}
		}
	}
	exit(0); // Finit le programme avec succés.
}
