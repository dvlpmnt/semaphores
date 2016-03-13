#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int trainId;
char trainType;

void updateFile(int val, int num_q);

struct message
{
    long mtype;
    char mtext[10000];
} msg;

int main(int argc, char *argv[]) {
	key_t key_sem_sync = 1025;
	key_t key_dir = 1045;
	int semid_sem_sync, semid_dir;
	struct sembuf wait11, signal11, wait12, signal12, wait13, signal13, wait14, signal14, wait2, signal2;
	if ((semid_sem_sync = semget(key_sem_sync, 2, 0)) == -1)
	{
		perror("semget: semget failed");
		exit(1);
	}
	if ((semid_dir = semget(key_dir, 4, 0)) == -1)
	{
		perror("semget: semget failed");
		exit(1);
	}

		key_t keymsg = 1024;
	int msgid;
	msgid = msgget(keymsg, IPC_CREAT | 0644);
	memset(msg.mtext, '\0', sizeof(msg.mtext));
	sprintf(msg.mtext,"%d",getpid());
	msg.mtype = 200;
    while( msgsnd(msgid, &msg, strlen(msg.mtext), 0) == -1);

	//0->N 1->E 2->S 3->W
	//0->junction 1->file
	wait11.sem_num = 0;
	wait11.sem_op = -1;
	wait11.sem_flg = 0;

	signal11.sem_num = 0;
	signal11.sem_op = 1;
	signal11.sem_flg = 0;

	wait12.sem_num = 0;
	wait12.sem_op = -1;
	wait12.sem_flg = 0;

	signal12.sem_num = 0;
	signal12.sem_op = 1;
	signal12.sem_flg = 0;

	wait13.sem_num = 0;
	wait13.sem_op = -1;
	wait13.sem_flg = 0;

	signal13.sem_num = 0;
	signal13.sem_op = 1;
	signal13.sem_flg = 0;

	wait14.sem_num = 0;
	wait14.sem_op = -1;
	wait14.sem_flg = 0;

	signal14.sem_num = 0;
	signal14.sem_op = 1;
	signal14.sem_flg = 0;

	wait2.sem_num = 0;
	wait2.sem_op = -1;
	wait2.sem_flg = 0;

	signal2.sem_num = 0;
	signal2.sem_op = 1;
	signal2.sem_flg = 0;

	pid_t pid = getpid();

	char type;
	trainId = atoi(argv[2]);
	// printf("Train ID: %d\n\n", trainId);
	if (argv[1][0] == 'N') {
		printf("Train <%d>: North Train started\n", pid);

		printf("Train <%d>: Requests for North-lock\n", pid);
		updateFile(1, 0);

		wait11.sem_num = 0;
		semop(semid_dir, &wait11, 1);
		printf("Train <%d>: Acquires North-lock\n", pid);
		updateFile(2, 0);

		printf("Train <%d>: Requests for West-lock\n", pid);
		updateFile(1, 3);
		wait14.sem_num = 3;
		semop(semid_dir, &wait14, 1);
		printf("Train <%d>: Acquires West-lock\n", pid);
		updateFile(2, 3);

		wait2.sem_num = 0;
		printf("Train <%d>: Requests Junction-Lock\n", pid);
		semop(semid_sem_sync, &wait2, 1);
		printf("Train <%d>: Acquires Junction-Lock; Passing Junction;\n", pid);
		sleep(2);
		signal2.sem_num = 0;
		semop(semid_sem_sync, &signal2, 1);
		printf("Train <%d>: Releases Junction-Lock\n", pid);

		signal11.sem_num = 0;
		semop(semid_dir, &signal11, 1);
		printf("Train <%d>: Releases North-lock\n", pid);
		updateFile(0, 0);

		signal14.sem_num = 3;
		semop(semid_dir, &signal14, 1);
		printf("Train <%d>: Releases West-lock\n", pid);
		updateFile(0, 3);

	}
	else if (argv[1][0] == 'E') {
		printf("Train <%d>: East Train started\n", pid);

		printf("Train <%d>: Requests East-Lock\n", pid);
		updateFile(1, 1);
		wait12.sem_num = 1;
		semop(semid_dir, &wait12, 1);
		printf("Train <%d>: Acquires East-Lock\n", pid);
		updateFile(2, 1);

		printf("Train <%d>: Requests North-Lock\n", pid);
		updateFile(1, 0);
		wait11.sem_num = 0;
		semop(semid_dir, &wait11, 1);
		printf("Train <%d>: Acquires North-Lock\n", pid);
		updateFile(2, 0);

		wait2.sem_num = 0;
		printf("Train <%d>: Requests Junction-Lock\n", pid);
		semop(semid_sem_sync, &wait2, 1);
		printf("Train <%d>: Acquires Junction-Lock; Passing Junction;\n", pid);
		sleep(2);
		signal2.sem_num = 0;
		semop(semid_sem_sync, &signal2, 1);
		printf("Train <%d>: Releases Junction-Lock\n", pid);

		signal12.sem_num = 1;
		semop(semid_dir, &signal12, 1);
		printf("Train <%d>: Releases East-Lock\n", pid);
		updateFile(0, 1);

		signal11.sem_num = 0;
		semop(semid_dir, &signal11, 1);
		printf("Train <%d>: Releases North-Lock\n", pid);
		updateFile(0, 0);
	}
	else if (argv[1][0] == 'S') {
		printf("Train <%d>: South Train started\n", pid);

		printf("Train <%d>: Requests for South-Lock\n", pid);
		updateFile(1, 2);

		wait13.sem_num = 2;
		semop(semid_dir, &wait13, 1);
		printf("Train <%d>: Acquires South-Lock\n", pid);
		updateFile(2, 2);

		printf("Train <%d>: Requests for East-Lock\n", pid);
		updateFile(1, 1);
		wait12.sem_num = 1;
		semop(semid_dir, &wait12, 1);
		printf("Train <%d>: Acquires East-Lock\n", pid);
		updateFile(2, 1);

		wait2.sem_num = 0;
		printf("Train <%d>: Requests Junction-Lock\n", pid);
		semop(semid_sem_sync, &wait2, 1);
		printf("Train <%d>: Acquires Junction-Lock; Passing Junction;\n", pid);
		sleep(2);
		signal2.sem_num = 0;
		semop(semid_sem_sync, &signal2, 1);
		printf("Train <%d>: Releases Junction-Lock\n", pid);

		signal13.sem_num = 2;
		semop(semid_dir, &signal13, 1);
		printf("Train <%d>: Releases South-Lock\n", pid);
		updateFile(0, 2);

		signal12.sem_num = 1;
		semop(semid_dir, &signal12, 1);
		printf("Train <%d>: Releases East-Lock\n", pid);
		updateFile(0, 1);
	}
	else if (argv[1][0] == 'W') {
		printf("Train <%d>: West Train started\n", pid);

		printf("Train <%d>: Requests for West-Lock\n", pid);
		updateFile(1, 3);
		wait14.sem_num = 3;
		semop(semid_dir, &wait14, 1);
		printf("Train <%d>: Acquires West-Lock\n", pid);
		updateFile(2, 3);

		printf("Train <%d>: Requests for South-Lock\n", pid);
		updateFile(1, 2);
		wait13.sem_num = 2;
		semop(semid_dir, &wait13, 1);
		printf("Train <%d>: Acquires South-Lock\n", pid);
		updateFile(2, 2);

		wait2.sem_num = 0;
		printf("Train <%d>: Requests Junction-Lock\n", pid);
		semop(semid_sem_sync, &wait2, 1);
		printf("Train <%d>: Acquires Junction-Lock; Passing Junction;\n", pid);
		sleep(2);
		signal2.sem_num = 0;
		semop(semid_sem_sync, &signal2, 1);
		printf("Train <%d>: Releases Junction-Lock\n", pid);

		signal14.sem_num = 3;
		semop(semid_dir, &signal14, 1);
		printf("Train <%d>: Releases West-lock\n", pid);
		updateFile(0, 3);

		signal13.sem_num = 2;
		semop(semid_dir, &signal13, 1);
		printf("Train <%d>: Releases South-Lock\n", pid);
		updateFile(0, 2);
	}
	// getchar();
	return 0;
}
void updateFile(int val, int num_q)
{
	FILE *f;
	struct sembuf waitf, signalf;
	int semflg =  0; /* semflg to pass tosemget() */
	waitf.sem_num = 0;
	waitf.sem_op = -1;
	waitf.sem_flg = 0;
	signalf.sem_num = 0;
	signalf.sem_op = 1;
	signalf.sem_flg = 0;
	int semid;
	int nsems = 1;
	key_t key = 1065;
	// if ((key = ftok("matrix.txt", 'B')) == -1) {
	// 	perror("ftok");
	// 	exit(1);
	// }
	if ((semid = semget(key, nsems, semflg)) == -1) {
		perror("semget: semget failed");
		exit(1);
	}
	semop(semid, &waitf, 1);
	FILE *ftr = fopen("matrix.txt", "r");
	int i, j, k;
	int graph[75][4];
	for (i = 0; i < 75; i++) {
		for (j = 0; j < 4; j++) {
			fscanf(ftr, "%d", &graph[i][j]);
		}
	}

	graph[trainId][num_q] = val;
	int count1 = 0;
	int count2 = 0;
	ftr = fopen("matrix.txt", "w");
	for (i = 0; i < 75; i++) {
		for (j = 0; j < 4; j++) {
			fprintf(ftr, "%d ", graph[i][j]);
			if (graph[i][j] == 1) {
				count1++;
			}
			else if (graph[i][j] == 2) {
				count2++;
			}
		}
		fprintf(ftr, "\n");
	}
	fclose(ftr);
	semop(semid, &signalf, 1);
}
