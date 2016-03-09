#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>


int main()
{
	printf("Prod\n");
	srand(time(NULL));
	int pid = (int)getpid();
	printf("PID %d\n", pid);
	key_t key = 1025;
	int nsems = 20;
	semid = semget(key, nsems, IPC_CREAT | 0666);
	if (semid < 0)
	{
		perror("Semaphore creation failed\n");
	}
	getchar();
	return 0;
}