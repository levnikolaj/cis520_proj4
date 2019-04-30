#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void algorithm();

void main()
{
  algorithm();
}


void algorithm()
{
	char s1[] = "afsaldfkjlkshfachodybanksaslkdfjasfochodychodychody";
	char s2[] = "asdfkllkjchodychodychodybankssfasdfkljsafklsa;dfuask;ldfk";
	int s1_len = strlen(s1);
	int s2_len = strlen(s2);
  //int table[s1_len + 1][s2_len + 1];
  int **table = (int **)malloc((s1_len + 1) * sizeof(int *));
  for(int i = 0; i < s1_len+1; i++)
    table[i] = (int *)malloc((s2_len + 1) * sizeof(int));
  /*int count = 0;
  for(int i = 0; i < s1_len; i++)
    for(int j = 0; j < s2_len; j++)
    table[i][j] = ++count;
  for(int i = 0; i < s1_len; i++)
    for(int j= 0; j < s2_len; j++)
      printf("%d", table[i][j]);*/


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
