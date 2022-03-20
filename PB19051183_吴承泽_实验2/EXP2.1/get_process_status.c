#include<linux/unistd.h>
#include<sys/syscall.h>
#include<stdio.h>
#include<unistd.h>

#define MAX 100

int main(void)
{
	int pid[MAX];
	char command[MAX][16];
	int stime[MAX];
	int result;
	
	syscall(333, &result,command,pid,stime);
	printf("process number is %d\n",result);
	printf("PID\tTIME/ms\tCOMMAND\n");

	for(int i = 0; i < result; i++)
	{
		printf("%d\t%d\t%s\n", pid[i],stime[i],command[i]);
	}
	return 0;
}
