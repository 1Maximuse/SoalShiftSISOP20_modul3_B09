#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* namapokemon[30] = {
	"Bulbasaur",
	"Charmander",
	"Squirtle",
	"Rattata",
	"Caterpie",
	"Pikachu",
	"Eevee",
	"Jigglypuff",
	"Snorlax",
	"Dragonite",
	"Mew",
	"Mewtwo",
	"Moltres",
	"Zapdos",
	"Articuno",
	"Shiny Bulbasaur",
	"Shiny Charmander",
	"Shiny Squirtle",
	"Shiny Rattata",
	"Shiny Caterpie",
	"Shiny Pikachu",
	"Shiny Eevee",
	"Shiny Jigglypuff",
	"Shiny Snorlax",
	"Shiny Dragonite",
	"Shiny Mew",
	"Shiny Mewtwo",
	"Shiny Moltres",
	"Shiny Zapdos",
	"Shiny Articuno"
};

int mode;
int mencari;
pthread_t threadcari;

int shmidpokemon;
int shmidlp;
int shmidpb;
int shmidb;
int* shmpokemon;
int* shmlp;
int* shmpb;
int* shmb;

int pokemoncapture;
pthread_t pokemoncapturethread;

int pokemonwrite;
int pokemon[7];
int pokemonAP[7];
pthread_t pokemonthread[7];

int pokedollar;
int lullaby;
int pokeball;
int berry;

int effect;
pthread_t lullabythread;

void printmainmenu() {
	if (mode == 0) {
		if (mencari) printf("[ Normal Mode ]\n1. Berhenti Mencari\n2. Pokedex\n3. Shop\n4. Go to capture mode\nInput: ");
		else printf("[ Normal Mode ]\n1. Cari Pokemon\n2. Pokedex\n3. Shop\n4. Go to capture mode\nInput: ");
	} else if (mode == 1) {
		if (pokemoncapture == -1) printf("[ Capture Mode ]\n1. Tangkap\n2. Item\n3. Keluar\nInput: ");
		else printf("[ Capture Mode ]\nMenemukan pokemon: %s\n1. Tangkap\n2. Item\n3. Keluar\nInput: ", namapokemon[pokemoncapture]);
	}
}

void* flullabythread() {
	while (1) {
		sleep(10);
	}
	effect = 0;
	printf("Efek lullaby powder habis.\n");
	pthread_exit(0);
}

void* fpokemonthread(void* arg) {
	int slot = *(int*)arg;
	while (1) {
		sleep(10);
		if (mode == 1) continue;
		while (pokemonwrite);
		pokemonwrite = 1;
		pokemonAP[slot] -= 10;
		if (pokemonAP[slot] == 0) {
			if (rand() % 100 < 90) {
				printf("Ada pokemon yang terlepas.\n");
				pokemon[slot] = -1;
				pokemonAP[slot] = -1;
				pthread_exit(0);
			} else {
				pokemonAP[slot] = 50;
			}
		}
		pokemonwrite = 0;
	}
}

void* fpokemoncapturethread() {
	int thres = (pokemoncapture % 15) / 5;
	if (thres == 0) thres = 5;
	else if (thres == 1) thres = 10;
	else if (thres == 2) thres = 20;
	if (pokemoncapture >= 15) thres += 5;
	while (1) {
		sleep(20);
		if (effect) continue;
		if (rand() % 100 < thres) {
			printf("Pokemon telah escape dari pencarian pokemon.\n");
			pokemoncapture = -1;
			pthread_exit(0);
		}
	}
}

void* fthreadcari() {
	while (1) {
		sleep(10);
		if (rand() % 100 < 60) {
			mencari = 0;
			pokemoncapture = *shmpokemon;
			pthread_create(&pokemoncapturethread, NULL, fpokemoncapturethread, NULL);
			mencari = 0;
			mode = 1;
			pthread_exit(0);
		}
	}
}

void caripokemon() {
	mencari = !mencari;
	if (mencari) {
		pthread_create(&threadcari, NULL, fthreadcari, NULL);
	} else {
		pthread_cancel(threadcari);
	}
}

void pokedex() {
	printf("Slot  Pokemon           Affection Points\n");
	for (int i = 0; i < 7; i++) {
		printf("%d     ", i+1);
		if (pokemon[i] == -1) {
			printf("-                 -\n");
		} else {
			printf("%-18s%d\n", namapokemon[pokemon[i]], pokemonAP[i]);
		}
	}
	printf("1. Lepas\n2. Beri semua pokemon Berry\n3. Keluar\nInput: ");
	int x;
	scanf("%d", &x);
	if (x == 1) {
		printf("Pilih pokemon (1-7): ");
		int poke = -1;
		while (poke == -1) {
			scanf("%d", &poke);
			if (poke >= 1 && poke <= 7 && pokemon[poke-1] != -1) {
				pthread_cancel(pokemonthread[poke-1]);
				pokemon[poke-1] = -1;
				pokemonAP[poke-1] = 0;
				break;
			}
			printf("Input invalid.\n");
		}
	} else if (x == 2) {
		while (pokemonwrite);
		pokemonwrite = 1;
		for (int i = 0; i < 7; i++) {
			if (pokemonAP[i] != -1) {
				pokemonAP[i] += 10;
			}
		}
		pokemonwrite = 0;
		printf("Berhasil memberi berry.\n");
	} else if (x == 3) {
		return;
	}
}

