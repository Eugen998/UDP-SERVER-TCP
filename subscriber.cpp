#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s client_id server_address server_port\n", file);
	exit(0);
}

void parse(char s[], char action[], char topic[], int *SF){
	   char *aux = strtok(s, " ");
	   strcpy(action, aux);
	   aux = strtok(NULL, " ");
	   strcpy(topic, aux);
	   *SF = atoi(strtok(NULL, " "));
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	fd_set read_fds;
	fd_set tmp_fds;

	FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

	FD_SET(sockfd, &read_fds);
	FD_SET(STDIN_FILENO, &read_fds);

	int max = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	char id_buff[20];
	strcpy(id_buff,argv[1]);
	printf("Id ul clientului este %s \n",id_buff);
	ret = send(sockfd, id_buff, strlen(id_buff), 0);
	DIE(ret < 0, "send id");

	while (1) {
		tmp_fds = read_fds;
		select(max + 1, &tmp_fds, NULL, NULL, NULL);

		for(int j = 0; j <= max; j++) {
			if(FD_ISSET(j, &tmp_fds)) {
				if(j == STDIN_FILENO) {
					// se citeste de la tastatura
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					printf("[SUBSCRIBER]: %s\n", buffer);

					if (strcmp(buffer, "exit\n") == 0) {
						exit(0);
					} else {
						char action[20], topic[20];
						int SF;
						char cpy[20];
						strcpy(cpy,buffer);
						parse(cpy, action, topic, &SF);
						if(strcmp(action, "subscribe") != 0 && strcmp(action, "unsubscribe") != 0){
							printf("Actiunea introdusa nu este valida\n");
							break;
						}
						if(SF != 0 && SF != 1){
							printf("Indicele de store&forward poate fi doar 0 sau 1\n");
							break;
						}
						// se trimite mesaj la server
						printf("mesajul: %s\n",buffer);
						n = send(sockfd, buffer, strlen(buffer), 0);
						DIE(n < 0, "send");
						printf("%s %s",action,topic);

					}

				} else if(j == sockfd) {
					// se citeste de pe socket
					memset(buffer, 0, BUFLEN);
            		n = recv(sockfd, buffer, sizeof(buffer), 0);
            		if(n == 0) {
              			return -1;
            		} else {
              			printf("[SUBSCRIBER] Am citit de la server mesajul:\n");
              			printf("%s\n", buffer);
            		}
				}
			}
		}
	}

	close(sockfd);

	return 0;
}
