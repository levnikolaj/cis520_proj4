#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/time.h>
#include <pthread.h>

#define NUM_WIKI_LINES 1000000


pthread_mutex_t mutexsum; // mutex for adding to longestCommonSubstring

void* algorithm(void* args);

// gcc -o pthreads_comp -pthread pthreads.c

/* Stucture used to calculate virtual and physical memory.
 */
typedef struct
{
	uint32_t virtualMem;
	uint32_t physicalMem;
} processMem_t;

typedef struct
{
	char **longestCommonSubstring;
	char **wiki_dump;
	int startIndex;
	int endIndex;
} algorithmArgs_t;
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

/* Calculates the physical and virtual memory used
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

void main(int argc, char *argv[])
{
  int i, j;
  FILE * fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
  char *buffer = NULL;
  size_t n = 0;
	double elapsedTime = 0.0;
  char **wiki_dump = (char **) malloc(NUM_WIKI_LINES * sizeof(char *));
  char **longestCommonSubstring = (char **) malloc((NUM_WIKI_LINES - 1)* sizeof(char *));
	processMem_t myMem;
	struct timeval t1, t2;
	int numOfThreads = atoi(argv[1]); //TODO: get from command line argument
	int linesToProcess = NUM_WIKI_LINES;
	int sectionSize = linesToProcess/numOfThreads;
	int rc;
	void *status;

	pthread_t threads[numOfThreads];
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for(i = 0; i < NUM_WIKI_LINES; i++)
  {
    size_t line_length = getline(&buffer, &n, fd);
    wiki_dump[i] = (char*)malloc((line_length) * sizeof(char));
    memcpy(wiki_dump[i], buffer, sizeof(char) * line_length);
    wiki_dump[i][line_length-2] = 0;
  }
	/*
	for(i = 0; i < 10; i++)
	{
		printf("%s\n", wiki_dump[i]);
	}
	*/
	gettimeofday(&t1, NULL);

	for(i = 0; i < numOfThreads; i++) // i < numOfThreads
	{
		algorithmArgs_t *args = (algorithmArgs_t*) malloc(sizeof(algorithmArgs_t));
		args->wiki_dump = wiki_dump;
		args->longestCommonSubstring = longestCommonSubstring;
		args->startIndex = i * sectionSize;

		if(i != (numOfThreads - 1))
		{
			args->endIndex = (i + 1) * sectionSize - 1; // OR without -1 and have algorithm run to < instead of <=
		}
		else
		{
			args->endIndex = linesToProcess - 1;
		}
		//printf("Iteration:%d, start:%d, end:%d\n", i, args->startIndex, args->endIndex);
		rc = pthread_create(&threads[i], &attr, algorithm, (void*)args);
		if(rc)
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	GetProcessMemory(&myMem);

	pthread_attr_destroy(&attr);
	for(i = 0; i < numOfThreads; i++)
	{
		rc = pthread_join(threads[i], &status);
		if(rc)
		{
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}

	gettimeofday(&t2, NULL);

	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms

	printf("Memory, Threads, %d, vMem, %u KB, pMem, %u KB\n", numOfThreads, myMem.virtualMem, myMem.physicalMem);
	printf("DATA, %f, Threads, %d\n", elapsedTime, numOfThreads);


	// TODO: add memory and time output
	/*
	for(i = 0; i < linesToProcess - 1; i++) // i < NUM_WIKI_LINES - 1
	{
		printf("Lines%d-%d: ", i, i+1);
		if(longestCommonSubstring[i] != NULL)
		{
			printf("%s\n", longestCommonSubstring[i]);
		}
		else
		{
			printf("None found\n");
		}
	}
	*/

	free(longestCommonSubstring);
	free(wiki_dump);
}

/* The algorithm that finds the longest common substring.
 * Uses matrix table, dynamic array allocation.
 *
 */
void* algorithm(void* parameters)
{
	algorithmArgs_t *args = (algorithmArgs_t *) parameters;
  int p, i, j, s1_len, s2_len, col, val, max = 0;
	//printf("Thread:%u, s:%d, e:%d\n", self_id, args->startIndex, args->endIndex);
	for(p = args->startIndex; p < args->endIndex; p++) // endIndex is -1 before set
	{
		max = 0;
		// TODO: check pointer
		char *s1 = args->wiki_dump[p];
		char *s2 = args->wiki_dump[p + 1];
		s1_len = strlen(args->wiki_dump[p]);
		s2_len = strlen(args->wiki_dump[p + 1]);

	  int **table = (int **)malloc((s1_len + 1) * sizeof(int *));
	  for(i = 0; i < s1_len+1; i++)
	    table[i] = (int *)malloc((s2_len + 1) * sizeof(int));


		//initialize outside of table to zeros
		for(i = 0; i <= s1_len; i++)
			table[i][0] = 0;
		for(i = 0; i <= s2_len; i++)
			table[0][i] = 0;

		//set rest of table to correct values
		//get max value of table and store associated column
		for(i = 1; i <= s1_len; i++)
		{
			for(j = 1; j <= s2_len; j++)
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
			for(i = 1; i <= max; i++)
			{
				substr[max-i] = s2[col-i];
			}
			//printf("p:%d %u\n", p, self_id);
			pthread_mutex_lock(&mutexsum);
			args->longestCommonSubstring[p] = (char *) malloc((max + 1) * sizeof(char));
			memcpy(args->longestCommonSubstring[p], substr, sizeof(char) * max);
			args->longestCommonSubstring[p][max] = 0;
			pthread_mutex_unlock (&mutexsum);
		}

		for(i = 0; i < s1_len + 1; i++)
		{
			free(table[i]);
		}
		free(table);
	}
	free(args);
	pthread_exit(NULL);
}
