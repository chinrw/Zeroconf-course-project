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

#include "zeroconf.h"

#define BUFFER_SIZE 64
#define MAX_CLIENTS 2

#define STATE_NO_NAME_NO_CHOICE 0
#define STATE_HAS_NAME_NO_CHOICE 1
#define STATE_HAS_NAME_HAS_CHOICE 2

#define CHOICE_PAPER 0
#define CHOICE_SCISSOR 1
#define CHOICE_STONE 2

struct UserData {
	char username[BUFFER_SIZE];
	int choice;
};
struct UserData user1;
struct UserData user2;

struct ThreadArgs {
	struct UserData user1;
	struct UserData user2;
};

void* TCP_connection(void* arg);


int main(int argc, char* argv[]) {
	printf("Started server\n");
	fflush(stdout);
	if (argc != 1) {
		fprintf(stderr, "ERROR: wrong number of arguments\n");
		fflush(stdout);
		return -1;
	}

	/*establish structure*/
	unsigned int tcp_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		fprintf(stderr, "ERROR: socket() failed\n");
		fflush(stdout);
		return -1;
	}
	struct sockaddr_in tcp_server, tcp_client, my_address;
	tcp_server.sin_family = PF_INET;
	tcp_server.sin_addr.s_addr = INADDR_ANY;
	tcp_server.sin_port = htons(0);
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
	int sizeOfsockaddr = sizeof(tcp_server);
	getsockname(tcp_socket, (struct sockaddr *)&my_address, &sizeOfsockaddr);

	printf("Listening for TCP connections on port: %d\n", ntohs(my_address.sin_port));
	fflush(stdout);
	/*establish structure*/

	fd_set readfds;
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
			int new_socket = accept(tcp_socket, (struct sockaddr*)&tcp_client, (socklen_t*)&sizeOfsockaddr);
			printf("Rcvd incoming TCP connection from: [%s] on port[%d]\n", inet_ntoa((struct in_addr)tcp_client.sin_addr), tcp_client.sin_port);
			fflush(stdout);
			pthread_t tid;
			if (pthread_create(&tid, NULL, TCP_connection, &new_socket) != 0) {
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
	int* arg_ptr = (int*)arg;
	int fd = (*arg_ptr);
	char buffer[BUFFER_SIZE];

	struct UserData* user_ptr = NULL;
	if (user1.username == "") {
		user_ptr = &user1;
		user_ptr->choice = STATE_NO_NAME_NO_CHOICE;
	}
	else if (user2.username == "") {
		user_ptr = &user2;
		user_ptr->choice = STATE_NO_NAME_NO_CHOICE;
	}
	else {
		fprintf(stderr, "ERROR: 3RD USER IS INVALID\n");
		fflush(stdout);
		return NULL;
	}

	while (1) {
		int n = recv(fd, buffer, BUFFER_SIZE - 1, 0);
		if (n < 0) {
			fprintf(stderr, "ERROR: recv() failed");
			fflush(stdout);
			return NULL;
		}
		else if (n == 0) {
			printf("[Thread %u] Client disconnected\n", (unsigned int)pthread_self());
			fflush(stdout);
			close(fd);
			return NULL;
		}
		else {
			buffer[n] = '\0';

			char* nextLinePos = strchr(buffer, '\n');
			if (nextLinePos == NULL) {
				fprintf(stderr, "ERROR: strchr(\'\\n\') failed\n");
				fprintf(stderr, "buffer[%s]\n", buffer);
				fflush(stdout);
				return NULL;
			}

			switch (user_ptr->choice) {
			case STATE_NO_NAME_NO_CHOICE:
				handle_NO_NAME_NO_CHOICE(user_ptr, buffer, n);
				break;
			case STATE_HAS_NAME_NO_CHOICE:
				handle_HAS_NAME_NO_CHOICE(user_ptr, buffer, n);
				break;
			case STATE_HAS_NAME_HAS_CHOICE:
				handle_HAS_NAME_HAS_CHOICE(user_ptr, buffer, n);
				break;
			}
		}
	}
	return NULL;
}

void handle_NO_NAME_NO_CHOICE(struct UserData* user_ptr, char* buffer, int buffer_size) {
	printf("What is your name?\n");
	user_ptr->choice = STATE_HAS_NAME_NO_CHOICE;
}

void handle_HAS_NAME_NO_CHOICE(struct UserData* user_ptr, char* buffer, int buffer_size) {
	if (strcmp(buffer, "\n")) {
		printf("What is your name?\n");
		return;
	}
	printf("Rock, paper, or scissors?\n");
	user_ptr->choice = STATE_HAS_NAME_HAS_CHOICE;
	strcpy(user_ptr->username, buffer);
}

void handle_HAS_NAME_HAS_CHOICE(struct UserData* user_ptr, char* buffer, int buffer_size) {

}