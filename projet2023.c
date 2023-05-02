#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
//#include <sys/stat.h> - si on utilise un open

int main(int argc, char **argv){
	int num_pid;
	char *argv2[argc -1];
	int fw[2];
	char buf;
	if (argc < 2){
		printf("Aucune commande et/ou parametre");
		exit(-1);
	}
	if (pipe(fw) == -1) {
		printf("Pipe Failed");
		exit(-4);
	}
	num_pid = fork();
	if (num_pid == -1) exit(-2); // on arrete le programme quand fork a une erreur
	if (num_pid == 0) {
		//printf("Processus fils\n");
		
		close(fw[0]); //close reading end of pipe
		for (int i = 1; i < argc; i++){ // Create the table with the command and its param
			argv2[i-1] = argv[i];
			//printf("%s\n", argv2[i-1]); - check the arguments added to argv2;
		}
		
		/*int fd = open("./resultats.txt", O_RDWR | O_TRUNC, "wt"); 
		// send the results to the text file
		if (fd < 0){
			printf("Erreur a la creation du fichier");
			exit(-3);
		}
		close(1);
		dup2(fd, 1);*/
		
		close(1); // close stdout
		dup2(fw[1], 1); // send stdout to the writing side of the pipe
		
		//int execvp(const char *argv[1], char *const argv2[]); - param de execvp
		execvp(argv[1], argv2); // arvg[1] = commande, argv2 = commande et param
		
		close(fw[1]); // Le lecteur verra EOF
		wait(NULL); // attend la lecture du pere
		exit(EXIT_SUCCESS);
	}else{
		//printf("Processus pere\n");
		
		close(fw[1]); //close writing end of the pipe
		while (read(fw[0], &buf, 1) > 0){
			printf("%s", &buf); //print every line in the pipe;
		}
		close(fw[0]); //end reading end of pipe
		exit(EXIT_SUCCESS);
	}
}
