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
	key_t key_sem = 1234;
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
	// int nsems = 20, semid;
	// semid = semget(key_sem, nsems, IPC_CREAT | 0666);
	// if (semid < 0)
	// {
	// 	perror("Semaphore creation failed\n");
	// }
	getchar();
	return 0;
}