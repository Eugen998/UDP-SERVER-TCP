#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;
int i;

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

//structura pentru a memora clientii activi
struct client {
 	char id[20];	//id-ul clientului
 	int fd;	//file descriptor asociat clientului
};

//structura pentru a memora topic urile curente
struct topic_structure {
	char name[51];	//denumirea topicului
	vector<client> subs;	//vectorul de abonati
};
//returneaza cloentul conectat la socketul dat
client get_client(vector<client> v, int sfd){
	for(i = 0; i < v.size(); i++){
		if(v[i].fd == sfd)
			return v[i];
	}
}
//printeaza clientii din vectorul de clienti
void print_clients(vector<client> v){
	for(i = 0; i < v.size(); i++){
		printf("[Client id: %s, socket : %d]\n", v[i].id, v[i].fd);
	}
}
//sterge clientul dat din vectorul de clienti
void erase_client(vector<client> &v, client rem){
	vector<client>::iterator it = v.begin();
	while(strcmp(it->id,rem.id) != 0){
		it++;
	}
	v.erase(it);
}
//verifica daca topicul dat exista sau nu in lista de topicuri
int topic_exists(vector<topic_structure>v, char topic[]){
	topic_structure aux;
	for(i = 0; i < v.size(); i++){
		aux = v[i];
		if(strcmp(aux.name,topic) == 0) return 1;
	}
	return 0;
}
//adauga un topic nou in lista de topicuri
void add_topic(vector<topic_structure> &v, char topic[]){
	topic_structure nou;
	strcpy(nou.name, topic);
	v.push_back(nou);
}
//verifica daca un client este in lista de clienti data ca parametru
int contains(vector<client> v, client c){
	for(vector<client>::iterator it = v.begin(); it != v.end(); it++){
		if(strcmp(it->id,c.id) == 0) return 1;
	}
	return 0;
}
//adauga un subscriber la topicul dat din lista de topicuri
void add_subscriber_to_topic(vector<topic_structure> &v, char topic[], client c){
	for(i = 0; i < v.size(); i++){
		if(strcmp(v[i].name,topic) == 0){
			if(contains(v[i].subs,c) == 0) v[i].subs.push_back(c);
			else printf("Client (%s) is already subscribed to topic (%s)\n",c.id,topic);
		}
	}
}
//analog cu cea de mai sus, doar ca sterge acel subscriber
void remove_subscriber_from_topic(vector<topic_structure> &v, char topic[], client c){
	for(i = 0; i < v.size(); i++){
		if(strcmp(v[i].name,topic) == 0) {
			if(contains(v[i].subs,c) != 0) erase_client(v[i].subs,c);
			else printf("Client (%s) is not subscribed to topic (%s), can not unsubscribe\n",c.id,topic);
		}
	}
}
//printeaza toate topicurile si clientii abonati la ele (just for debbuging)
void print_subs(vector<topic_structure> v){
	for(i = 0; i < v.size(); i++){
		vector<client> aux = v[i].subs;
		printf("TOPICUL %s; Subscriberi: \n",v[i].name);
		for(int j = 0; j < aux.size(); j++){
			printf("[Client id: %s socket: %d]\n",aux[j].id, aux[j].fd);
		}
	}
}

topic_structure get_topic(vector <topic_structure> v, char topic[]){
	topic_structure aux;
	for(i = 0; i < v.size(); i++){
		aux = v[i];
		if(strcmp(aux.name,topic) == 0) return aux;
	}
}

void forward_message(vector<topic_structure> &v, char buffer[], char ip[], int port){
	char topic[51], content[1501], send_message[1600];
	int type, sign, ret;

	memset(topic, 0, 51);
	memcpy(topic, buffer, 50);

	if(topic_exists(v, topic) == 0) {
		add_topic(v, topic);
	}

	topic_structure aux = get_topic(v,topic);
	vector<client> send_to = aux.subs;

	type = buffer[50];

	if(type == 0){
		int sign = buffer[51];
		uint32_t value0;
		memset(&value0, 0, sizeof(uint32_t));
		memcpy(&value0, buffer + 52, sizeof(uint32_t));
		int int_value0 = ntohl(value0);
		if(sign == 1) int_value0 = int_value0 * (-1);

		memset(send_message, 0 , 1600);
		sprintf(send_message, "%s : %d - %s - INT - %d", ip, port, topic, int_value0);
	} else if (type == 1){
		uint16_t value1;
		memset(&value1, 0, sizeof(uint16_t));
		memcpy(&value1, buffer + 51, sizeof(uint16_t));
		int int_value1 = ntohl(value1) / 100;

		memset(send_message, 0 , 1600);
		sprintf(send_message, "%s : %d - %s - SHORT_REAL - %d", ip, port, topic, int_value1);
	} else if (type == 2){
		memset(content, 0, 1500);
		memset(send_message, 0 , 1600);
		strcpy(send_message, "2 not yet");
	} else if (type == 3){
		memset(content, 0, 1501);
		memset(send_message, 0 , 1600);
		memcpy(content, buffer + 51, 1501);
		sprintf(send_message, "%s : %d - %s - STRING - %s",ip,port,topic,content);
	}

	for(i = 0; i < send_to.size(); i++){
		ret = send(send_to[i].fd, send_message, 1600, 0);
		DIE(ret < 0, "forward message from server to tcp client");
	}

}
//parseaza mesajele trimise de clientii TCP
void parse(char s[], char action[], char topic[], int *SF){
		char *aux = strtok(s, " ");
		strcpy(action, aux); 	//primul cuvant este operatiunea

		if(strcmp(action,"unsubscribe") == 0){ //daca se da comanda cu unsubscribe, nu se mai primeste SF
			aux = strtok(NULL, "\n");
			strcpy(topic,aux);
			return;
		}

		aux = strtok(NULL, " ");
		strcpy(topic, aux);		//al doilea cuvant este topicul
		*SF = atoi(strtok(NULL, " "));	//al treilea este parametrul SF
}


#define BUFLEN		1600	// dimensiunea maxima a calupului de date
#define MAX_SUBSCRIBERS	5	// numarul maxim de clienti in asteptare

#endif
