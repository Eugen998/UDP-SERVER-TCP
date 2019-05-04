
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "helpers.h"


void usage(char *file)
{
	fprintf(stderr, "Usage: %s ip_server port_server file\n", file);
	exit(0);
}

/*
 * Utilizare: ./client_udp ip_server port_server
 */
int main(int argc, char **argv)
{
	if (argc != 3)
		usage(argv[0]);

	int fd, sock;
	struct sockaddr_in to_station;
	char buf[BUFLEN];
	int ret;

	sock = socket(PF_INET,SOCK_DGRAM,0);

	inet_aton (argv[1],&(to_station.sin_addr));
	to_station.sin_family = AF_INET;
	to_station.sin_port = htons(atoi (argv[2]));

    memset(buf, 0, BUFLEN);
    fgets(buf, BUFLEN - 1, stdin);
	sendto(sock, buf, BUFLEN, 0,(struct sockaddr*)&to_station, sizeof(to_station));

	fd = close(sock);

	return 0;
}
