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

struct topic_structure {
	char name[20];	//denumirea topicului
	vector<client> subs;	//vectorul de abonati
};

client get_client(vector<client> v, int sfd){
	for(i = 0; i < v.size(); i++){
		if(v[i].fd == sfd)
			return v[i];
	}
}

void print_clients(vector<client> v){
	for(i = 0; i < v.size(); i++){
		printf("[Client id: %s, socket : %d]\n", v[i].id, v[i].fd);
	}
}

void erase_client(vector<client> &v, client rem){
	vector<client>::iterator it = v.begin();
	while(strcmp(it->id,rem.id) != 0){
		it++;
	}
	v.erase(it);
}

int topic_exists(vector<topic_structure>v, char topic[]){
	topic_structure aux;
	for(i = 0; i < v.size(); i++){
		aux = v[i];
		if(strcmp(aux.name,topic) == 0) return 1;
	}
	return 0;
}

void add_topic(vector<topic_structure> &v, char topic[]){
	topic_structure nou;
	strcpy(nou.name, topic);
	v.push_back(nou);
}

int contains(vector<client> v, client c){
	for(vector<client>::iterator it = v.begin(); it != v.end(); it++){
		if(strcmp(it->id,c.id) == 0) return 1;
	}
	return 0;
}

void add_subscriber_to_topic(vector<topic_structure> &v, char topic[], client c){
	for(i = 0; i < v.size(); i++){
		if(strcmp(v[i].name,topic) == 0){
			if(contains(v[i].subs,c) == 0) v[i].subs.push_back(c);
			else printf("Client (%s) is already subscribed to topic (%s)\n",c.id,topic);
		}
	}
}

void remove_subscriber_from_topic(vector<topic_structure> &v, char topic[], client c){
	for(i = 0; i < v.size(); i++){
		if(strcmp(v[i].name,topic) == 0) {
			if(contains(v[i].subs,c) != 0) erase_client(v[i].subs,c);
			else printf("Client (%s) is not subscribed to topic (%s), can not unsubscribe\n",c.id,topic);
		}
	}
}

void print_subs(vector<topic_structure> v){
	for(i = 0; i < v.size(); i++){
		vector<client> aux = v[i].subs;
		printf("TOPICUL %s; Subscriberi: \n",v[i].name);
		for(int j = 0; j < aux.size(); j++){
			printf("[Client id: %s socket: %d]\n",aux[j].id, aux[j].fd);
		}
	}
}

void parse(char s[], char action[], char topic[], int *SF){
		char *aux = strtok(s, " ");
		strcpy(action, aux);

		if(strcmp(action,"unsubscribe") == 0){
			aux = strtok(NULL, "\n");
			strcpy(topic,aux);
			return;
		}

		aux = strtok(NULL, " ");
		strcpy(topic, aux);
		*SF = atoi(strtok(NULL, " "));
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
