#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080

typedef struct user {
	char username[200];
	char password[200];
}user;

typedef struct clientdata {
	int client;
	int skiplogin;
}clientdata;

int connected;
int connectedwrite;
int accountwrite;
int filewrite;
int accountn;
user accounts[100];

typedef struct gamedata {
	int client, cenemy;
	int* health1;
	int* health2;
}gamedata;

void* game(void* arg) {
	gamedata data = *(gamedata*)arg;
	int client = data.client;
	int cenemy = data.cenemy;
	int* own = data.health1;
	int* enemy = data.health2;
	int go = 1;
	send(client, &go, sizeof(go), 0);
	// printf("%d: %p, %p\n", client, own, enemy);
	while (1) {
		char c;
		read(client, &c, sizeof(c));
		*enemy -= 10;
		send(cenemy, enemy, sizeof(*enemy), 0);
	}
}

void* serve(void* arg) {
	clientdata d = *(clientdata*)arg;
	int client = d.client;
	int skiplogin = d.skiplogin;
	if (!skiplogin) {
		screen1:;
		char data[500];
		memset(data, 0, sizeof(data));
		int mode;
		char username[200], password[200];
		read(client, data, sizeof(data));
		mode = data[0] - '0';
		int i = 2;
		for (i = 2; i < strlen(data); i++) {
			if (data[i] == '\t') break;
			username[i-2] = data[i];
		}
		username[i-2] = '\0';
		int ii = ++i;
		for (; i < strlen(data); i++) {
			if (data[i] == '\t') break;
			password[i-ii] = data[i];
		}
		if (mode == 0) { // LOGIN
			int success = 0;
			while (accountwrite);
			for (int i = 0; i < accountn; i++) {
				if (strcmp(username, accounts[i].username) == 0 && strcmp(password, accounts[i].password) == 0) {
					success = 1;
					break;
				}
			}
			send(client, &success, sizeof(success), 0);
			if (success) printf("Auth success\n");
			else {
				printf("Auth failed\n");
				goto screen1;
			}
		} else if (mode == 1) { // REGISTER
			accountwrite = 1;
			user newacc;
			strcpy(newacc.username, username);
			strcpy(newacc.password, password);
			accounts[accountn] = newacc;
			while (filewrite);
			filewrite = 1;
			FILE *f = fopen("akun.txt", "a");
			fprintf(f, "%s\t%s\n", username, password);
			fclose(f);
			filewrite = 0;
			accountn++;
			accountwrite = 0;
			for (int i = 0; i < accountn; i++) {
				printf("%s\t%s\n", accounts[i].username, accounts[i].password);
			}
		}
	}
	skiplogin = 0;
	int mode;
	read(client, &mode, sizeof(mode));
	if (mode == 0) { // FIND MATCH
		while (connectedwrite);
		connectedwrite = 1;
		connected++;
		connectedwrite = 0;
		while (connected < 2);
	} else if (mode == 1) { // LOGOUT
		goto screen1;
	}
	pthread_exit(0);
}

int main() {
	int sock;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	  
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	if (bind(sock, (struct sockaddr *)&address, sizeof(address))<0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(sock, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	FILE* f = fopen("akun.txt", "a+");
	char username[200], password[200];
	int index = 0;
	while (fscanf(f, "%[^\t]\t%[^\n]\n", username, password) != EOF) {
		user newacc;
		strcpy(newacc.username, username);
		strcpy(newacc.password, password);
		accounts[index] = newacc;
		index++;
	}
	fclose(f);
	accountn = index;
	accountwrite = 0;
	filewrite = 0;
	
	connected = 0;
	connectedwrite = 0;

	pthread_t serveThreads[2];
	int client[2];
	for (int i = 0; i < 2; i++) {
		client[i] = accept(sock, (struct sockaddr *)&address, (socklen_t*)&addrlen);
		if (client[i] < 0) {
			perror("Cannot accept client.\n");
			i--;
			continue;
		}
		printf("Accepted client.\n");
		clientdata c;
		c.client = client[i];
		c.skiplogin = 0;
		pthread_create(&serveThreads[i], NULL, serve, (void*)&c);
	}

	while (1) {
		pthread_join(serveThreads[0], NULL);
		pthread_join(serveThreads[1], NULL);
		// printf("start\n");
		int h1write = 0;
		int h2write = 0;
		int health1 = 100;
		int health2 = 100;
		gamedata data1, data2;
		data1.client = client[0];
		data1.cenemy = client[1];
		data1.health1 = &health1;
		data1.health2 = &health2;
		data2.client = client[1];
		data2.cenemy = client[0];
		data2.health1 = &health2;
		data2.health2 = &health1;
		pthread_create(&serveThreads[0], NULL, game, (void*) &data1);
		pthread_create(&serveThreads[1], NULL, game, (void*) &data2);

		while (1) {
			if (health1 <= 0 || health2 <= 0) break;
		}
		pthread_cancel(serveThreads[0]);
		pthread_cancel(serveThreads[1]);
		int stop = -1337;
		int win = 1, lose = 0;
		send(client[0], &stop, sizeof(stop), 0);
		send(client[1], &stop, sizeof(stop), 0);
		connected -= 2;
		if (health1 <= 0) {
			send(client[0], &lose, sizeof(lose), 0);
			send(client[1], &win, sizeof(win), 0);
		} else if (health2 <= 0) {
			send(client[0], &win, sizeof(win), 0);
			send(client[1], &lose, sizeof(lose), 0);
		}
		
		clientdata c1, c2;
		c1.client = client[0];
		c1.skiplogin = 1;
		c2.client = client[1];
		c2.skiplogin = 1;
		pthread_create(&serveThreads[0], NULL, serve, (void*)&c1);
		pthread_create(&serveThreads[1], NULL, serve, (void*)&c2);
	}
}