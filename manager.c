#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int graph[12][12] = {0};

struct message
{
	long mtype;
	char mtext[10000];
} msg;

struct sembuf
{
	ushort sem_num;
	short	 sem_op;
	short sem_flg;
} sop;

int msglen = 10000;

void read_graph(int semid_file);
void DFSCheckCycle ();
void Visit(int p[], int u,int color[]);
void PrintCycle(int p[], int v, int u);

int main()
{
	int i, j, msgid, msgid_q1, msgid_q2, status;
	char pRun[100] = {"./p"};
	char cRun[100] = {"./c"};
	pid_t procs[10], p ,p1, p2;
	/**
		Create 3 Empty Message Queues
	*/
	/*
		Queue to get PIDs of producers/consumers
	*/
    key_t key = 1025;
    msgid = msgget(key,IPC_CREAT|0644);	
	msgctl(msgid_q1, IPC_RMID, NULL);	// remove already existing queue
    msgid = msgget(key,IPC_CREAT|0644);	// recreate a queue which is empty
	/*
		Queue1 for producers
	*/
    key_t key_q1 = 2025;
    msgid_q1 = msgget(key_q1,IPC_CREAT|0644);
	msgctl(msgid_q1, IPC_RMID, NULL);	// remove already existing queue
    msgid_q1 = msgget(key_q1,IPC_CREAT|0644);
	/*
		Queue2 for producers
	*/
    key_t key_q2 = 3025;
    msgid_q2 = msgget(key_q2,IPC_CREAT|0644);
	msgctl(msgid_q2, IPC_RMID, NULL);	// remove already existing queue
    msgid_q2 = msgget(key_q2,IPC_CREAT|0644);

	/**
		Create 3 Empty Semaphores
	*/
	key_t key_sem_file = 1523;
	key_t key_sem_q1 = 2523;
	key_t key_sem_q2 = 3523;
	int semid_file, semid_q1, semid_q2;
	semid_file = semget(key_sem_file, 1, IPC_CREAT | 0666);	// 1 subsemaphore
	semid_q1 = semget(key_sem_q1, 2, IPC_CREAT | 0666);	// 2 subsemaphores
	semid_q2 = semget(key_sem_q2, 2, IPC_CREAT | 0666);	// 2 subsemaphores
	semctl(semid_file, 0, IPC_RMID, 0);
	semctl(semid_q1, 0, IPC_RMID, 0);
	semctl(semid_q2, 0, IPC_RMID, 0);
	semid_file = semget(key_sem_file, 1, IPC_CREAT | 0666);	// 1 subsemaphore
	semid_q1 = semget(key_sem_q1, 2, IPC_CREAT | 0666);	// 2 subsemaphores
	semid_q2 = semget(key_sem_q2, 2, IPC_CREAT | 0666);	// 2 subsemaphores
	semctl(semid_file, 0, SETVAL, 1);	// Mutex for file
	semctl(semid_q2, 0, SETVAL, 1);		// Mutex for Locking Queue1
	semctl(semid_q2, 1, SETVAL, 0);		// Count for Queue1
	semctl(semid_q1, 0, SETVAL, 1);		// Mutex for Locking Queue2
	semctl(semid_q1, 1, SETVAL, 0);		// Count for Queue2
	if (semid < 0)
	{
		perror("Semaphore creation failed\n");
	}

	FILE *fp = fopen("matrix.txt", "w+");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fprintf(fp, "%d ", 0);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	char temp[50];
	for (i = 0; i < 5; i++)
	{
		p1 = fork();
		if (p1 == 0)
		{
			memset(temp,'\0',50);
			sprintf(temp, "./p %d",i + 1);
			execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", temp , (void*)NULL);
		}
		else
			{
				p2 = fork();
				if (p2 == 0)
				{
				memset(temp,'\0',50);
				sprintf(temp, "./c %d",i + 1);
				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", temp, (void*)NULL);
				}
			}
	}
    for(i=0;i<10;i++)
    {
    	memset(msg.mtext,'\0',msglen);
    	status = msgrcv(msgid, &msg, msglen, 200, 0);
    	procs[i] = atoi(msg.mtext);
    	printf("\t%d\n",(int)procs[i] );
    }
	do
	{
		sleep(2);
		read_graph(semid_file);
	}
	while(DFSCheckCycle() != 1);
	sleep(20);
	for(i=0;i<10;i++)
		kill(procs[i], SIGINT);
	return 0;
}

void read_graph(int semid_file)
{
	int i, j, temp;

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(semid_file, &sop, 1);

	FILE *fp = fopen("matrix.txt", "r");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fscanf(fp, "%d", &temp);
			if (temp == 1)
				graph[i][j+10] = 1;
			if (temp == 2)
				graph[j+10][i] = 1;
		}
	}
	fclose(fp);

	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(semid_file, &sop, 1);

	for (j = 0; j < 12; j++)
	{
		for (i = 0; i < 12; i++)
		{
			printf("%4d", graph[j][i]);
		}
		printf("\n\n");
	}
}

void DFSCheckCycle ()
{
    int p[12],color[12];
    int i,j;
    for(j=0;j<12;j++)
    	{
    		p[j]=-2;
    		color[j]=0;
    	}
   for(i=0;i<12;i++)
   {
        if (color[i] == 0) {
            p[i] = -1; // meaning it is a root of a DFS-tree of the DFS forest
            if(Visit(p,i,color) == 1)
            	return 1;
        }
    }
    return 0;
}

int Visit(int p[], int u,int color[])
{
    int cycle =0, v ;
    color[u] = 1;
    for(v=0;v<12;v++)
    	if(graph[u][v] == 1)
		{
			if (color[v] == 0) {
            	p[v] = u;
            	Visit(p,v,color);
        	}
        	else if (color[v] == 1) 
			{
            	cycle = 1;
            	break;
        	}
    	}
    color[u] = 2; // once DFS for this vertex ends, assign its color to black

    if(cycle==1)	//there is a cycle
        {
        	PrintCycle(p,v,u);
        	return 1;
        }
    return 0;
}

void PrintCycle(int p[], int v, int u) 
{
    do {
        printf(" %2d ->", u);
        u = p[u];
        if(u == -1)
        	{
        		break;
        	}
    } while (u != v);
}