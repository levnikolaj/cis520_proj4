#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/time.h>

#define NUM_WIKI_LINES 1000000

char **longestCommonSubstring;

void algorithm();
int parseLine(char *line);
void GetProcessMemory(processMem_t* processMem);

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

void main()
{
  int i, j;
  FILE * fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
  char *buffer = NULL;
  size_t n = 0;
  char **wiki_dump = (char **) malloc(NUM_WIKI_LINES * sizeof(char *));
  longestCommonSubstring = (char **) malloc((NUM_WIKI_LINES - 1)* sizeof(char *));

  for(i = 0; i < NUM_WIKI_LINES; i++)
  {
    size_t line_length = getline(&buffer, &n, fd);
    wiki_dump[i] = (char*)malloc((line_length) * sizeof(char));
    memcpy(wiki_dump[i], buffer, sizeof(char) * line_length);
    wiki_dump[i][line_length-2] = 0;
  }

  #pragma omp parallel for
    for (i = 0; i < NUM_WIKI_LINES - 1; i++)
    {
      algorithm(*wiki_dump + i,*wiki_dump + i + 1, i);
    }
		// TODO: add memory and time output
}

/* The algorithm that finds the longest common substring.
 * Uses matrix table, dynamic array allocation.
 *
 */
void algorithm(char *firstLine, char*secondLine, int firstEntryIndex)
{
  int i, j;
	int s1_len = strlen(firstLine);
	int s2_len = strlen(secondLine);
  //int table[s1_len + 1][s2_len + 1];
  int **table = (int **)malloc((s1_len + 1) * sizeof(int *));
  for(i = 0; i < s1_len+1; i++)
    table[i] = (int *)malloc((s2_len + 1) * sizeof(int));


	//initialize outside of table to zeros
	for(i = 0; i <= s1_len; i++)
		table[i][0] = 0;
	for(i = 0; i <= s2_len; i++)
		table[0][i] = 0;

	int max = 0;
	int col;
	int val;

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

		printf("longest common substring: ");
		for(i = 0; i < max; i++)
    {
      // TODO: add to longestCommonSubstring array, critical section
      // malloc char* into longestCommonSubstring and memcpy
    	printf("%c", substr[i]);
    }

		printf("\n");
	}
	else
		printf("No common substring.\n"); // TODO: add to longestCommonSubstring array, critical section

	free(table);
}
