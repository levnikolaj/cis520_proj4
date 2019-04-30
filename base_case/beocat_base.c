#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void algorithm();

void main()
{
  int i, j;
  FILE * fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
  char *buffer = NULL;
  size_t n = 0;
  char **wiki_dump = (char **) malloc(1000000 * sizeof(char *));
  for(i = 0; i < 1000000; i++)
  {
    size_t line_length = getline(&buffer, &n, fd);
    wiki_dump[i] = (char*)malloc((line_length) * sizeof(char));
    memcpy(wiki_dump[i], buffer, sizeof(char) * line_length);
    wiki_dump[i][line_length-2] = 0;
  }
  for(i = 0; i < 50; i++)
  {
    for(j = 0; wiki_dump[i][j] != 0; j++)
    {
      printf("%c", wiki_dump[i][j]);
    }
    printf("\n");
  }
  //algorithm();
}

void algorithm()
{
  int i, j;
	char s1[] = "afsaldfkjlkshfachodybanksaslkdfjasfochodychodychody";
	char s2[] = "asdfkllkjchodychodychodybankssfasdfkljsafklsa;dfuask;ldfk";
	int s1_len = strlen(s1);
	int s2_len = strlen(s2);
  //int table[s1_len + 1][s2_len + 1];
  int **table = (int **)malloc((s1_len + 1) * sizeof(int *));
  for(i = 0; i < s1_len+1; i++)
    table[i] = (int *)malloc((s2_len + 1) * sizeof(int));
  /*int count = 0;
  for(int i = 0; i < s1_len; i++)
    for(int j = 0; j < s2_len; j++)
    table[i][j] = ++count;
  for(int i = 0; i < s1_len; i++)
    for(int j= 0; j < s2_len; j++)
      printf("%d", table[i][j]);*/


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
			printf("%c", substr[i]);
		printf("\n");
	}
	else
		printf("No common substring.\n");
	free(table);
}
