#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

//****************************************************************

/* Stucture used to calculate virtual and physical memory.
 */
typedef struct
{
	uint32_t virtualMem;
	uint32_t physicalMem;
} processMem_t;


/* This function is called in GetProcessMemory() to parse the parse
 * line and return the uint32_t value of it.
 */
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

/* Calculates the physical and virtual memory used.
 */
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

//************************************************************

int main(int argc, char *arg[])
{
	// to compile use(gcc -o openmp_comp -fopenmp openmp.c)
	struct timeval t1, t2;
	int numOfThreads;
	processMem_t myMem;
	double elapsedTime;

	numOfThreads = atoi(argv[1]); // argv[0] is the program name, argv[1] is the first argument

	omp_set_num_threads(numOfThreads);

	gettimeofday(&t1, NULL);

	#pragma omp parallel
	{
		/* void *functionName(omp_get_thread_num(), numOfThreads);
		 * 
		 */
	}

	gettimeofday(&t2, NULL);
	GetProcessMemory(&myMem);

	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; // sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms

	printf("Memory: vMem, %u KB, pMem, %u KB\n", myMem.virtualMem, myMem.physicalMem);
	printf("DATA, %f\n", elapsedTime);

}
