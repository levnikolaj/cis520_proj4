#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

#define TAG_MSG_CHUNK 				1
#define TAG_MSG_RESULT_LEN 			2
#define TAG_MSG_RESULT_DATA			3	
#define TAG_MSG_EXIT_PROCESS 		4

#define MAX_WORDS 					50000
#define MAX_LINES					1000000

#define DBG(x)

double elapsedTime = 0.0;
const int LINES_PER_CHUNK = 500;
const int MAX_LINE_LENGTH = 2001;
const int MAX_KEYWORD_LENGTH = 10;
const int CHUNK_SIZE = sizeof(long) + LINES_PER_CHUNK * MAX_LINE_LENGTH;

typedef struct resList 
{
	long *results;
    long lineResCount;
    struct resList *next;
} resList;

typedef struct 
{
	uint32_t virtualMem;
	uint32_t physicalMem;
} processMem_t;

int parseLine(char *line);
void GetProcessMemory(processMem_t* processMem);
int processChunk(int startLine, char *chunkLines, char *keywords, int numKeywords, long **results, long *resTotalLongs);
int cleanupResults(resList **lineRes, int nwords);
int outputResults(resList **lineRes, int nwords, char *words);

int insertResult(long *results, long resTotalLongs, resList **lineRes, int nwords);

