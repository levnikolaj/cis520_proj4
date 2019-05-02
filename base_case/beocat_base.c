#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/time.h>

void algorithm();
#define NUM_WIKI_LINES 100 //Should be 1000000

void main()
{
  int i, j, p;
  FILE * fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
  //FILE * fd = fopen("test.txt", "r");
  char *buffer = NULL;
  size_t n = 0;
  char **longestCommonSubstring = (char**) malloc((NUM_WIKI_LINES-1) * sizeof(char*));
  char **wiki_dump = (char **) malloc(NUM_WIKI_LINES * sizeof(char *));
  for(i = 0; i < NUM_WIKI_LINES; i++)
  {
    size_t line_length = getline(&buffer, &n, fd);
    wiki_dump[i] = (char*)malloc((line_length) * sizeof(char));
    memcpy(wiki_dump[i], buffer, sizeof(char) * line_length);
    wiki_dump[i][line_length-2] = 0;
  }
  fclose(fd);
  //Run Algorithm
  p = 0;
  for(i = 0; i < NUM_WIKI_LINES - 1; i++, p++)
  {
    algorithm(wiki_dump[i], wiki_dump[i+1], longestCommonSubstring, p);
  }

  //Print results
  for(i = 0; i < NUM_WIKI_LINES-1; i++) // i < NUM_WIKI_LINES - 1
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
  free(wiki_dump);
}

void algorithm(char *s1, char *s2, char ** resultsArray, int p)
{
  int i, j;
	int s1_len = strlen(s1);
	int s2_len = strlen(s2);
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
  char substr[max];
	if(max > 0)
	{
    for(i = 1; i <= max; i++)
    {
      substr[max-i] = s2[col-i];
    }
    resultsArray[p] = (char *) malloc((max + 1) * sizeof(char));
    memcpy(resultsArray[p], substr, sizeof(char) * max);
    resultsArray[p][max] = 0;
	}
	free(table);
}
