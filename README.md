# SoalShiftSISOP20_modul3_B09

Soal shift Sistem Operasi 2020
Modul 3
Kelompok B09

## #1 &ndash; Poke\*ZONE

>Source code: [soal1_traizone.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal1/soal1_traizone.c), [soal1_pokezone.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal1/soal1_pokezone.c)
---
### pokezone
Pada program ini terdapat tiga fungsi yang harus dibuat, yaitu generasi Pokemon secara random, mengelola stok toko, dan terakhir untuk keluar dari kedua program.

Sebelum memulai ketiga tugas tersebut, program melakukan inisialisasi yaitu mengalokasikan dan menentukan alamat *shared memory* yang akan digunakan. *Shared memory* yang digunakan ada empat, yaitu untuk random Pokemon, dan tiga lainnya untuk jumlah stok ketiga barang yang bisa dibeli di toko.

---
Untuk melakukan generasi Pokemon, dibentuk sebuah *thread* baru yang akan berjalan secara *infinite loop* dengan delay waktu satu detik. Pada setiap detik, program akan menggunakan fungsi `rand()` untuk menentukan Pokemon yang terpilih sesuai dengan probabilitas yang diminta.

Pada implementasinya, Pokemon berupa angka dari 0 sampai dengan 29 (inklusif), dengan masing-masing angka merupakan jenis pokemon yang berbeda.

Hasil generasi kemudian disimpan pada *shared memory* yang telah dialokasikan untuk random Pokemon.
```c
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
```
---
Untuk mengelola stok toko, dibuat lagi sebuah *thread* yang akan memastikan jumlah stok bertambah setiap 10 detik dan tidak melebihi 200 untuk setiap item nya. Semua operasi yang dilakukan langsung dioperasikan pada ketiga alamat *shared memory* yang sudah dialokasikan di awal.
```c
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
```
---
Yang terakhir, sembari kedua *thread* tersebut berjalan, program akan menunggu inputan dari pengguna. Apabila pengguna menginputkan untuk *quit game*, maka program akan menjalankan tugas ketiganya, yaitu melakukan *kill* proses program `traizone` kemudian keluar.

Untuk melakukan *kill* dapat dengan mudah menggunakan fungsi bawaan dari bahasa C, yaitu `kill()`. Namun, karena diminta untuk menggunakan `exec()`, maka caranya sedikit lebih panjang.

Pertama, program membuka direktori `/proc` untuk melihat semua proses-proses yang ada. Di dalam folder `/proc` merupakan subfolder yang bernama sesuai dengan *process ID* suatu proses yang berjalan. Program akan melakukan *looping* untuk semua subfolder, namun melewati yang tidak sepenuhnya numerik (bukan folder *process ID*).

Untuk setiap subfolder, program membaca file `/proc/<pid>/cmdline` yang akan berisi detil proses yang berkaitan. Apabila file ada, maka kata pertama dicocokkan dengan nama proses yang diinginkan (`./soal1_traizone`).

Setelah dipastikan bahwa nama proses sesuai, maka kita dapat melakukan `exec()` untuk mengirim sinyal `SIGINT` kepada *process ID* `./soal1_traizone`. Sinyal `SIGINT` digunakan dan bukan `SIGKILL` atau `SIGTERM` karena `SIGINT` dapat diterima terlebih dahulu oleh program yang bersangkutan untuk melakukan pembersihan seperti *detach* memori, dan sebagainya.

---
### traizone
Untuk program ini, sebelum dimulai pertama-tama program akan melakukan inisialisasi hal-hal yang dibutuhkan terlebih dahulu, yaitu alamat *shared memory*, *mutex* untuk *thread* yang akan dibuat, *event-handling* untuk `SIGINT`, dan yang terakhir nilai-nilai untuk variabel yang dibuat seperti uang awal, jumlah Pokeball awal, dan sebagainya.

Program ini mempergunakan beberapa *thread*, sebagai berikut:
1. `threadcari`, untuk melakukan pencarian pokemon setiap 10 detik.
2. `inputthread`, untuk meng-*handle* inputan sehingga dapat digagalkan.
3. `pokemoncapturethread` untuk menentukan probabilitas *escape* dari pokemon yang ketemu dari fitur cari pokemon (setiap 20 detik).
4. `pokemonthread` untuk meng-*handle* pengurangan AP setiap 10 detik. Thread ini jumlahnya menyesuaikan dengan jumlah pokemon yang dimiliki.
5. `lullabythread` untuk mengatur efek dari item *lullaby* yang akan habis setelah 10 detik.

Program akan berlanjut menuju loop utama. Loop ini akan berjalan secara *infinite*, dan untuk setiap iterasi pertama-tama akan mencetak menu utama menggunakan fungsi `printmainmenu()`. Main menu yang dicetak akan menyesuaikan dengan mode yang sedang berlaku (normal atau capture).

