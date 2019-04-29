#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

const int NUM_ITER = 1000000000;
double sum = 0.0;
double elapsedTime = 0.0;
int numOfThreads = 1;
pthread_mutex_t mutexsum; // mutex for sum

typedef struct 
{
	uint32_t virtualMem;
	uint32_t physicalMem;
} processMem_t;

int parseLine(char *line) 
{
	// This assumes that a digit will be found and the line ends in " Kb".
	int i = strlen(line);
	const char *p = line;
	while (*p < '0' || *p > '9') p++;
	line[i - 3] = '\0';
	i = atoi(p);
	return i;
}

void GetProcessMemory(processMem_t* processMem) 
{
	FILE *file = fopen("/proc/self/status", "r");
	char line[128];

	while (fgets(line, 128, file) != NULL) 
	{
		//printf("%s", line);
		if (strncmp(line, "VmSize:", 7) == 0) 
		{
			processMem->virtualMem = parseLine(line);
		}

		if (strncmp(line, "VmRSS:", 6) == 0) 
		{
			processMem->physicalMem = parseLine(line);
		}
	}
	fclose(file);
}

int min(int a, int b)
{
	if(a < b)
	{
		return a;
	}
	else
	{
		return b;
	}
	
	return a;
}

void *runCounterPThread(void* threadNum)
{	
	struct timeval t1, t2;
	int i, section_size, my_section_start, my_section_end;
	double x = 0.0;
	double local_sum = 0.0;
	double st = 1.0/((double) NUM_ITER);
	
	gettimeofday(&t1, NULL);
	
	section_size = NUM_ITER / numOfThreads;
	
	my_section_start = (int)threadNum * section_size; 
	my_section_end = ((int)threadNum + 1) * section_size;
	//printf("Thread %d, Start %d, End %d\n", (int)threadNum, my_section_start, my_section_end);
	for(i = my_section_start; i < min(my_section_end, NUM_ITER); i++)
	{
		x = (i + 0.25) * st;
		local_sum += 4.0*(x*x + 1);
	}
	
	//printf("Thread %d, Local Sum %f\n", (int)threadNum, local_sum);
	
	gettimeofday(&t2, NULL);
	
	pthread_mutex_lock(&mutexsum);
	sum += local_sum;
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	pthread_mutex_unlock (&mutexsum);
	
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{	
	processMem_t myMem; 
	int i, rc;
	void *status;
	
	numOfThreads = atoi(argv[1]);
	
	pthread_t threads[numOfThreads];
	pthread_attr_t attr;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
		
	for(i = 0; i < numOfThreads; i++)
	{
		rc = pthread_create(&threads[i], &attr, runCounterPThread, (void*)i);
		if(rc)
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	GetProcessMemory(&myMem);
	
	pthread_attr_destroy(&attr);
	for(i=0; i<numOfThreads; i++) {
	     rc = pthread_join(threads[i], &status);
	     if (rc) {
		   printf("ERROR; return code from pthread_join() is %d\n", rc);
		   exit(-1);
	     }
	}
	
	printf("SUM, %f\n", sum);
	printf("Memory: vMem, %u KB, pMem, %u KB\n", myMem.virtualMem, myMem.physicalMem);
	printf("DATA, %f\n", elapsedTime);
	
	return 0;
}
