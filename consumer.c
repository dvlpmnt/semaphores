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
int consumer_num;
int probability;

int main(int argc, char *argv[])
{
	consumer_num = atoi(argv[1]);
	int msgid, msgid_q1, msgid_q2, curr, next, status;
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
	if(rand()%100 < probability) //read from one queue
	{
		if(rand()%2)
		{
			curr = msgid_q1;
		}
		else
		{
			curr = msgid_q2;
		}
		msgctl(curr,IPC_STAT,&qstat);
		if(qstat.msg_qnum > 0)
		{
			//check lock and read
			status = msgrcv(curr, &msg, msglen, 500, 0);
    	
		}
			
	}
	else	//read from both queues
	{
		if(rand()%2)	// read from q1 first then from q2
		{
			curr = msgid_q1;
			next = msgid_q2;
		}
		else			// read from q2 first then from q1
		{
			curr = msgid_q2;
			next = msgid_q1;
		}
		msgctl(curr,IPC_STAT,&qstat);
		if(qstat.msg_qnum > 0)
		{
			//check lock and read
			status = msgrcv(curr, &msg, msglen, 500, 0);
    	
		}
		msgctl(next,IPC_STAT,&qstat);
		if(qstat.msg_qnum > 0)
		{
			//check lock and read
			status = msgrcv(next, &msg, msglen, 500, 0);
    	
		}
	}
	// getchar();
	return 0;
}