Lalu, program akan membentuk sebuah *thread* untuk membaca inputan. Fungsinya, agar sewaktu-waktu apabila program ingin menghentikan proses input data (misalnya, berhasil menemukan Pokemon) maka *thread* inputan dapat di-*cancel* sehingga program dapat mencetak menu utama lagi (untuk mode capture mode).

Berdasarkan inputan tersebut, program akan menjalankan fungsi-fungsi yang sesuai:
1. `caripokemon()`, memulai dan menghentikan *thread* untuk mencari Pokemon.
2. `pokedex()`, yang akan melakukan perulangan sebanyak 7 kali dan menampilkan Pokemon yang dimiliki. Kemudian, bisa memberikan Berry dan melepas Pokemon.
3. `shop()`, yang akan menampilkan toko dengan membaca *shared memory*. Pemain dapat membeli dari sini dan mengurangi stok toko.
4. `tangkap()`, pemain dapat menggunakan Pokeball untuk menangkap Pokemon yang telah ditemukan sesuai dengan persentase probabilitasnya.
5. `useitem()`, pemain dapat menggunakan Lullaby Powder dan mengaktifkan efeknya selama 10 detik.

Program akan berjalan terus sampai menerima sinyal `SIGINT`, kemudian akan menjalankan `cleanup()`, untuk melakukan *detach* dari *shared memory*, kemudian melakukan dealokasi *shared memory* tersebut. Kemudian, program akan menghapus *mutex* yang telah dibuat dan program akan keluar.

## #2 &ndash; TapTap Game
>Source code: [tapclient.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal2/tapclient.c), [tapserver.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal2/tapserver.c)

### Server
Pertama, program akan membuka sebuah *socket* dan siap menerima koneksi. Selain itu, program juga akan membaca (atau membuat baru jika tidak ada) file `akun.txt`, dan membaca data pada file tersebut untuk disimpan pada sebuah *array of struct*, dengan tipe data yang dibuat untuk menyimpan username serta password.

Server kemudian menyiapkan 2 *thread* untuk dua client yang akan melakukan koneksi, dan menunggu ada koneksi. Untuk setiap koneksi, tanpa menunggu koneksi lainnya maka server akan membentuk *thread* dengan fungsi `serve()` untuk melayani setiap client yang terhubung. 

*Thread* tersebut melakukan akan menerima kiriman data dari client berupa mode (0 untuk login, 1 untuk register), username, dan password. Lalu, username dan password dipastikan (untuk login) atau diinputkan sebagai data user baru.

Setiap client dan server akan saling mengirim dan menerima data untuk memastikan sinkronisasi, termasuk di mana posisi client saat ini (ready to play, atau belum login, dan sebagainya). Server melakukan penghitungan untuk jumlah client yang telah melakukan *find match*. Apabila sudah dua, maka kedua *thread* yang aktif tersebut akan berhenti.

Thread program utama yang sejak awal menunggu kedua *thread* ini untuk berhenti dengan menggunakan `pthread_join()` kemudian melakukan inisialisasi awal data pemain yang ada (*health* di awal 100). Kemudian, secara bersamaan program akan membuat dua *thread* baru.

Kedua *thread* baru ini akan terus menerus membaca kiriman dari setiap client untuk setiap penekanan tombol spasi. Untuk setiap data yang dikirim, *thread* akan mengurangi nyawa lawan dengan 10. Pada tahap ini tidak diperlukan *mutual exclusion* karena setiap pemain hanya mengubah data pemain lawan, sehingga tidak ada dua pemain yang mengubah data yang sama. *Thread* juga mengirim data nyawa kepada lawan.

```c
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
```

*Thread* utama akan menunggu sampai salah satu nyawa pemain mencapai 0. Apabila sudah, maka kedua *thread* game tersebut dihentikan dengan `pthread_cancel()`, dan server mengirim data ke client bahwa game telah selesai, dengan kondisi (menang atau kalah) yang sesuai kepada kedua client.

Terakhir, program akan membuat lagi dua *thread* untuk melayani kedua client tersebut, namun kali ini tanpa login karena client telah login sebelumnya.

Program ini akan berjalan secara *infinite*.

---
### Client

Sewajarnya, client akan memulai dengan melakukan koneksi dengan server. Setelah terkoneksi, client akan menampilkan terlebih dahulu menu untuk login atau register.

Dari menu ini client akan mengirim data kepada server dengan data yang sesuai spesifikasi server. Client kemudian menerima data dari server terkait dengan sukses atau gagalnya login, dan sukses untuk register.

Setelah masuk, pemain dapat memilih untuk keluar, di mana program akan melakukan `goto` kembali ke menu sebelum login (dan mengirim data kepada server agar siap menerima data login lagi), atau pemain dapat memilih untuk *find match*.

Apabila pemain memilih *find match*, client akan membuat sebuah *thread* yang akan mencetak pesan menunggu setiap detik. *Thread* utama kemudian menunggu server mengirim data bahwa permainan siap berjalan, sehingga *thread* menunggu dapat dihentikan dan game dimainkan.

