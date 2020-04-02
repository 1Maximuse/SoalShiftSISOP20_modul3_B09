#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

typedef struct data {
	int angka;
	int hasil;
}data;

int jumlah(int n) 
{ 
	int sum=0;
	for(n;n>0;n--)
	{
		sum+=n;
	}
	return sum;
}

void *jumn(void* arg)
{
	data* d = (data*) arg;
        d->hasil=jumlah(d->angka);
}
void main()
{
	key_t key = 1234;
	int *value;
	int shmid = shmget(key, sizeof(int) * 20, IPC_CREAT | 0644);
	value = shmat(shmid, NULL, 0);

	pthread_t tid[20];
	data d[20];
	for (int i = 0;i < 20;i++)
	{
		d[i].angka = value[i];
		if (i % 5 == 0) printf("\n");
		printf("%4d", value[i]);
		pthread_create(&tid[i],NULL, &jumn, (void*)&d[i]);
	}
	printf("\n");
	for(int i=0; i< 20; i++)
	{
		if (i % 5 == 0) printf("\n");
		printf("%d\t", d[i].hasil);
		pthread_join(tid[i],NULL);
	}
	printf("\n");
	shmdt(value);
	shmctl(shmid, IPC_RMID, NULL);
}
