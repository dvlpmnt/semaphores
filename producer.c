#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define MATRIX 0
#define QUEUE0 1
#define QUEUE1 2
#define INSERT 3
#define REMOVE 4
#define EMPTY0 5
#define FULL0  6
#define EMPTY1 7
#define FULL1  8

struct message
{
	long mtype;
	char mtext[10000];
} msg;

struct sembuf sop;

int msglen = 10000;
int producer_num;
struct msqid_ds qstat;

void release_lock(int semid, int sub);
int grab_lock_ipc(int semid, int sub);
void grab_lock(int semid, int sub);
void update_matrix(int semid_q, int k, int val);

int main(int argc, char *argv[])
{
	srand(getpid());
	producer_num = atoi(argv[1]);
	int pid = getpid(), msgid, msgid_q1, msgid_q2,fq0 = 0,fq1 = 0;
	key_t key_msg = 1025;
	key_t key_q1 = 2025;
	key_t key_q2 = 3025;

    msgid = msgget(key_msg,IPC_CREAT|0644);	
    msgid_q1 = msgget(key_q1,IPC_CREAT|0644);	
    msgid_q2 = msgget(key_q2,IPC_CREAT|0644);	
	
	int semid, c, number;
	key_t key_sem = 1523;
	semid = semget(key_sem, 9, IPC_CREAT | 0666);	// 5 subsemaphore

	memset(msg.mtext,'\0',msglen);
	sprintf(msg.mtext,"%d",getpid());
	msg.mtype = 200;
	while(msgsnd(msgid,&msg,strlen(msg.mtext),0) == -1);

	while(1)
	{
		if( rand()%2 )
		{	
			memset(msg.mtext,'\0',msglen);
			sprintf(msg.mtext,"%d",rand()%50 + 1);
			msg.mtype = 500;

			if(where(semid) == 1)
			{
				sleep(rand()%3);
				continue ;
			}

			if(grab_lock_ipc(semid, EMPTY0) == -1)
			{
				sleep(rand()%3);
				continue;
			}

			printf("Producer%d , Trying to Insert %s in Queue0...\n",producer_num, msg.mtext);

			update_matrix(semid, 0, 1);

    		grab_lock(semid, QUEUE0);

			update_matrix(semid, 0, 2);
			
			while( msgsnd(msgid_q1,&msg,strlen(msg.mtext),0) == -1);

			printf("Producer%d , Successfully inserted.\n\n", producer_num);

			release_lock(semid, QUEUE0);

			update_matrix(semid, 0, 0);

			release_lock(semid, FULL0);
			/*
				Increase Insertion Count
			*/
			sop.sem_num = INSERT;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
		}
		else
		{
			memset(msg.mtext,'\0',msglen);
			sprintf(msg.mtext,"%d",rand()%50 + 1);
			msg.mtype = 500;

			if(where(semid) == 0)
			{
				sleep(rand()%3);
				continue ;
			}

			if(grab_lock_ipc(semid, EMPTY1) == -1)
			{
				sleep(rand()%3);
				continue;
			}
			printf("Producer%d , Trying to Insert %s in Queue1...\n",producer_num, msg.mtext);

			update_matrix(semid, 1, 1);

    		grab_lock(semid, QUEUE1);

			update_matrix(semid, 1, 2);

			while(msgsnd(msgid_q2,&msg,strlen(msg.mtext),0) == -1);

			release_lock(semid ,QUEUE1);

			update_matrix(semid, 1, 0);

			release_lock(semid, FULL1);	

			printf("Producer%d , Successfully inserted.\n\n", producer_num);
				/*
					Increase Insertion Count
				*/
			sop.sem_num = INSERT;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
		}
		sleep(rand()%3);
	}
	return 0;
}

void update_matrix(int semid_q, int k, int val)	// k = 0,1
{
	int i, j, temp;
	int A[2][10];
	grab_lock(semid_q, MATRIX);
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
	release_lock(semid_q, MATRIX);
}

int where(int semid_q)
{
	int i, j, temp,l=0,m=0;
	int A[2][10];

	grab_lock(semid_q, MATRIX);

	FILE *fp = fopen("matrix.txt", "r");
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 10; i++)
		{
			fscanf(fp, "%d", &A[j][i]);
		}
	}
	fclose(fp);
	release_lock(semid_q, MATRIX);
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 5; i++)
		{
			if(j==0)
				l += A[j][i];
			else
				m += A[j][i];
		}
	}
	if(l == (5 - 1))
		return 1;
	if(m == (5 -1))
		return 0;
	return -1;
}

void grab_lock(int semid, int sub)
{
	struct sembuf sop_var;
	sop_var.sem_num = sub;
	sop_var.sem_op = -1;
	sop_var.sem_flg = 0;
	semop(semid, &sop_var, 1);
}

int grab_lock_ipc(int semid, int sub)
{
	struct sembuf sop_var;
	sop_var.sem_num = sub;
	sop_var.sem_op = -1;
	sop_var.sem_flg = IPC_NOWAIT;
	return semop(semid, &sop_var, 1);
}

void release_lock(int semid, int sub)
{
	struct sembuf sop_var;
	sop_var.sem_num = sub;
	sop_var.sem_op = 1;
	sop_var.sem_flg = 0;
	semop(semid, &sop_var, 1);
}