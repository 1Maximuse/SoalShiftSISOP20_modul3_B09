#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <termios.h> 
#define PORT 8080

void* waiting() {
    while (1) {
        printf("Waiting for player ...\n");
        sleep(1);
    }
}

void* taptap(void* arg) {
    int sock = *(int*) arg;
    while (1) {
        char c = getchar();
        if (c == ' ') send(sock, &c, sizeof(c), 0);
    }
}

void game(int sock) {
    printf("Game dimulai silakan \e[3mtap tap\e[0m secepat mungkin !!\n");
    struct termios old, current;
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    current.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &current); 

    pthread_t gamethread;
    pthread_create(&gamethread, NULL, taptap, (void*) &sock);
    
    int status = 100;
    while (1) {
        read(sock, &status, sizeof(status));
        if (status == -1337) break;
        printf("Health: %d\n", status);
    }
    pthread_cancel(gamethread);
    int win;
    read(sock, &win, sizeof(win));
    if (win) printf("Game berakhir kamu menang\n");
    else printf("Game berakhir kamu kalah\n");

    tcsetattr(0, TCSANOW, &old);
}
  
int main(int argc, char const *argv[]) {
    int sock;
    struct sockaddr_in address;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }
  
    memset(&address, '0', sizeof(address));
  
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
  
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }

    screen1:;
    int mode = -1;
    while (mode == -1) {
        char input[20];
        printf("1. Login\n2. Register\nChoices: ");
        scanf("%s", input);
        if (strcmp(input, "login") == 0) {
            mode = 0;
            break;
        } else if (strcmp(input, "register") == 0) {
            mode = 1;
            break;
        }
        printf("\nInvalid input.\n");
    }
    char username[210], password[200];
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    username[0] = mode + '0';
    username[1] = '\t';
    password[0] = '\t';
    printf("Username: ");
    getchar();
    scanf("%[^\n]", username+2);
    getchar();
    printf("Password: ");
    scanf("%[^\n]", password+1);
    strcat(username, password);
    send(sock, username, strlen(username), 0);
    if (mode == 0) {// LOGIN
        int resp;
        read(sock, &resp, sizeof(resp));
        if (resp) printf("login success\n");
        else {
            printf("login failed\n");
            goto screen1;
        }
    } else if (mode == 1) { // REGISTER
        printf("register success\n");
    }
    screen2:;
    mode = -1;
    while (mode == -1) {
        printf("1. Find Match\n2. Logout\nChoices: ");
        char input[20];
        scanf("%s", input);
        if (strcmp(input, "find") == 0) {
            mode = 0;
            break;
        } else if (strcmp(input, "logout") == 0) {
            mode = 1;
            break;
        }
        printf("\nInvalid input.\n");
    }
    send(sock, &mode, sizeof(mode), 0);
    if (mode == 0) { // FIND MATCH
        pthread_t wthread;
        pthread_create(&wthread, NULL, waiting, NULL);
        int go;
        read(sock, &go, sizeof(go));
        pthread_cancel(wthread);
        game(sock);
        goto screen2;
    } else if (mode == 1) { // LOGOUT
        goto screen1;
    }
}