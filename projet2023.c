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
	} // Vérifie s'il y a au moins un script shell en paramètre
	
	for (int script_num = 1; script_num < argc; script_num++){ // Parcoure chaque script shell
		struct stat sts; // la structure de lecture d'un fichier
		int fd; // le fichier script shell
		char **tab; // le tableau des commandes du script shell
		int place; // la position d'espace mémoire des lignes du script shell
		char *sc; // une ligne du script shell
		
		printf("Fichier #%d: %s\n", script_num, argv[script_num]); // le numéro et le nom du script shell
		
		fd = open(argv[script_num], O_RDONLY); // Ouvre le fichier script shell en lecture.
		if (fd < 0){
			printf("Open File Failed");
			exit(-2);
		} // Vérifie l'erreur à l'ouverture du fichier script shell.
		
		if (fstat(fd, &sts) < 0) {
			printf("Fstat failed");
			exit(-3);
		} // Vérifie l'erreur à l'éxecution de la lecture statistique du fichier sript shell.
		
		char buff[sts.st_ino]; // la chaine de caractères de la taille du nombre d'inodes du script shell
		
		read(fd, buff, sts.st_ino); // Lit le contenu du fichier script shell dans le buffer.
		sc = strtok(buff, "\n"); // Lit la 1ère ligne du fichier scipt shell
		//printf("Premiere sous-chiane = %s\n", sc); // la ligne #!/bin/bash du scipt shell => Aucun intérêt à éxécuter.
		
		place = 0; // Initialise la position d'espace mémoire du script shell à 0 octet.
		while ((sc=strtok(NULL, "\n")) != NULL){ // Lit puis parcoure les commandes du script shell.
			tab = realloc(tab, sizeof(char *) * (place + 1)); // Alloue un espace mémoire propre à l'adresse de la commande du script shell.
			tab[place] = sc; // Stocke la commande.
			place++; // Incrémente la position de l'espace mémoire.
		}
		
		// --- EXECUTION DES COMMANDES ---
		for (int i = 0; i < place; i++){ // Parcoure chaque commande du script shell.
			int fw[2]; // le pipe avec son entrée de lecture (0) et d'écriture (1)
			int num_pid; // le numéro de processus fils (0) ou père (1)
			
			printf("Fichier #%d: Commande#%d: %s\n", script_num, i+1, tab[i]); // le numéro du script shell, le numéro et le nom de la commande
			if (pipe(fw) == -1) {
				printf("Pipe Failed");
				exit(-4);
			} // Vérifie l'erreur à la création du pipe.
			
			num_pid = fork(); // Génère un processus fils.
			if (num_pid == -1) {
				printf("Fork Failed");
				exit(-5);
			} // Vérifie l'erreur à la création du processus fils.
			
			// --- PROCESSUS FILS ---
			if (num_pid == 0) { 
				char **com; // le tableau de la commande suivi de ses paramètres
				int num; // la position d'espace mémoire de la commande et de ses paramètres
				char *sc2; // la commande ou un de ses paramètres
				
				close(fw[0]); // Ferme le pipe de lecture inutile au processus fils.
				
				sc2 = strtok(tab[i], " "); // Lit la commande.
				num = 0; // Initialise la position d'espace mémoire de la commande ou d'un de ses paramètres à 0 octet.
				while (sc2 != NULL){ // Parcoure la commande et ses paramètres.
					com = realloc(com, sizeof(char *) * (num + 1)); // Alloue un espace mémoire propre à l'adresse de la commande ou de son paramètre.
					com[num] = sc2; // Stocke la commande ou l'un de ses paramètres.
					sc2 = strtok(NULL, " "); // Lit le paramètre suivant de la commande
					num++; // Incrémente la position de l'espace mémoire.
				}
				com[num] = 0; // Termine le tbleau par un 0, ce qui est nécessaire à l'éxecution de la commande.
				
				close(1); // Ferme le canal de sortie du terminal (stdout).
				dup2(fw[1], 1); // place le pipe d'écriture comme canal de sortie à la place de celui du terminal.
				
				//int execvp(const char *argv[1], char *const argv2[]); définition des paramètres de la fonction 'execvp'
				if (execvp(com[0], com) == -1){ // Exécute la commande (com[0]) avec ses paramètres.
					printf("Execvp Failed");
					exit(-6);
				} // Vérifie l'erreur à l'éxecution de la commande.
				
				/* PAS NECESSAIRE : execvp tue le processus fils à l'éxecution.
				close(fw[1]); // Ferme le pipe d'écriture.
				wait(NULL); // Attend la lecture du processus père.
				exit(0); // Termine le processus fils avec succés.
				*/
				
			} 
			
			// --- PROCESSUS PERE ---
			else {
				char buf; // les résultats de l'éxécution des commandes du script shell
			
				close(fw[1]); // Ferme le pipe d'écriture inutile au processus père.
				
				while (read(fw[0], &buf, 1) > 0){ // Lit chaque caractère des résultats de l'éxécution des commandes du script shell.
					printf("%s", &buf); // Affiche chacun de ces caractères.
				}
				printf("\n"); // Termine l'affichage des résultats.
				
				close(fw[0]); // Ferme le pipe de lecture.
			}
		}
		close(fd); // Ferme le fichier script shell.
	}
}
