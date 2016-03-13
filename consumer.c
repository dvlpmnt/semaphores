#include <sys/sem.h> 
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <string.h>
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

struct message
{
	long mtype;
	char mtext[10000];
} msg;

int msglen = 10000;
int consumer_num;
int probability = 50;
int isDeadlock = 0;
struct msqid_ds qstat;
struct sembuf sop;

int where(int semid_q);
void release_lock(int semid, int sub);
void grab_lock(int semid, int sub);
void update_matrix(int semid_q, int k, int val);

int main(int argc, char *argv[])
{
	srand(getpid());
	if(strcmp(argv[2],"with") == 0)
	{
		isDeadlock = 1;
		probability = atoi(argv[3]);
	}
	
	consumer_num = atoi(argv[1]);
	int msgid, msgid_q1, msgid_q2, temp1;
	int curr, curr_sema, next, next_sema, status;

	key_t key_msg = 1025;
	key_t key_q1 = 2025;
	key_t key_q2 = 3025;
    msgid = msgget(key_msg,IPC_CREAT|0644);	
    msgid_q1 = msgget(key_q1,IPC_CREAT|0644);	
    msgid_q2 = msgget(key_q2,IPC_CREAT|0644);	
	
	int semid, c , d, first, second;
	key_t key_sem = 1523;

	semid = semget(key_sem, 9, IPC_CREAT | 0666);	// 5 subsemaphore

	memset(msg.mtext,'\0',msglen);
	sprintf(msg.mtext,"%d",getpid());
	msg.mtype = 200;
	while(msgsnd(msgid,&msg,strlen(msg.mtext),0) == -1);

	// printf("Consumer %d : sent %s\n",consumer_num, msg.mtext);
	sleep(1);
	while(1)
	{
		if(rand()%100 < probability) //read from one queue
		{
			if(rand()%2)
			{
				curr = msgid_q1;
				curr_sema = QUEUE0;
				first = 0;
			}
			else
			{
				curr = msgid_q2;
				curr_sema = QUEUE1;
				first = 1;
			}
			if(where(semid) == (1-first))
			{
				sleep(rand()%2 + 1);
				continue;
			}
			if(first == 0)
				grab_lock(semid, FULL0);
			else
				grab_lock(semid, FULL1);

			printf("Consumer%d , Trying to consume from Queue%d...\n",consumer_num, first );
			
			update_matrix(semid, first, 1);
	    	
	    	grab_lock(semid, curr_sema);

			update_matrix(semid, first, 2);

			memset(msg.mtext,'\0',msglen);

			while(msgrcv(curr, &msg, msglen, 500, 0) == -1);

	    	printf("Consumer%d , Consumed %s from Queue%d.\n\n",consumer_num,msg.mtext, first);	
	
			release_lock(semid, curr_sema);

			update_matrix(semid, first, 0);

			if(first == 0)
				release_lock(semid, EMPTY0);
			else
				release_lock(semid, EMPTY1);
			/*
				Increase Remove Count
			*/
			sop.sem_num = REMOVE;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);				
		}		
		else	//read from both queues
		{
			if(isDeadlock == 0)
			{
				temp1 = 1;
			}
			else
			{
				temp1 = rand()%2;
			}
			if( temp1 == 1)	// read from q1 first then from q2
			{
				first = 0;
				second = 1;
				curr = msgid_q1;
				curr_sema = QUEUE0;
				next = msgid_q2;
				next_sema = QUEUE1;
			}
			else			// read from q2 first then from q1
			{
				first = 1;
				second = 0;
				curr = msgid_q2;
				curr_sema = QUEUE1;
				next = msgid_q1;
				next_sema = QUEUE0;
			}

			if(where(semid) == (1-first))
			{
				sleep(rand()%2 + 1);
				continue;
			}

			if(first == 0)
				grab_lock(semid, FULL0);
			else
				grab_lock(semid, FULL1);

			printf("Consumer%d , Trying to consume from Queue%d and Queue%d...\n" , consumer_num, first, second);

			update_matrix(semid, first, 1);

	    	grab_lock(semid, curr_sema);

			update_matrix(semid, first, 2);

			update_matrix(semid, second, 1);

			// printf("acquired Queue%d\n" , first);

			// sleep(7);

			memset(msg.mtext,'\0',msglen);
			while( msgrcv(curr, &msg, msglen, 500, 0) == -1);

			printf("Consumer%d , Consumed %s from Queue%d.\n",consumer_num,msg.mtext,first);	

			/*
				Increase Remove Count
			*/

			sop.sem_num = REMOVE;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);

			// consume from 2nd queue

			if(second == 0)
				grab_lock(semid, FULL0);
			else
				grab_lock(semid, FULL1);

			update_matrix(semid, second, 1);

			grab_lock(semid, next_sema);

			update_matrix(semid, second, 2);

			memset(msg.mtext,'\0',msglen);

			while(msgrcv(next, &msg, msglen, 500, 0)  == -1);

			printf("Consumer%d , Consumed %s from Queue%d.\n\n",consumer_num,msg.mtext,second);	
			/*
				Increase Remove Count
			*/
			sop.sem_num = REMOVE;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			release_lock(semid, curr_sema);
			update_matrix(semid, first, 0);

			if(first == 0)
				release_lock(semid, EMPTY0);
			else
				release_lock(semid, EMPTY1);

			release_lock(semid, next_sema);

			if(second == 0)
				release_lock(semid, EMPTY0);
			else
				release_lock(semid, EMPTY1);

			update_matrix(semid, second, 1);
		}
		sleep(rand()%2 + 1);	
	}
	return 0;
}

void update_matrix(int semid_q, int k, int val)	// k = 0,1
{
	int i, j, temp;
	int A[2][10];

	grab_lock(semid_q, MATRIX);
	// printf("\n\n-------------------NEW----------------\n");	
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
			 // printf("%3d ",A[j][i]);
			fprintf(fp, "%d ", A[j][i]);
		}
		fprintf(fp, "\n");
		 // printf("\n");
	}
	fclose(fp);
	release_lock(semid_q, MATRIX);
}

void grab_lock(int semid, int sub)
{
	struct sembuf sop_var;
	sop_var.sem_num = sub;
	sop_var.sem_op = -1;
	sop_var.sem_flg = 0;
	semop(semid, &sop_var, 1);
}

void release_lock(int semid, int sub)
{
	struct sembuf sop_var;
	sop_var.sem_num = sub;
	sop_var.sem_op = 1;
	sop_var.sem_flg = 0;
	semop(semid, &sop_var, 1);
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
		for (i = 5; i < 10; i++)
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
