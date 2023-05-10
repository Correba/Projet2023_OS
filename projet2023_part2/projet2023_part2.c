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
	if (argc < 2){
		printf("Aucun script shell");
		exit(-1);
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0){
		printf("Erreur a l'ouverture du fichier");
		exit(-2);
	}
	if (fstat(fd, &sts) != 0) {
		printf("Erreur a l'execution de fstat");
		exit(-3);
	}
	//printf("%ld", sts.st_ino); Nombre de inode
	char buff[sts.st_ino];
	read(fd, buff, sts.st_ino);
	sc = strtok(buff, "\n");
	printf("Premiere sous-chiane = %s\n", sc);
	while ((sc=strtok(NULL, "\n")) != NULL){
		printf("Sous-chaine suivante = %s\n", sc);
	}
	close(fd);
}