Pada saat game dimainkan, pertama program akan mengubah mode *terminal* agar input tidak memerlukan untuk menekan `Enter`, dengan kode berikut:

```c
struct termios old, current;
tcgetattr(0, &old);
current = old;
current.c_lflag &= ~ICANON;
current.c_lflag &= ~ECHO;
tcsetattr(0, TCSANOW, &current); 
...
tcsetattr(0, TCSANOW, &old);
```
Lalu, client membuat *thread* baru yang akan meng-*handle* inputan dengan `getchar()` dan mengirim ke server. Selama *thread* itu berjalan, *thread* utama akan menerima data dari server yang akan menampilkan nyawa yang tersisa.

Apabila server mengirimkan suatu kode khusus (`-1337`), maka client akan mengetahui bahwa game telah berakhir. Maka, *thread* untuk menerima inputan akan dihentikan dan client membaca dari server untuk informasi siapa yang menang.

Selesai dari game, pemain akan dikembalikan ke menu setelah login (screen 2).

## #3 &ndash; Mengkategorikan File
> Source code: [soal3.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal3/soal3.c)

---
Pada soal ini, yang pertama harus dilakukan adalah membagi-bagi sesuai dengan tipe tindakan yang ingin dilakukan. Untuk argumen `-d` dan `\*` dapat dijadikan satu, dengan `\*` menggunakan direktori *working directory*.

Pertama, program akan menentukan jumlah thread yang perlu dibuat dengan melakukan perulangan sejumlah file-file yang valid. Setelah itu, program akan melakukan perulangan lagi, hanya saja kali ini membentuk thread untuk memindahkan sesuai dengan ekstensi file.

Pada setiap thread, program akan membaca nama file dan mengambil nama ekstensinya sebagai berikut.
```c
char* ext = strrchr(d->name, '.');
```

Apabila file memiliki ekstensi, maka pertama-tama ekstensi tersebut diubah jadi *lowercase* dengan kode berikut.
```c
void lower(char* s) {
    for (int i = 0; i < strlen(s); i++) {
        s[i] = tolower(s[i]);
    }
}
```
Setelah itu, folder dengan nama sesuai ekstensi *lowercase* tersebut dibuat, dan file dipindah dengan menggunakan fungsi `rename()` ke folder yang bersangkutan. Apabila ekstensi tidak ada atau `NULL`, maka file dipindah ke subfolder `Unknown`.

Untuk argumen `-f`, program akan melakukan hal yang sama, hanya saja bedanya program tidak perlu melakukan looping untuk semua file-file, sedangkan langsung melakukan pemindahan dari argumen yang diinputkan.

## #4 &ndash; Teka-teki Norland
> Source code: [4a.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal4/4a.c), [4b.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal4/4b.c), [4c.c](https://github.com/1Maximuse/SoalShiftSISOP20_modul3_B09/blob/master/soal4/4c.c)

---
### a. Perkalian Matriks
Pada soal 4a, diminta untuk mengalikan matriks 4x2 dengan matriks 2x5, dengan hasil matriks 4x5. Hasil dari perkalian ini akan diproses pada soal berikutnya. Untuk dapat memberi output dari 4a ke soal 4b, maka digunakan shared memory, dengan tidak lupa untuk melakukan *detach* setelah digunakan. Berikut syntax untuk memberi share memory kepada soal no 4b.

```c
for (c = 0; c < 4; c++) 
{
	for (d = 0; d < 5; d++)
	{
	*value = multiply[c][d];
	sleep(2);
	printf("%d\t",*value);
	}
	printf("\n");
}
```
	
### b. Faktorial
Pada soal 4b, kita diminta untuk mencari hasil dari faktorial setiap array yang telah dikirimkan oleh soal no 4a, dengan catatan harus menggunakan thread. Maka berikut adalah output hasil faktorial dengan menggunakan thread.

```c
for (int i = 0;i < 20;i++)
{
	arr[i]=*value;
	angka=arr[i];
	pthread_create(&tid[idx],NULL, &fac, (void*)angka);
	idx++;
	sleep(2);
	if(i%5==4 && i>0)
	{
		printf("\n");
	}
	else
	{
		printf("\t\t");
	}
}
for(int i=0; i< idx; i++)
{
	pthread_join(tid[i],NULL);
}
```

### c. Word Count
Pada soal no 4c, kita diminta untuk mencari wc (word count) pada suatu directory. Berikut fungsi untuk mencari word count dengan menggunakan pipe.

```c
if (pipe(pipe1) == -1)
	exit(1);

if ((fork()) == 0) 
{
	dup2(pipe1[1], 1);
	close(pipe1[0]);
	close(pipe1[1]);
	char *argv1[] = {"ls", NULL};
	execv("/bin/ls", argv1);
}

if (pipe(pipe2) == -1) 
	exit(1);
else
{
	dup2(pipe1[0], 0);
	close(pipe1[0]);
	close(pipe1[1]);
	char *argv1[] = {"wc", "-l", NULL};
	execv("/usr/bin/wc", argv1);
}
```
