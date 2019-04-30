#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"


#define PATH_TO_WIKI_DUMP				"/homes/levnikolaj/Proj4_520/wiki_dump.txt"
#define MAX_CHARS_READ 					10000000
/* Dynamically allocated array for storing substrings.
 */
char **longestSubstring;
void *findLongestSubstring();

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

int main()
{
	// to compile use(gcc -o openmp_comp -fopenmp openmp.c)
	struct timeval t1, t2;
	int numOfThreads;
	processMem_t myMem;
	double elapsedTime;

	//numOfThreads = atoi(argv[1]); // argv[0] is the program name, argv[1] is the first argument, should be number of threads

	//omp_set_num_threads(numOfThreads);

	gettimeofday(&t1, NULL);

	/*
	#pragma omp parallel
	{
		 void *functionName(omp_get_thread_num(), numOfThreads);
	}
  */

	findLongestSubstring();

	gettimeofday(&t2, NULL);
	GetProcessMemory(&myMem);

	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; // sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms

	printf("Memory: vMem, %u KB, pMem, %u KB\n", myMem.virtualMem, myMem.physicalMem);
	printf("DATA, %f\n", elapsedTime);

}

void *findLongestSubstring()
{
	FILE *fd;
	char firstLine[MAX_CHARS_READ];
	char secondLine[MAX_CHARS_READ];
	fd = fopen("/homes/levnikolaj/Proj4_520/wiki_dump.txt", "r");

	fgets(firstLine, MAX_CHARS_READ, fd);
	fgets(secondLine, MAX_CHARS_READ, fd);

	printf("Line 1, %s\n", firstLine);
	printf("Line 2, %s\n", secondLine);

	fclose(fd);
}

void algorithm()
{
	char s1[] = "afsaldfkjlkshfachodybanksaslkdfjasfochodychodychody";
	char s2[] = "asdfkllkjchodychodychodybankssfasdfkljsafklsa;dfuask;ldfk";
	int s1_len = strlen(s1);
	int s2_len = strlen(s2);
	//int table[s1_len + 1][s2_len + 1];
	int table = malloc(sizeof(int) * ((s1_len+1) * (s2_len+1)));
	//initialize outside of table to zeros
	for(int i = 0; i <= s1_len; i++)
		table[i][0] = 0;
	for(int i = 0; i <= s2_len; i++)
		table[0][i] = 0;

	int max = 0;
	int col;
	int val;

	//set rest of table to correct values
	//get max value of table and store associated column
	for(int i = 1; i <= s1_len; i++)
	{
		for(int j = 1; j <= s2_len; j++)
		{
			if(s1[i-1] == s2[j-1])
			{
				val = table[i-1][j-1] + 1;
				table[i][j] = val;
				if(val > max)
				{
					max = val;
					col = j;
				}
			}
			else
				table[i][j] = 0;
		}
	}

	//create longest substring based on index of max value
	if(max > 0)
	{
		char substr[max];
		for(int i = 1; i <= max; i++)
		{
			substr[max-i] = s2[col-i];
		}

		printf("longest common substring: ");
		for(int i = 0; i < max; i++)
			printf("%c", substr[i]);
		printf("\n");
	}
	else
		printf("No common substring.\n");
	free(table);
}
