#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define MAX 500
		
int multiply(int x, int res[], int res_size) 
{ 
	int carry = 0;
	for (int i=0; i<res_size; i++) 
	{ 
		int prod = res[i] * x + carry; 
		res[i] = prod % 10; 
		carry = prod/10;	 
	} 

	while (carry) 
	{
		res[res_size] = carry%10; 
		carry = carry/10; 
		res_size++; 
	} 
	return res_size; 
} 

void factorial(int n, char* c) 
{ 
	int res[MAX];

	res[0] = 1; 
	int res_size = 1; 

	for (int x=2; x<=n; x++) {
		res_size = multiply(x, res, res_size);
	}
	
	for (int i=res_size-1; i>=0; i--){
		char cc[2];
		sprintf(cc, "%d", res[i]);
		strcat(c, cc);
	}
}

typedef struct data {
	int angka;
	char hasil[1500];
}data;

void *fac(void* arg)
{
	data* d = (data*) arg;
	memset(d->hasil, 0, sizeof(d->hasil));
	factorial(d->angka, d->hasil);
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
		pthread_create(&tid[i],NULL, &fac, (void*)&d[i]);
	}
	printf("\n");
	for(int i=0; i< 20; i++)
	{
		if (i % 5 == 0) printf("\n");
		printf("%20s", d[i].hasil);
		pthread_join(tid[i],NULL);
	}
	printf("\n");
	shmdt(value);
	shmctl(shmid, IPC_RMID, NULL);
}
