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

int input;
pthread_t inputthread;

int shmidpokemon;
int shmidlp;
int shmidpb;
int shmidb;
int* shmpokemon;
int* shmlp;
int* shmpb;
int* shmb;


pthread_mutex_t pokeslotwrite;
int pokeslot;

pthread_mutex_t pokemoncapturewrite;
int pokemoncapture;
pthread_t pokemoncapturethread;

pthread_mutex_t pokemonwrite;
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
	printf("\n");
	if (mode == 0) {
		if (mencari) printf("[ Normal Mode ]\n1. Berhenti Mencari\n2. Pokedex\n3. Shop\n4. Go to capture mode\nInput: ");
		else printf("[ Normal Mode ]\n1. Cari Pokemon\n2. Pokedex\n3. Shop\n4. Go to capture mode\nInput: ");
	} else if (mode == 1) {
		if (pokemoncapture == -1) printf("[ Capture Mode ]\n1. Tangkap\n2. Item\n3. Keluar\nInput: ");
		else printf("[ Capture Mode ]\nMenemukan pokemon: %s\n1. Tangkap\n2. Item\n3. Keluar\nInput: ", namapokemon[pokemoncapture]);
	}
}

void* flullabythread() {
	sleep(10);
	effect = 0;
	printf("Efek lullaby powder habis.\n");
	pthread_exit(0);
}

void* fpokemonthread() {
	int slot = pokeslot;
	pthread_mutex_unlock(&pokeslotwrite);
	while (1) {
		sleep(10);
		if (mode == 1) continue;
		pthread_mutex_lock(&pokemonwrite);
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
		pthread_mutex_unlock(&pokemonwrite);
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
			pthread_mutex_lock(&pokemoncapturewrite);
			pokemoncapture = -1;
			pthread_mutex_unlock(&pokemoncapturewrite);
			pthread_exit(0);
		}
	}
}

void* fthreadcari() {
	while (1) {
		sleep(10);
		if (rand() % 100 < 60) {
			mencari = 0;
			pthread_mutex_lock(&pokemoncapturewrite);
			pokemoncapture = *shmpokemon;
			pthread_mutex_unlock(&pokemoncapturewrite);
			pthread_create(&pokemoncapturethread, NULL, fpokemoncapturethread, NULL);
			mencari = 0;
			mode = 1;
			input = -1;
			pthread_cancel(inputthread);
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
				pthread_mutex_lock(&pokemonwrite);
				pthread_cancel(pokemonthread[poke-1]);
				if (pokemon[poke-1] >= 15) {
					pokedollar += 5000;
					pokemon[poke-1] -= 15;
				}
				if (pokemon[poke-1] < 5) {
					pokedollar += 80;
				} else if (pokemon[poke-1] < 10) {
					pokedollar += 100;
				} else if (pokemon[poke-1] < 15) {
					pokedollar += 200;
				}
				pokemon[poke-1] = -1;
				pokemonAP[poke-1] = 0;
				pthread_mutex_unlock(&pokemonwrite);
				break;
			}
			printf("Input invalid.\n");
		}
	} else if (x == 2) {
		pthread_mutex_lock(&pokemonwrite);
		for (int i = 0; i < 7; i++) {
			if (pokemonAP[i] != -1) {
				pokemonAP[i] += 10;
			}
		}
		pthread_mutex_unlock(&pokemonwrite);
		printf("Berhasil memberi berry.\n");
	} else if (x == 3) {
		return;
	}
}

void shop() {
	printf("Pokedollar: %d\n", pokedollar);
	printf("No  Nama            Punya  Stok  Harga\n");
	printf("1.  Lullaby Powder  %-3d    %-3d   60\n", lullaby, *shmlp);
	printf("2.  Pokeball        %-3d    %-3d   5\n", pokeball, *shmpb);
	printf("3.  Berry           %-3d    %-3d   15\n", berry, *shmb);
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
			lullaby += jml;
			printf("Berhasil membeli %d Lullaby Powder.\n", jml);
			return;
		}
	} else if (x == 2) {
		if (jml <= *shmpb && pokeball + jml <= 99 && pokedollar >= jml * 5) {
			*shmpb -= jml;
			pokedollar -= jml * 5;
			pokeball += jml;
			printf("Berhasil membeli %d Pokeball.\n", jml);
			return;
		}
	} else if (x == 3) {
		if (jml <= *shmb && berry + jml <= 99 && pokedollar >= jml * 15) {
			*shmb -= jml;
			pokedollar -= jml * 15;
			berry += jml;
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
		int slot = -1;
		for (int i = 0; i < 7; i++) {
			if (pokemon[i] == -1) {
				slot = i;
				break;
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
			pthread_mutex_lock(&pokemonwrite);
			pokemon[slot] = pokemoncapture;
			pokemonAP[slot] = 100;
			pthread_mutex_unlock(&pokemonwrite);
			pthread_mutex_lock(&pokeslotwrite);
			pokeslot = slot;
			pthread_create(&pokemonthread[slot], NULL, fpokemonthread, NULL);
			pthread_cancel(pokemoncapturethread);
			pthread_mutex_lock(&pokemoncapturewrite);
			pokemoncapture = -1;
			pthread_mutex_unlock(&pokemoncapturewrite);
		}
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
	pthread_mutex_destroy(&pokemonwrite);
	pthread_mutex_destroy(&pokemoncapturewrite);
	pthread_mutex_destroy(&pokeslotwrite);
	exit(EXIT_SUCCESS);
}

void* finputthread() {
	scanf("%d", &input);
}

int main() {
	srand(time(NULL));
	signal(SIGINT, cleanup);
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
	pthread_mutex_init(&pokemonwrite, NULL);
	pthread_mutex_init(&pokemoncapturewrite, NULL);
	pthread_mutex_init(&pokeslotwrite, NULL);
	mode = 0;
	mencari = 0;
	lullaby = 0;
	pokeball = 0;
	berry = 0;
	effect = 0;
	pokedollar = 500;
	pokeslot = -1;

	while (1) {
		printmainmenu();
		pthread_create(&inputthread, NULL, finputthread, NULL);
		pthread_join(inputthread, NULL);
		if (input == -1) {
			printf("\n");
			continue;
		}
		int x = input;
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
