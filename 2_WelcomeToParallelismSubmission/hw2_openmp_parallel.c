#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

const int NUM_ITER = 5000000;
double sum = 0.0;

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

void *runCounterOpenMP(int threadNum, int numOfThreads)
{
	int i, section_size, my_section_start, my_section_end;
	double x = 0.0;
	double local_sum = 0.0, local_x = 0.0;
	double st = 1.0/((double)NUM_ITER);
	
	section_size = NUM_ITER / numOfThreads;
	
	#pragma omp private(i, local_sum, local_x, st, my_section_end, my_section_start)
	{
		my_section_start = threadNum * section_size; 
		my_section_end = (threadNum + 1) * section_size;
		//printf("Thread %d, Start %d, End %d\n", threadNum, my_section_start, my_section_end);
		for(i = my_section_start; i < min(my_section_end, NUM_ITER); i++)
		{
			local_x = (i + 0.25) * st;
			local_sum += 4.0*(local_x*local_x + 1);
		}
		//printf("Thread %d, Local Sum %f\n", threadNum, local_sum);
		#pragma omp critical
		{
			sum += local_sum;
		}		
	}	
}

int main(int argc, char *argv[])
{
	// to compile use(gcc -o openmp_comp -fopenmp hw2_openmp_parallel.c)
	struct timeval t1, t2;
	int numOfThreads;
	processMem_t myMem; 
	double elapsedTime;	
	
	numOfThreads = atoi(argv[1]);
	
	//move below #pragma
	omp_set_num_threads(numOfThreads);
	
	gettimeofday(&t1, NULL);
	
	#pragma omp parallel
	{
		runCounterOpenMP(omp_get_thread_num(), numOfThreads);
	}	
	
	gettimeofday(&t2, NULL);
	GetProcessMemory(&myMem);
	printf("SUM, %f\n", sum);
	
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	
	printf("Memory: vMem, %u KB, pMem, %u KB\n", myMem.virtualMem, myMem.physicalMem);
	printf("DATA, %f\n", elapsedTime);
	
	return 0;
}
