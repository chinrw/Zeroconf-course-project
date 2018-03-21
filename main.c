#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>

#define BUFFER_SIZE 64
#define MAX_CLIENTS 100

fd_set readfds;

void* TCP_function(void* arg);


int main(int argc, char* argv[]) {
	printf("Started server\n");
	fflush(stdout);
	if (argc != 2) {
		fprintf(stderr, "ERROR: wrong number of arguments\n");
		fflush(stdout);
		return -1;
	}

	/*establish structure*/
	int tcp_port = atoi(argv[1]);
	if (tcp_port == 0) {
		fprintf(stderr, "ERROR: invalid argument(s)\n");
		fflush(stdout);
		return -1;
	}
	int tcp_socket_temp = socket(PF_INET, SOCK_STREAM, 0);
	unsigned int tcp_socket;
	if (tcp_socket < 0) {
		fprintf(stderr, "ERROR: socket() failed\n");
		fflush(stdout);
		return -1;
	}
	else { tcp_socket = (unsigned int)tcp_socket_temp; }
	struct sockaddr_in tcp_server;
	struct sockaddr_in tcp_client;
	tcp_server.sin_family = PF_INET;
	tcp_server.sin_addr.s_addr = INADDR_ANY;
	tcp_server.sin_port = htons(tcp_port);
	if (bind(tcp_socket, (struct sockaddr*)&tcp_server, sizeof(tcp_server)) < 0) {
		fprintf(stderr, "ERROR: bind() tcp failed\n");
		fflush(stdout);
		return -1;
	}
	if (listen(tcp_socket, 5) < 0) {
		fprintf(stderr, "ERROR: listen() tcp failed\n");
		fflush(stdout);
		return -1;
	}
	int sizeOfTCPserver = sizeof(tcp_server);
	printf("Listening for TCP connections on port: %d\n", tcp_port);
	fflush(stdout);
	/*establish structure*/

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(tcp_socket, &readfds);
		int n = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
		if (n < 0) {
			fprintf(stderr, "ERROR: select() failed\n");
			perror("select()");
			fflush(stdout);
			return -1;
		}
		else if (n == 0) { continue; }
		if (FD_ISSET(tcp_socket, &readfds)) {//incoming TCP connection
			int new_socket = accept(tcp_socket, (struct sockaddr*)&tcp_client, (socklen_t*)&sizeOfTCPserver);
			printf("Rcvd incoming TCP connection from: %s\n", inet_ntoa((struct in_addr)tcp_client.sin_addr));
			fflush(stdout);
			pthread_t tid;
			if (pthread_create(&tid, NULL, TCP_function, &new_socket) != 0) {
				fprintf(stderr, "ERROR: pthread_create() failed\n");
				fflush(stdout);
				return -1;
			}
		}
	}

	return 0;
}

void* TCP_connection(void* arg) {
	pthread_detach(pthread_self());
	int* fd_ptr = (int*)arg;
	int fd = *fd_ptr;
	char buffer[BUFFER_SIZE];
	char fileName[32];

	while (1) {
		int n = recv(fd, buffer, BUFFER_SIZE - 1, 0);
		if (n < 0) {
			fprintf(stderr, "ERROR: recv() failed");
			fflush(stdout);
			return NULL;
		}
		else if (n == 0) {
			printf("[child %u] Client disconnected\n", (unsigned int)pthread_self());
			fflush(stdout);
			close(fd);
			return NULL;
		}
		else {
			buffer[n] = '\0';
			char command[5];
			strncpy(command, buffer, 4);
			command[4] = '\0';

			char* nextLinePos = strchr(buffer, '\n');
			if (nextLinePos == NULL) {
				fprintf(stderr, "ERROR: strchr(\'\\n\') failed\n");
				fprintf(stderr, "buffer[%s]\n", buffer);
				fflush(stdout);
				return NULL;
			}

			if (1) {
				//TODO
				printf("HELLO\n");
			}
			else {
				fprintf(stderr, "ERROR: INVALID ACTION\n");
				fflush(stdout);
				return NULL;
			}
		}
	}
	return NULL;
}
