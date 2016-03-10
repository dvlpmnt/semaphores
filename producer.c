#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

struct message
{
	long mtype;
	char mtext[10000];
} msg;

int msglen = 10000;
int producer_num;
int main(int argc, char *argv[])
{
	producer_num = atoi(argv[1]);
	srand(time(NULL));
	int pid = getpid(), msgid, msgid_q1, msgid_q2;
	printf("PID %d\n", pid);
	key_t key_msg = 1025;
	key_t key_q1 = 2025;
	key_t key_q2 = 3025;
	struct msqid_ds qstat;
    msgid = msgget(key_msg,IPC_CREAT|0644);	
    msgid_q1 = msgget(key_q1,IPC_CREAT|0644);	
    msgid_q2 = msgget(key_q2,IPC_CREAT|0644);	
	memset(msg.mtext,'\0',msglen);
	sprintf(msg.mtext,"%d",getpid());
	msg.mtype = 200;
	while(msgsnd(msgid,&msg,strlen(msg.mtext),0) == -1);
	while(1)
	{
		if(rand()%2)
		{
			msgctl(msgid_q1,IPC_STAT,&qstat);
			if(qstat.msg_qnum < 10)
			{
				memset(msg.mtext,'\0',msglen);
				sprintf(msg.mtext,"%d",rand()%50 + 1);
				msg.mtype = 500;
				printf("Producer%d , Queue1 Trying to insert %s...\n", producer_num, msg.mtext);
				// check lock
				msgsnd(msgid_q1,&msg,strlen(msg.mtext),0);
				printf("Producer%d, Successfully inserted.\n", producer_num);
				printf("%d messages\n",(int)qstat.msg_qnum);
			}

		}
		else
		{
			msgctl(msgid_q2,IPC_STAT,&qstat);
			if(qstat.msg_qnum < 10)
			{
				memset(msg.mtext,'\0',msglen);
				sprintf(msg.mtext,"%d",rand()%50 + 1);
				msg.mtype = 500;
				printf("%d , Queue2 Trying to insert %s...\n",getpid(),msg.mtext);
				//check lock
				msgsnd(msgid_q2,&msg,strlen(msg.mtext),0);
				printf("%d , Successfully inserted.\n", getpid());
				printf("%d messages\n",(int)qstat.msg_qnum);
			}

		}
		sleep(rand()%10 + 1);
	}
	getchar();
	return 0;
}

/// to be finished

void update_graph(int semid_q, int k, int val)	// k = 0,1
{
	int i, j, temp;
	int A[2][10];

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(semid_q, &sop, 1);
	
	FILE *fp = fopen("matrix.txt", "r");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fscanf(fp, "%d", &A[j][i]);
		}
	}
	fclose(fp);
	int ret = semctl(semid_q, 1, GETVAL, 0);
	A[k][producer_num-1] = val;	
	FILE *fp = fopen("matrix.txt", "w");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fprintf(fp, "%d ", &[j][i]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);

	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(semid_q, &sop, 1);
}