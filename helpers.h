#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;
/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */
int i;
struct client {
 	char id[20];	//id-ul clientului
 	int fd;	//file descriptor asociat clientului
};

client get_client(vector<client> v, int sfd){
	for(i = 0; i < v.size(); i++){
		if(v[i].fd == sfd) return v[i];
	}
}
void print_clients(vector<client> v){
	for(i = 0; i < v.size(); i++){
		printf("[Client id: %s, socket : %d]\n", v[i].id, v[i].fd);
	}
}

void parse(char s[], char action[], char topic[], int *SF){
		char *aux = strtok(s, " ");
		strcpy(action, aux);
		aux = strtok(NULL, " ");
		strcpy(topic, aux);
		*SF = atoi(strtok(NULL, " "));
}

void erase_client(vector<client> &v, client rem){
	vector<client>::iterator it = v.begin();
	while(strcmp(it->id,rem.id) != 0){
		it++;
	}
	v.erase(it);

}

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN		256	// dimensiunea maxima a calupului de date
#define MAX_SUBSCRIBERS	5	// numarul maxim de clienti in asteptare

#endif
