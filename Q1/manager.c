#include <sys/sem.h> 
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define MATRIX 0
#define QUEUE0 1
#define QUEUE1 2
#define INSERT 3
#define REMOVE 4
#define EMPTY0 5
#define FULL0  6
#define EMPTY1 7
#define FULL1  8

int graph[2][10] = {0};

struct message
{
	long mtype;
	char mtext[10000];
} msg;

struct sembuf sop;

int msglen = 10000, isCyclic ;

void read_graph(int semid_file);  // reads graph and check whether there exists a cycle or not

int main(int argc, char *argv[])
{
	if( argc == 1 || !(argc==3 && strcmp(argv[1],"with") == 0) && !( strcmp(argv[1],"without") == 0 && (argc==3 || argc==2)) )
	{
		printf("\nincorrect arguments!\n\ncommand : ./m <with/without> <probabiliy>\n\n");
		return 1;
	}

	FILE *fp;
	int i, j, msgid, msgid_q1, msgid_q2, status, probability = 50, producer_count = 5, consumer_count = 5;
	pid_t procs[10], p ,p1, p2;

    key_t key = 1025;
    key_t key_q1 = 2025;
    key_t key_q2 = 3025;    
	key_t key_sem = 1523;

	int semid;

	semid = semget(key_sem, 9, IPC_CREAT | 0666);	// 5 subsemaphore for file lock

	if( strcmp(argv[1],"with") == 0 )
	{
		// fp = fopen("result.txt","w+");
		// fprintf(fp, "Probability\tInsertCount\tRemoveCount\n");
		// fclose(fp);
	}
	if(argc == 3)
	{
		probability = atoi(argv[2]);
	}
	// do
	// {
		// printf("\n\n\t\tNEW\n\n\n" );
		isCyclic = 0;
	/*
		Queue to get PIDs of producers/consumers
	*/
		msgid = msgget(key,IPC_CREAT|0644);	
		msgctl(msgid_q1, IPC_RMID, NULL);	// remove already existing queue
    	msgid = msgget(key,IPC_CREAT|0644);	// recreate a queue which is empty
	/*
		Queue1
	*/
		
		msgid_q1 = msgget(key_q1,IPC_CREAT|0644);
		msgctl(msgid_q1, IPC_RMID, NULL);	// remove already existing queue
    	msgid_q1 = msgget(key_q1,IPC_CREAT|0644);
	/*
		Queue2
	*/
		
		msgid_q2 = msgget(key_q2,IPC_CREAT|0644);
		msgctl(msgid_q2, IPC_RMID, NULL);	// remove already existing queue
    	msgid_q2 = msgget(key_q2,IPC_CREAT|0644);

		semctl(semid, MATRIX, SETVAL, 1);	// Mutex for file
		
		semctl(semid, QUEUE0, SETVAL, 1);		// Mutex for Locking Queue1
		
		semctl(semid, QUEUE1, SETVAL, 1);		// Mutex for Locking Queue2
		
		semctl(semid, INSERT, SETVAL, 0);		// Insert Count
		
		semctl(semid, REMOVE, SETVAL, 0);		// Remove Count

		semctl(semid, EMPTY0, SETVAL, 10);		// Insert Count
		
		semctl(semid, FULL0, SETVAL, 0);		// Remove Count

		semctl(semid, EMPTY1, SETVAL, 10);		// Insert Count
		
		semctl(semid, FULL1, SETVAL, 0);		// Remove Count

		fp = fopen("matrix.txt", "w+");
		for (j = 0; j < 2; j++)
		{
			for (i = 0; i < 10; i++)
			{
				fprintf(fp, "%d ", 0);
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
		char temp[100];
		char prob[50];
		for (i = 0; i < producer_count; i++)
		{
			p1 = fork();
			if (p1 == 0 )
			{
				memset(temp,'\0',50);
				sprintf(temp, "./p %d ",i + 1);
				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", temp , (void*)NULL);
			}
		}
		for (i = 0; i < consumer_count; i++)
		{
			p2 = fork();
			if (p2 == 0)
			{
				sprintf(prob," %d",probability);
				memset(temp,'\0',50);
				sprintf(temp, "./c %d ",i + 1);
				strcat(temp, argv[1]);
				strcat(temp, prob);
				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", temp, (void*)NULL);
			}
		}

	    for(i=0;i < producer_count + consumer_count;i++)
	    {
	    	memset(msg.mtext,'\0',msglen);
	    	status = msgrcv(msgid, &msg, msglen, 200, 0);
	    	procs[i] = atoi(msg.mtext);
	    	printf("\t%s%d PID : %d\n",i<5 ? "Producer" : "Consumer", i<5 ? i+1 : i-4,(int)procs[i] );
	    }

		while(isCyclic == 0)
		{
			sleep(2);
			// printf("E0 %d,E1 %d, F0 %d, F1 %d\n", semctl(semid, EMPTY0, GETVAL, 0), semctl(semid, EMPTY1, GETVAL, 0), semctl(semid, FULL0, GETVAL, 0), semctl(semid, FULL1, GETVAL, 0));
			read_graph(semid); // reads graph and check whether there exists a cycle or not
		}
		if( strcmp(argv[1],"with") == 0 )
		{
			fp = fopen("result.txt","a");
			fprintf(fp, "\t.%d\t\t%6d\t\t%6d\n",probability, semctl(semid, INSERT, GETVAL, 0), semctl(semid, REMOVE, GETVAL, 0) );
			fclose(fp);
		}

		// sleep(20);	//so that user can observe the deadlock
		
		for(i=0;i<10;i++)
			kill(procs[i], SIGINT);

		// probability += 10;
	 // }
	 // while(strcmp(argv[1],"with") == 0 && probability < 100);
	return 0;
}

void read_graph(int semid)  // reads graph and check whether there exists a cycle or not
{
	int i, j, temp;

	sop.sem_num = MATRIX;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(semid, &sop, 1);
	// printf("locked matrix.txt\n");
	FILE *fp = fopen("matrix.txt", "r");
	printf("\n--------------------MATRIX--------------------\n\n");
	// printf("value read\n");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fscanf(fp, "%d", &temp);
			// printf("%4d , ",temp );
			graph[j][i] = temp;
		}
		// printf("\n");
	}
	fclose(fp);

	sop.sem_num = MATRIX;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(semid, &sop, 1);
	
	int a = -1, b = -1;
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			printf("%4d", graph[j][i]);
		}
		printf("\n\n");
	}
	// printf("released matrix.txt\n");
	printf("----------------------------------------------\n");

	for (j = 0; j < 2 && b == -1; j++)
	{
		for (i = 0; i < 10; i++)
		{
			if(a != -1)
			{
				if(graph[0][i] == 1 && graph[1][i] == 2)
					{
						b = i;
						break;
					}
			}
			else if( graph[0][i] == 2 && graph[1][i] == 1)
			{
				a = i;
			}
		}
	}
	if( a == -1 || b == -1)
		return ;
	isCyclic = 1;
	printf("Consumer%d -> Queue1 -> Consumer%d -> Queue2 -> Consumer%d\n\n", a-4,b-4,a-4);
}


