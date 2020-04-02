#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct tdata {
    char d[200];
    char name[200];
}tdata;

void* move(void* arg) {
    tdata* d = (tdata*)arg;
    char oldpath[400], newpath[400];
    sprintf(oldpath, "%s/%s", d->d, d->name);
    char* ext = strrchr(d->name, '.');
    if (ext != NULL) {
        mkdir(ext+1, 0755);
        sprintf(newpath, "./%s/%s", ext+1, d->name);
    } else {
        mkdir("Unknown", 0755);
        sprintf(newpath, "./Unknown/%s", d->name);
    }
    rename(oldpath, newpath);
    free(d);
}

void perfile(int argc, char* argv[]) {
    for (int i = 2; i < argc; i++) {
        char* name = strrchr(argv[i], '/');
        if (name == NULL) name = argv[i];
        else name++;
        char* ext = strrchr(argv[i], '.');
        char newpath[400];
        if (ext != NULL) {
            mkdir(ext+1, 0755);
            sprintf(newpath, "./%s/%s", ext+1, name);
        } else {
            mkdir("Unknown", 0755);
            sprintf(newpath, "./Unknown/%s", name);
        }
        rename(argv[i], newpath);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    DIR* d;
    char dirname[200];
    if (argc == 2 && !strcmp(argv[1], "*")) {
        if ((d = opendir(".")) == NULL) {
            printf("Cannot open folder!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(dirname, ".");
    } else if (argc == 3 && !strcmp(argv[1], "-d")) {
        if ((d = opendir(argv[2])) == NULL) {
            printf("Cannot open folder!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(dirname, argv[2]);
    } else if (argc > 2 && !strcmp(argv[1], "-f")) {
        perfile(argc, argv);
    } else {
        printf("Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    int n = 0;
    struct dirent* ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_type == DT_REG) n++;
    }

    pthread_t threads[n];
    int index = 0;
    rewinddir(d);
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_type != DT_REG) continue;
        if (!strcmp(ent->d_name, argv[0]+2)) {
            threads[index] = 0;
            index++;
            continue;
        }
        tdata* data = malloc(sizeof(tdata));
        strcpy(data->d, dirname);
        strcpy(data->name, ent->d_name);
        pthread_create(&threads[index], NULL, move, (void*)data);
        index++;
    }
    for (int i = 0; i < n; i++) {
        if (threads[i]) pthread_join(threads[i], NULL);
    }
    exit(EXIT_SUCCESS);
}