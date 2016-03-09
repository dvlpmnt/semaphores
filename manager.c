#include <stdio.h>
#include <unistd.h>

int graph[10][10] = {0};
int done = 0;

void read_graph( );
void DFSCheckCycle( );
void Visit(int p[], int u, int color[]);
void PrintCycle(int p[], int v, int u);

int main()
{
	int i, j;
	pid_t p, p1, p2;
	char pRun[100] = {"./p"};
	char cRun[100] = {"./c"};
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
	sleep(3);
	printf("yo\n");
	p = fork();
	if (p == 0)
	{
		for (i = 0; i < 5; i++)
		{
			p1 = fork();
			if (p1 == 0)
			{
				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", pRun, (void*)NULL);
			}
			p2 = fork();
			if (p2 == 0)
			{
				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", cRun, (void*)NULL);
			}
			sleep(1);
		}
	}
	else
	{
		while (1)
		{
			read_graph();
			DFSCheckCycle();
			sleep(1);
		}
	}
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
			fscanf(fp, "%d ", &temp);
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

void DFSCheckCycle ()
{
	int p[10], color[10];
	int i, j;
	for (j = 0; j < 10; j++)
	{
		p[j] = -2;
		color[j] = 0;
	}

	for (i = 0; i < 10; i++)
	{
		if (color[i] == 0) {
			p[i] = -1; // meaning it is a root of a DFS-tree of the DFS forest
			Visit(p, i, color);
		}
	}
}

void Visit(int p[], int u, int color[])
{
	int cycle = 0, v ;
	color[u] = 1;
	for (v = 0; v < 10; v++)
		if (graph[u][v] == 1)
		{
			if (color[v] == 0) {
				p[v] = u;
				Visit(p, v, color);
			}
			else if (color[v] == 1)
			{
				cycle = 1;
				break;
			}
		}
	color[u] = 2; // once DFS for this vertex ends, assign its color to black

	if (cycle == 1)	//there is a cycle
		PrintCycle(p, v, u);
}

void PrintCycle(int p[], int v, int u)
{
	do {
		printf(" %2d ->", u);
		u = p[u];
		if (u == -1)
		{
			break;
		}
	} while (u != v);
}