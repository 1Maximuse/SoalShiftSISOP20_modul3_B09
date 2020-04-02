#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>

void jumlah(int n)
{
	int sum=0;
	for(n;n>0;n--)
	{
		sum+=n;
	}
	printf("%d",sum);
        // return sum;
}
void *jum(int angka)
{
        jumlah(angka);
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
                pthread_create(&tid[idx],NULL, &jum, (void*)angka);
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