void shop() {
	printf("Pokedollar: %d\n", pokedollar);
	printf("No  Nama            Stok  Harga\n");
	printf("1.  Lullaby Powder  %3d   60\n", *shmlp);
	printf("2.  Pokeball        %3d   5\n", *shmpb);
	printf("3.  Berry           %3d   15\n", *shmb);
	printf("4.  Keluar\n");
	printf("Beli atau keluar: ");
	int x;
	scanf("%d", &x);
	if (x == 4) return;
	printf("Jumlah: ");
	int jml;
	scanf("%d", &jml);
	if (jml > 99) {
		printf("Gagal membeli.\n");
		return;
	}
	if (x == 1) {
		if (jml <= *shmlp && lullaby + jml <= 99 && pokedollar >= jml * 60) {
			*shmlp -= jml;
			pokedollar -= jml * 60;
			printf("Berhasil membeli %d Lullaby Powder.\n", jml);
			return;
		}
	} else if (x == 2 && pokeball + jml <= 99 && pokedollar >= jml * 5) {
		if (jml <= *shmpb) {
			pokedollar -= jml * 5;
			printf("Berhasil membeli %d Pokeball.\n", jml);
			return;
		}
	} else if (x == 3 && berry + jml <= 99 && pokedollar >= jml * 15) {
		if (jml <= *shmb) {
			*shmb -= jml;
			pokedollar -= jml * 15;
			printf("Berhasil membeli %d Berry.\n", jml);
			return;
		}
	} else if (x == 4) {
		return;
	}
	printf("Gagal membeli.\n");
}

void tangkap() {
	if (pokemoncapture == -1) {
		printf("Tidak ada pokemon.\n");
		return;
	}
	if (pokeball == 0) {
		printf("Tidak ada pokeball.\n");
		return;
	}
	pokeball--;
	int thres = (pokemoncapture % 15) / 5;
	if (thres == 0) thres = 70;
	else if (thres == 1) thres = 50;
	else if (thres == 2) thres = 30;
	if (pokemoncapture >= 15) thres -= 20;
	if (effect) thres += 20;
	if (rand() % 100 < thres) {
		printf("Berhasil menangkap %s!\n", namapokemon[pokemoncapture]);
		while (pokemonwrite);
		pokemonwrite = 1;
		int slot = -1;
		for (int i = 0; i < 7; i++) {
			if (pokemon[i] == -1) {
				slot = i;
			}
		}
		if (slot == -1) {
			int money = (pokemoncapture % 15) / 5;
			if (money == 0) money = 80;
			else if (money == 1) money = 100;
			else if (money == 2) money = 200;
			if (pokemoncapture >= 15) money += 5000;
			pokedollar += money;
			printf("Slot pokemon penuh! Anda mendapatkan %d.\n", money);
		} else {
			pokemon[slot] = pokemoncapture;
			pokemonAP[slot] = 100;
			pthread_create(&pokemonthread[slot], NULL, fpokemonthread, (void*)&slot);
			pthread_cancel(pokemoncapturethread);
			pokemoncapture = -1;
		}
		pokemonwrite = 0;
	} else {
		printf("Tidak berhasil menangkap %s.\n", namapokemon[pokemoncapture]);
	}
}

void useitem() {
	if (lullaby <= 0) {
		printf("Lullaby Powder tidak cukup.\n");
		return;
	}
	if (effect) {
		printf("Efek lullaby powder masih ada.\n");
		return;
	}
	printf("Gunakan lullaby powder?\n1. Ya\n2. Tidak\nInput: ");
	int x;
	scanf("%d", &x);
	if (x == 1) {
		lullaby--;
		effect = 1;
		pthread_create(&lullabythread, NULL, flullabythread, NULL);
	}
}

void cleanup() {
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

int main() {
	srand(time(NULL));
	signal(SIGKILL, cleanup);
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
	pokemoncapture = -1;
	memset(pokemon, -1, sizeof(pokemon));
	memset(pokemonAP, -1, sizeof(pokemonAP));
	memset(pokemonthread, 0, sizeof(pokemonthread));
	mode = 0;
	mencari = 0;
	pokemonwrite = 0;
	lullaby = 0;
	pokeball = 0;
	berry = 0;
	effect = 0;
	pokedollar = 0;

	while (1) {
		printmainmenu();
		int x;
		scanf("%d", &x);
		if (mode == 0) {// NORMAL
			if (x == 1) {
				caripokemon();
			} else if (x == 2) {
				pokedex();
			} else if (x == 3) {
				shop();
			} else if (x == 4) {
				mode = 1;
			}
		} else if (mode == 1) { // CAPTURE
			if (x == 1) {
				tangkap();
			} else if (x == 2) {
				useitem();
			} else if (x == 3) {
				mode = 0;
			}
		}
	}
}