#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#define MAX 500

int multiply(int x, int res[], int res_size);
void factorial(int n) 
{ 
	int res[MAX]; 

	res[0] = 1; 
	int res_size = 1; 

	for (int x=2; x<=n; x++) {
                res_size = multiply(x, res, res_size);
        }
	
	for (int i=res_size-1; i>=0; i--){
               printf("%d",res[i]);
        }  
}
		
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

void *fac(int angka)
{
        factorial(angka);
}

void main()
{
        key_t key = 1234;
        int *value;
        int arr[20];
        int angka, i, j;
        int shmid = shmget(key, sizeof(int), IPC_CREAT | 0644);
        value = shmat(shmid, NULL, 0);

        pthread_t tid[20];
        int idx;
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
        shmdt(value);
        shmctl(shmid, IPC_RMID, NULL);
}
