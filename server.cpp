#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}


int main(int argc, char *argv[])
{
	int sockfd, newsockfd, udp_sockfd, portno;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;
	vector<client> subscribers;
	vector<topic_structure> topics;

	fd_set read_fds;	// multimea de citire
	fd_set tmp_fds;		// multime folosita pentru a clona read_fds
	int fdmax;			// valoare maxima fd

	if (argc < 2) {
		usage(argv[0]);
	}

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket tcp");

	udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(udp_sockfd < 0, "socket udp");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind tcp");

	ret = bind(udp_sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind udp");

	ret = listen(sockfd, MAX_SUBSCRIBERS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);	//pt tcp
	FD_SET(udp_sockfd, &read_fds);	//pt udp

	fdmax = sockfd > udp_sockfd ? sockfd : udp_sockfd;

	while (1) {
		tmp_fds = read_fds;
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					memset(buffer, 0, BUFLEN);
					n = recv(newsockfd, buffer, sizeof(buffer),0);
					DIE(n < 0, "recv_id");

					char cli_id[20];
					strcpy(cli_id, buffer);

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);

					if (newsockfd > fdmax) {
						fdmax = newsockfd;
					}

					client current;
					strcpy(current.id, cli_id);
					current.fd = newsockfd;
					subscribers.push_back(current);	//se adauga clientul in vectorul de clienti

					printf("New Client (%s) connected from %s : %d\n",
					cli_id,inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

				} else if (i == STDIN_FILENO){
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					if(strcmp(buffer, "exit\n")) exit(0);
				} else if(i == udp_sockfd){
					memset(buffer, 0, BUFLEN);
					recvfrom(i, buffer, BUFLEN, 0, (struct sockaddr*) &serv_addr,0);
					printf("Mesaj de la udp : %s\n", buffer);
				} else {
					// s-au primit date pe unul din socketii de subscriber,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0) {
						// conexiunea s-a inchis
						client rem = get_client(subscribers,i);
						printf("Client (%s) disconnected\n", rem.id);
						// se scoate din multimea de citire socketul inchis
						close(i);
						FD_CLR(i, &read_fds);
						//se sterge clientul din vectorul de clienti
						erase_client(subscribers,rem);
						//erase client from all topics
					} else {
						char *start;
						start = buffer;
						char action[20], topic[20];
						int SF = 0;

						parse(start, action, topic, &SF);
						client aux = get_client(subscribers,i);
						printf("Client (%s) %sd to (%s)\n", aux.id, action, topic);

						if(topic_exists(topics,topic)){	//daca topicul exista deja
							if(strcmp(action,"subscribe") == 0)
								add_subscriber_to_topic(topics, topic, aux);
							else{
								remove_subscriber_from_topic(topics, topic, aux);
							}
						}else{
							if(strcmp(action,"subscribe") == 0){
								add_topic(topics,topic);
								add_subscriber_to_topic(topics,topic,aux);
							}else{
								printf("Given topic does not exist; Unable to perform action\n");
							}

						}

						print_subs(topics);
					}
				}
			}
		}
	}

	close(sockfd);

	return 0;
}
