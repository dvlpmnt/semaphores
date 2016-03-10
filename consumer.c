#include <sys/sem.h> 
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
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

struct sembuf sop;

int msglen = 10000;
int consumer_num;
int probability;
struct msqid_ds qstat;

void update_matrix(int semid_q, int k, int val);

int main(int argc, char *argv[])
{
	consumer_num = atoi(argv[1]);
	int msgid, msgid_q1, msgid_q2;
	int curr, curr_sema, next, next_sema, status;
	key_t key_sem = 1234;
	key_t key_msg = 1025;
	key_t key_q1 = 2025;
	key_t key_q2 = 3025;
    msgid = msgget(key_msg,IPC_CREAT|0644);	
    msgid_q1 = msgget(key_q1,IPC_CREAT|0644);	
    msgid_q2 = msgget(key_q2,IPC_CREAT|0644);	
	
	key_t key_sem_file = 1523;
	key_t key_sem_q1 = 2523;
	key_t key_sem_q2 = 3523;
	int semid_file, semid_q1, semid_q2;
	semid_file = semget(key_sem_file, 1, IPC_CREAT | 0666);	// 1 subsemaphore
	semid_q1 = semget(key_sem_q1, 1, IPC_CREAT | 0666);	// 1 subsemaphores
	semid_q2 = semget(key_sem_q2, 1, IPC_CREAT | 0666);	// 1 subsemaphores

	memset(msg.mtext,'\0',msglen);
	sprintf(msg.mtext,"%d",getpid());
	msg.mtype = 200;
	while(msgsnd(msgid,&msg,strlen(msg.mtext),0) == -1);
	while(1)
	{
		if(rand()%100 < probability) //read from one queue
		{
			if(rand()%2)
			{
				curr = msgid_q1;
				curr_sema = semid_q1;
			}
			else
			{
				curr = msgid_q2;
				curr_sema = semid_q2;
			}
			msgctl(curr,IPC_STAT,&qstat);
			if(qstat.msg_qnum > 0)
			{

				update_matrix(semid_file, curr != msgid_q1, 1);

				//check lock and read
				sop.sem_num = 0;
				sop.sem_op = -1;
				sop.sem_flg = 0;
				semop(curr_sema, &sop, 1);
		
				update_matrix(semid_file, curr != msgid_q1, 2);

				status = msgrcv(curr, &msg, msglen, 500, 0);
	    			
	    		sop.sem_num = 0;
				sop.sem_op = 1;
				sop.sem_flg = 0;
				semop(curr_sema, &sop, 1);

				update_matrix(semid_file, curr != msgid_q1, 0);
			
			}
				
		}
		else	//read from both queues
		{
			if(rand()%2)	// read from q1 first then from q2
			{
				curr = msgid_q1;
				curr_sema = semid_q1;
				next = msgid_q2;
				next_sema = semid_q2;
			}
			else			// read from q2 first then from q1
			{
				curr = msgid_q2;
				curr_sema = semid_q2;
				next = msgid_q1;
				next_sema = semid_q1;
			}
			msgctl(curr,IPC_STAT,&qstat);
			if(qstat.msg_qnum > 0)
			{
				update_matrix(semid_file, curr != msgid_q1, 1);

				//check lock and read
				sop.sem_num = 0;
				sop.sem_op = -1;
				sop.sem_flg = 0;
				semop(curr_sema, &sop, 1);

				update_matrix(semid_file, curr != msgid_q1, 2);

				status = msgrcv(curr, &msg, msglen, 500, 0);

				sop.sem_num = 0;
				sop.sem_op = 1;
				sop.sem_flg = 0;
				semop(curr_sema, &sop, 1);

				update_matrix(semid_file, curr != msgid_q1, 1);

			}
			msgctl(next,IPC_STAT,&qstat);
			if(qstat.msg_qnum > 0)
			{

				update_matrix(semid_file, next != msgid_q1, 1);

				//check lock and read
				sop.sem_num = 0;
				sop.sem_op = -1;
				sop.sem_flg = 0;
				semop(next_sema, &sop, 1);
		
				update_matrix(semid_file, next != msgid_q1, 2);

				status = msgrcv(next, &msg, msglen, 500, 0);
	    	
	    		sop.sem_num = 0;
				sop.sem_op = 1;
				sop.sem_flg = 0;
				semop(next_sema, &sop, 1);

				update_matrix(semid_file, next != msgid_q1, 0);
			}
		}
		sleep(rand()%5);	
	}
	// getchar();
	return 0;
}

void update_matrix(int semid_q, int k, int val)	// k = 0,1
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
	A[k][5 + consumer_num - 1] = val;	
	fp = fopen("matrix.txt", "w");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fprintf(fp, "%d ", A[j][i]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);

	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(semid_q, &sop, 1);
}