int main(int argc, char *argv[])
{
	processMem_t myMem;
	struct timeval t1, t2;
	resList **lineRes = NULL;
	int i, rc, err, numTasks, rank, nlines, lineStart, currChunk, chunksSent;
	long totalLines, nchunks, *results, resTotalLongs, recvTotalLongs, tag, tmpLen, longestLineLen = 0, nwords = 0;
	int maxWords = MAX_WORDS, maxLines = MAX_LINES;
	double nchars = 0;
	char *words, *lines, **chunks, *lineBuff;
	FILE *fd;
	MPI_Status Status;
	MPI_Request *request = NULL;
	
	rc = MPI_Init(&argc, &argv);
	if(rc != MPI_SUCCESS)
	{
		printf("Error starting MPI program. Terminating.\n");
		fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	
	MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if(rank == 0)
	{
		gettimeofday(&t1, NULL);
	}
	
	printf("size = %d rank = %d\n", numTasks, rank);
	fflush(stdout);
	
	// This MPI_Bcast allows the 2 MPI_Bcast calls to work below	
	rc = MPI_Bcast(&nwords, 1, MPI_LONG, 0, MPI_COMM_WORLD);
	DBG(printf("(%d) nwords: %ld\n", __LINE__, nwords);
	fflush(stdout);)
	
	
	// Every process allocates memory the word array.
	words = (char *)malloc((maxWords + 1) * MAX_KEYWORD_LENGTH);
	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0)
	{		
		// Read in keywords
		fd = fopen("/homes/dan/625/keywords.txt", "r");
		nwords = 0;
		do 
		{
			err = fscanf(fd, "%[^\n]\n", &words[nwords*MAX_KEYWORD_LENGTH]);
			words[(nwords+1)*MAX_KEYWORD_LENGTH - 1] = 0;
			nwords++;
		} while (err != EOF && nwords < maxWords);
		fclose(fd);
		printf("(%ld) Read in %ld words\n", (long)__LINE__, nwords);
		fflush(stdout);
		
		lineRes = (resList**)calloc(nwords, sizeof(void*));
		// Read in wiki entries
		fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
		nlines = 0;
		nchunks = 0;
		totalLines = 0;
		lineBuff = (char *)malloc(CHUNK_SIZE);
		lines = lineBuff + sizeof(long);
		memcpy(lineBuff, &totalLines, sizeof(totalLines));
		chunks = (char **)malloc((maxLines/LINES_PER_CHUNK + 1)*sizeof(void*));
		fflush(stdout);
		do
		{
			err = fscanf(fd, "%[^\n]\n", &lines[nlines*MAX_LINE_LENGTH]);
			if(err == EOF)
			{
				lines[nlines*MAX_LINE_LENGTH] = 0;
				break;
			}
			tmpLen = strlen(&lines[nlines*MAX_LINE_LENGTH]);
			nchars += (double)tmpLen;
			totalLines++;
			nlines++;
			DBG(printf("(%ld) totalLines: %ld\n", (long)__LINE__, totalLines);fflush(stdout);)
			if(tmpLen > longestLineLen)
			{
				longestLineLen = tmpLen;
				printf("(%ld) longestLineLen: %ld\n", (long)__LINE__, longestLineLen);
				fflush(stdout);
			}
			if(nlines >= LINES_PER_CHUNK)
			{
				chunks[nchunks++] = lineBuff;
				lineBuff = (char *)malloc(CHUNK_SIZE);
				lines = lineBuff + sizeof(long);
				// first long, starting line number
				memcpy(lineBuff, &totalLines, sizeof(totalLines));
				nlines = 0;
				DBG(printf("(%ld) nchunks: %ld\n", (long)__LINE__, nchunks);fflush(stdout);)
			}
		} while(err != EOF && totalLines < maxLines);
		
		if(nlines > 0)
		{
			chunks[nchunks++] = lineBuff;
		}
		else
		{
			free(lineBuff);
		}	
		fclose(fd);
		printf("Read in %ld lines averaging %.0lf chars/line\n", (long)totalLines, nchars/totalLines);
		fflush(stdout);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	DBG(printf("(%ld) Rank: %ld, nwords: %ld\n", (long)__LINE__, (long)rank, nwords);fflush(stdout);)
	rc = MPI_Bcast(&nwords, 1, MPI_LONG, 0, MPI_COMM_WORLD); // Error is in this line
	DBG(printf("(%ld) Rank: %ld, nwords: %ld\n", (long)__LINE__, (long)rank, nwords);fflush(stdout);)
	if(rc != MPI_SUCCESS)
	{
		printf("MPI_Bcast nwords Failed.\n");
		fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	DBG(printf("(%ld) Rank: %ld, Broadcast nwords.\n", (long)__LINE__, (long)rank);fflush(stdout);)	
	rc = MPI_Bcast(words, nwords*MAX_KEYWORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);	
	if(rc != MPI_SUCCESS)
	{
		printf("MPI_Bcast Words Failed.\n");
		fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}	
	MPI_Barrier(MPI_COMM_WORLD);
	DBG(printf("(%ld) Rank: %ld, Both Broadcasts.\n", (long)__LINE__, (long)rank);)
	fflush(stdout); 
	currChunk = 0; 
	// Send share of **line to each process
	if(rank == 0)
	{
		request = (MPI_Request *)malloc(numTasks * sizeof(MPI_Request));
		while(1)
		{
			chunksSent = 0;			
			// Here 'i' represents the rank of the receiving process
			for(i = 1; i < numTasks; i++)
			{
				DBG(printf("Prior Isend.\n");)
				rc = MPI_Isend(chunks[currChunk], CHUNK_SIZE, MPI_CHAR, i, TAG_MSG_CHUNK, MPI_COMM_WORLD, &request[i]);
				DBG(printf("After Isend.\n");)
				if(rc != MPI_SUCCESS)
				{
					printf("MPI_Isend Failed for process%d. Error: %d\n", i, rc);
					MPI_Abort(MPI_COMM_WORLD, rc);
				}
				currChunk++;
				chunksSent++;
				if(currChunk >= nchunks - 1)
				{
					break;
				}
				DBG(printf("(%d) SentTo: %d\n", __LINE__, i);fflush(stdout);)
			}
			DBG(printf("(%ld) Prior to memcpy.\n", __LINE__);)
			memcpy(&lineStart, chunks[currChunk], sizeof(long));
			results = NULL;
			resTotalLongs = 0;
			DBG(printf("(%ld) Prior to processChunk.\n", __LINE__);)
			processChunk(lineStart, chunks[currChunk] + sizeof(long), words, nwords, &results, &resTotalLongs);
			printf("(%d) Rank: 0, lineStart:%d, resTotalLongs: %ld\n", __LINE__, lineStart, resTotalLongs);
			fflush(stdout);
			insertResult(results, resTotalLongs, lineRes, nwords);
			DBG(printf("(%d) After Insert Result\n", __LINE__);fflush(stdout);)
			currChunk++;
			
			for(i = 0; i < chunksSent; i++)
			{
				rc = MPI_Recv(&recvTotalLongs, 1, MPI_LONG, MPI_ANY_SOURCE,TAG_MSG_RESULT_LEN, MPI_COMM_WORLD, &Status);
				if(rc != MPI_SUCCESS)
				{
					printf("MPI_Recv Data Size Failed. Error: %d\n", rc);
					MPI_Abort(MPI_COMM_WORLD, rc);
				}
				if(resTotalLongs < recvTotalLongs)
				{
					results = (long*)realloc(results, recvTotalLongs*sizeof(long));
					resTotalLongs = recvTotalLongs;
				}
				rc = MPI_Recv(results, recvTotalLongs, MPI_LONG, Status.MPI_SOURCE, TAG_MSG_RESULT_DATA, MPI_COMM_WORLD, &Status);
				if(rc != MPI_SUCCESS)
				{
					printf("MPI_Recv DATA Failed. Error: %d\n", rc);
					MPI_Abort(MPI_COMM_WORLD, rc);
				}
				insertResult(results, recvTotalLongs, lineRes, nwords);
			}
			
			if(currChunk >= nchunks)
			{
				DBG(printf("(%ld) currChunk: %ld, nchunks: %ld\n", (long)__LINE__, (long)currChunk, (long)nchunks);fflush(stdout);)
				tag = TAG_MSG_EXIT_PROCESS;
				for(i = 1; i < numTasks; i++)
				{
					rc = MPI_Send(&tag, 1, MPI_LONG, i, TAG_MSG_EXIT_PROCESS,MPI_COMM_WORLD);
					if(rc != MPI_SUCCESS)
					{
						printf("MPI_Send Exit Failed. Error: %d\n", rc);
						MPI_Abort(MPI_COMM_WORLD, rc);
					}
				}
				break;
			}
		}
		free(request);
	}
	else
	{
		char *rcvBuff = (char *)malloc(CHUNK_SIZE);
		results = NULL;
		resTotalLongs = 0;
		while(1)
		{
			rc = MPI_Recv(rcvBuff, CHUNK_SIZE, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
			if(rc != MPI_SUCCESS)
			{
				printf("MPI_Recv Chunk Failed. Error: %d\n", rc);
				//MPI_Abort(MPI_COMM_WORLD, rc);
			}
			if(Status.MPI_TAG == TAG_MSG_CHUNK)
			{
				memcpy(&lineStart, rcvBuff, sizeof(long));
				processChunk(lineStart, rcvBuff + sizeof(long), words, nwords, &results, &resTotalLongs);
				DBG(printf("(%d) Rank: %d, lineStart:%d, resTotalLongs: %ld\n", __LINE__, rank, lineStart, resTotalLongs);)
				// !!! Insert Error Handling
				rc = MPI_Send(&resTotalLongs, 1, MPI_LONG, 0, TAG_MSG_RESULT_LEN, MPI_COMM_WORLD);
				rc = MPI_Send(results, resTotalLongs, MPI_LONG, 0, TAG_MSG_RESULT_DATA, MPI_COMM_WORLD);
			}
			else
			{
				printf("Non CHUNK Tag: %d. Exiting.\n", Status.MPI_TAG);
				break;
			}			
		}
		free(rcvBuff);
	}	
	
	if(rank == 0)
	{
		outputResults(lineRes, nwords, words);
		cleanupResults(lineRes, nwords);
	}
	
	if(rank == 0)
	{
		gettimeofday(&t2, NULL);
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
		printf("(%ld) DATA, %f\n", (long)__LINE__, elapsedTime);
	}
	GetProcessMemory(&myMem);
	printf("(%ld) Rank: %ld, Memory: vMem, %u KB, pMem %u KB\n", (long)__LINE__, (long)rank, myMem.virtualMem, myMem.physicalMem);
	fflush(stdout);
	MPI_Finalize();
	
	return 0;
}

int cleanupResults(resList **lineRes, int nwords)
{
	long i, j;
	resList *currNode, *nextNode;
	
	for(i = 0; i < nwords; i++)
	{
		currNode = lineRes[i];
		while(currNode)
		{
			nextNode = currNode->next;
			free(currNode->results);
			free(currNode);
			currNode = nextNode;
		}
	}
	free(lineRes);
	return 0;
}

int outputResults(resList **lineRes, int nwords, char *words)
{
	char placeComma = 0;
	long i, j;
	FILE *fd;
	resList *currNode;
	
	fd = fopen("/homes/levnikolaj/CurrentWork/wiki_out.txt", "w");
	for(i = 0; i < nwords; i++)
	{
		DBG(printf("(%ld) wordIndex: %ld\n", (long)__LINE__, i);)
		currNode = lineRes[i];
		fprintf(fd, "%s: ", &words[MAX_KEYWORD_LENGTH * i]);
		while(currNode)
		{
			for(j = 0; j < currNode->lineResCount; j++)
			{
				if(placeComma)
				{
					fprintf(fd,", %ld", currNode->results[j]);
				}
				else
				{
					fprintf(fd, "%ld", currNode->results[j]);
					placeComma = 1;
				}				
			}
			currNode = currNode->next;
		}
		placeComma = 0;
		fprintf(fd, "\n");
		DBG(printf("(%ld)\n", (long)__LINE__);)
	}
	
	fclose(fd);
	return 0;
}

int insertResult(long *results, long resTotalLongs, resList **lineRes, int nwords)
{
	long resPos = 0;
	long wordIndex;
	resList *node, *currNode, *prevNode; 
	
	while(results[resPos] != -1)
	{
		if(results[resPos] > nwords)
		{
			return -1;
		}
		wordIndex = results[resPos]; 
		node = (resList*)malloc(sizeof(resList));
		node->lineResCount = results[resPos + 1];
		node->results = malloc(node->lineResCount * sizeof(long));
		node->next = NULL;
		memcpy(node->results, &results[resPos + 2], node->lineResCount * sizeof(long));
		
		if(lineRes[wordIndex] == NULL)
		{
			lineRes[wordIndex] = node;
		}
		else
		{
			prevNode = NULL;
			currNode = lineRes[wordIndex];
			while(currNode != NULL)
			{
				if(currNode->results[0] > node->results[0])
				{
					node->next = currNode;
					if(prevNode == NULL)
					{
						lineRes[wordIndex] = node;
					}
					else
					{
						prevNode->next = node;
					}
					break;
				}
				prevNode = currNode;
				currNode = currNode->next;
			}
		}	
		resPos += 2 + node->lineResCount;		
	}
	
	return 0;
}

int processChunk(int startLine, char* chunkLines, char *keywords, int numKeywords, long **results, long *resTotalLongs)
{
	int i, j, resPos = 0, lineCount, recordStartPos;
	long numLongs = 10000;
	long *resBuff = (long*)malloc(numLongs*sizeof(long));
	
	for(i = 0; i < numKeywords; i++)
	{
		char *word = &keywords[i*MAX_KEYWORD_LENGTH];
		recordStartPos = resPos;
		resBuff[resPos] = i;
		lineCount = 0;
		for(j = 0; j < LINES_PER_CHUNK; j++)
		{
			char *line = &chunkLines[j*MAX_LINE_LENGTH];
			if(strstr(line,word) != NULL)
			{
				if(resPos >= numLongs - 5)
				{
					numLongs+=10000;
					resBuff = (long*)realloc(resBuff, numLongs*sizeof(long));
				}
				if(lineCount == 0)
				{
					resPos+=2;
				}
				lineCount++;
				resBuff[resPos++] = j+startLine;
			}
		}
		resBuff[recordStartPos+1] = lineCount;
	}
	resBuff[resPos++] = -1;
	
	*results = resBuff;
	*resTotalLongs = resPos;
	return 0;
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



