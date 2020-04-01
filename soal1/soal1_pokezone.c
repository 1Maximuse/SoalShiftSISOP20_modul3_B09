#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int* shmpokemon;
int* shmlp;
int* shmpb;
int* shmb;

pthread_t restockthread;
pthread_t randomthread;

void* frestockthread() {
    while (1) {
        sleep(10);
        *shmlp += 10;
        if (*shmlp > 200) *shmlp = 200;
        *shmpb += 10;
        if (*shmpb > 200) *shmpb = 200;
        *shmb += 10;
        if (*shmb > 200) *shmb = 200;
    }
}

void* frandomthread() {
    while (1) {
        int poke;
        int r = rand() % 100;
        if (r < 5) {
            poke = (rand() % 5) + 10;
        } else if (r < 15) {
            poke = (rand() % 5) + 5;
        } else {
            poke = (rand() % 5);
        }
        if (rand() % 8000 == 0) {
            poke += 15;
        }
        *shmpokemon = poke;
        sleep(1);
    }
}

int main() {
	srand(time(NULL));
	key_t key1 = 1000;
	key_t key2 = 1001;
	key_t key3 = 1002;
	key_t key4 = 1003;
    int shmidpokemon = shmget(key1, sizeof(int), IPC_CREAT | 0666);
    int shmidlp = shmget(key2, sizeof(int), IPC_CREAT | 0666);
    int shmidpb = shmget(key3, sizeof(int), IPC_CREAT | 0666);
    int shmidb = shmget(key4, sizeof(int), IPC_CREAT | 0666);
	shmpokemon = shmat(shmidpokemon, NULL, 0);
	shmlp = shmat(shmidlp, NULL, 0);
	shmpb = shmat(shmidpb, NULL, 0);
	shmb = shmat(shmidb, NULL, 0);
    *shmpokemon = 100;
    *shmlp = 100;
    *shmpb = 100;
    *shmb = 100;
    pthread_create(&restockthread, NULL, frestockthread, NULL);
    pthread_create(&randomthread, NULL, frandomthread, NULL);
    printf("1. Quit game\nInput: ");
    int x;
    scanf("%d", &x);
    if (x == 1) {
        DIR* dir = opendir("/proc");
        struct dirent* ent;
        char* endptr;
        while ((ent = readdir(dir)) != NULL) {
            long lpid = strtol(ent->d_name, &endptr, 10);
            if (*endptr != '\0') continue;

            char buf[512];
            snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
            FILE* fp = fopen(buf, "r");
            if (fp) {
                if (fgets(buf, sizeof(buf), fp) != NULL) {
                    char* first = strtok(buf, " ");
                    if (strcmp(first, "./soal1_traizone") == 0) {
                        pid_t child = fork();
                        if (child == 0) {
                            char cmd[500];
                            sprintf(cmd, "kill -INT %ld", lpid);
                            execl("/bin/sh", "sh", "-c", cmd, NULL);
                        }
                    }
                }
                fclose(fp);
            }
        }
        closedir(dir);
        int status;
        while(wait(&status) > 0);


        shmdt(shmpokemon);
        shmdt(shmlp);
        shmdt(shmpb);
        shmdt(shmb);
        shmctl(shmidpokemon, IPC_RMID, NULL);
        shmctl(shmidlp, IPC_RMID, NULL);
        shmctl(shmidpb, IPC_RMID, NULL);
        shmctl(shmidb, IPC_RMID, NULL);
        exit(EXIT_SUCCESS);
    }
}