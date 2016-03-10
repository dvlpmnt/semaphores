#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int graph[2][10] = {0};

struct message
{
	long mtype;
	char mtext[10000];
} msg;

int msglen = 10000;

void read_graph( );
int CheckCycle();
int main()
{
	int i, j, msgid, msgid_q1, msgid_q2, status;
	char pRun[100] = {"./p"};
	char cRun[100] = {"./c"};
	pid_t procs[10], p ,p1, p2;
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
		read_graph();
	}
	while(CheckCycle()!=1);
	sleep(20);
	for(i=0;i<10;i++)
		kill(procs[i], SIGINT);
	return 0;
}

void read_graph()
{
	int i, j, temp;
	FILE *fp = fopen("matrix.txt", "r");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fscanf(fp, "%d", &temp);
			if (temp == 1)
				graph[j][i] = 1;
			if (temp == 2)
				graph[i][j] = 1;
		}
	}
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			printf("%4d", graph[j][i]);
		}
		printf("\n\n");
	}
	fclose(fp);
}

int CheckCycle()
{
	/**
		pi -> qk -> pj -> ql ->pi
	*/
	int i, j, k, l, m, n;
	for(i=0;i<10;i++)
		{
			
		}
	return 1;
}