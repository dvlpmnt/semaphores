#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define val 25
#define trainCount 50

int nodes[val + 50][4], source1, dest1;
int temp[val + 50][val + 50], visited[val + 50], parents[val + 50], recstack[val + 50];
int currentNodes[75];
int instack[75];
int countCurr = 0;
int cycle = 0;
int fft = 0;

pid_t trainPid[101];

void drawGraph();
void DFS(int u, int p);
int checkCycle();
void printCycle();

struct message
{
    long mtype;
    char mtext[10000];
} msg;

int main(int argc, char *argv[])
{
	srand48 ( time(NULL) );
	int nsems = 1;
	int semflg =  IPC_CREAT | 0666 ; /* semflg to pass tosemget() */
	double p;
	if (argv[1] != NULL)
		p = atof(argv[1]);
	int i, j, n;
	if (argc != 2 || p < 0.2 || p > 0.7)
	{
		printf("Invalid probability!\n");
		return 1;
	}

	char temp[100];
	FILE *fp;
	fp = fopen("sequence.txt", "r");
	fscanf(fp, "%[^\n]", temp);
	n = strlen(temp);
	fclose(fp);

	int mat[75][4];
	fp = fopen("matrix.txt", "w+");
	for (i = 0; i < 75; i++)
	{
		for (j = 0; j < 4; j++)
		{
			mat[i][j] = 0;
			fprintf(fp, "%d ", mat[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);

	key_t key_sem_sync = 1025;
	key_t key_dir = 1045;
	int semid_sem_sync, semid_dir, semid;
	struct sembuf wait1, signal1;
	ushort synDir[4] = {1, 1, 1, 1};
	ushort synVal[2] = {1, 1};
	ushort synFile = 1;
	if ((semid_sem_sync = semget(key_sem_sync, 2, IPC_CREAT | 0666)) == -1)
	{
		perror("semget: semget failed");
		exit(1);
	}
	if ((semid_dir = semget(key_dir, 4, IPC_CREAT | 0666)) == -1)
	{
		perror("semget: semget failed");
		exit(1);
	}
	key_t key = 1065;
	// if ((key = ftok("matrix.txt", 'B')) == -1) {
	// 	perror("ftok");
	// 	exit(1);
	// }
	if ((semid = semget(key, nsems, semflg)) == -1) {
		perror("semget: semget failed");
		exit(1);
	}

	if (semctl(semid_sem_sync, 0, SETALL, synVal) == -1) {
		printf("SETVAL error\n");
	}

	if (semctl(semid_dir, 0, SETALL, synDir) == -1) {
		printf("SETVAL error\n");
	}

	if (semctl(semid, 0, SETVAL, synFile) == -1) {
		printf("SETVAL error\n");
	}
	//0->N 1->E 2->S 3->W
	//0->junction 1->file
	wait1.sem_num = 0;
	wait1.sem_op = -1;
	wait1.sem_flg = 0;
	signal1.sem_num = 0;
	signal1.sem_op = 1;
	signal1.sem_flg = 0;
	int cycleDetect = 0;
	char callProc[100];
	int k = 0;
	do {
		float ran = drand48();
		if (ran < p && k < n)
		{
			k++;
			pid_t child1 = fork();
			if (child1 == 0)
			{
				memset(callProc, '\0', 100);
				sprintf(callProc, "./t %c %d", temp[k - 1], k);
				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", callProc , (void*)NULL);
			}
		}
		else {
			semop(semid, &wait1, 1);
			FILE *f = fopen("matrix.txt", "r");
			int count1 = 0;
			int count2 = 0;
			for (i = 0; i < 75; i++) {
				for (j = 0; j < 4; j++) {
					fscanf(f, "%d", &nodes[i][j]);
					if (nodes[i][j] == 1) {
						count1++;
					}
					else if (nodes[i][j] == 2) {
						count2++;
					}
				}
				fscanf(f, "\n");
			}
			fclose(f);
			semop(semid, &signal1, 1);
			if (checkCycle() == 1) {
				cycleDetect = 1;
				break;
			}
		}
		if (k == n) {
			break;
		}
	} while (1);

	key_t keymsg = 1024;
	int msgid;
	msgid = msgget(keymsg, IPC_CREAT | 0644);
	int ll;
	for(ll = 0; ll < n ; ll++)
	{
		memset(msg.mtext, '\0', sizeof(msg.mtext));
    	while( msgrcv(msgid, &msg, sizeof(msg.mtext), 200, 0) == -1);
    	trainPid[ll] = atoi(msg.mtext);
	}
	while (1 && !cycleDetect) {
		sleep(1);
		semop(semid, &wait1, 1);
		FILE *f = fopen("matrix.txt", "r");
		int count1 = 0;
		int count2 = 0;

		for (i = 0; i < 75; i++) {
			for (j = 0; j < 4; j++) {
				fscanf(f, "%d", &nodes[i][j]);
				if (nodes[i][j] == 1) {
					count1++;
				}
				else if (nodes[i][j] == 2) {
					count2++;
				}
			}
			fscanf(f, "\n");
		}
		// printf("count1->%d\n", count1);
		// printf("count2->%d\n", count2);
		if (count1 == 0 && count2 == 0)
		{
			printf("All Trains have crossed with no Deadlock\n");
			break;
		}
		else
			printf("No deadlocks yet\n\n");
		fclose(f);
		semop(semid, &signal1, 1);
		if (checkCycle() == 1) {
			cycleDetect = 1;
			break;
		}
	}

	getchar();
	// if (semctl(semid_sem_sync, 0, IPC_RMID) == -1)
	// 	printf("SEMREM error\n");
	// if (semctl(semid_dir, 0, IPC_RMID) == -1)
	// 	printf("SEMREM error\n");
	for(ll = 0 ;ll < n; ll++)
		kill(trainPid[ll],SIGINT);
	return 0;
}

void drawGraph()
{
	int i, j;
	memset(temp, 0, sizeof(temp));
	memset(visited, 0, sizeof(visited));
	memset(parents, 0, sizeof(parents));
	memset(recstack, 0, sizeof(recstack));
	for (i = 0; i < trainCount; i++) {
		for (j = 0; j < 4; j++) {
			if (nodes[i][j] == 1) {
				temp[i][j + trainCount] = 1;
			}
			if (nodes[i][j] == 2) {
				temp[j + trainCount][i] = 1;
			}
		}
	}

	for (i = 0; i < trainCount + 4; i++) {
		temp[i][0] = 0;
		temp[0][i] = 0;
	}
}

void DFS(int u, int p)
{
	visited[u] = 1;
	int i, j, k;
	countCurr++;
	instack[u] = 1;
	if (cycle) {
		return ;
	}
	currentNodes[countCurr] = u;
	for (i = 0; i < 75; i++) {
		if (temp[u][i] == 1 && !visited[i] && i != p) {
			DFS(i, u);
		}
		else {
			if (cycle) {
				return ;
			}
			if (visited[i] && temp[u][i] && instack[i]) {
				int start = u;
				int end = i;
				printf("\nSystem Deadlocked\n");
				int trainList[100];
				int index1 = 0;
				while (1) {
					int index = currentNodes[countCurr];
					if (index >= trainCount) {
						// printf("Resource->%d\n", index - trainCount);
					}
					else {
						trainList[index1] = index;
						index1++;
						// printf("Train->%d\n", index);
					}
					if (countCurr == 0 || currentNodes[countCurr] == end) {
						break;
					}
					countCurr--;
					cycle = 1;
				}
				// printf("Train list of dependecies->\n");
				int l = 0;
				for (l = 0; l < index1 - 1; l++) {
					printf("Train <%d> is waiting on Train <%d> ---------> ", trainPid[trainList[l % index1] - 1], trainPid[trainList[(l + 1) % index1] - 1]);
				}
				printf("Train <%d> is waiting on Train <%d>\n", trainPid[trainList[l % index1] - 1], trainPid[trainList[(l + 1) % index1] - 1]);
			}
		}
	}
	instack[u] = 0;
	countCurr--;
}

int checkCycle()
{
	drawGraph();
	memset(visited, 0, sizeof(visited));
	memset(instack, 0, sizeof(instack));
	int i = 0;
	for (i = 1; i <= 74; i++) {
		if (!visited[i]) {
			DFS(i, 0);
			countCurr = 0;
			if (cycle) {
				break;
			}
		}
	}
	return cycle;
	return 0;
}

void printCycle()
{
	int i, j;
	i = source1;
	j = 0;
	int cycle[100];
	cycle[j++] = dest1;
	while (i != dest1) {
		cycle[j++] = i;
		int k = i;
		if (k >= trainCount) {
			printf("Resource->%d\n", k - trainCount);
		}
		else {
			printf("Train->%d\n", k);
		}
		i = parents[i];
	}
	cycle[j] = dest1;
	i = j;
}
