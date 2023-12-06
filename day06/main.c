#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

#define LINE_MAX 1024

static size_t seekDigit(const char *line, size_t curs)
{
	while ((line[curs] != '\0')
	&& ((line[curs] < '0') || (line[curs] > '9')))
	{
		curs++;
	}

	return curs;
}

static size_t seekSpace(const char *line, size_t curs)
{
	while ((line[curs] != '\0')
	&& (line[curs] != ' ') 
	&& (line[curs] != '\t'))
	{
		curs++;
	}

	return curs;
}

static long int* extractLineValues(const char *line, size_t *len)
{
	long int *values = NULL;
	long int tmp     = 0;
	size_t avail     = 0;
	size_t offset    = 0;

	offset = seekDigit(line, offset);

	while ((line[offset] != '\0')
	&& (sscanf(line + offset, "%ld ", &tmp) == 1))
	{
		if (*len == avail)
		{
			long int *arr = NULL;

			avail = (avail == 0) ? 2 : avail * 3 / 2;
			arr = realloc(values, avail * sizeof(long int));

			if (arr == NULL)
			{
				return NULL;
			}

			values = arr;
		}

		values[(*len)++] = tmp;
		offset = seekSpace(line, offset);
		offset = seekDigit(line, offset);
	}

	return values;
}

typedef void (PRE_FUNC)(char *line);

static long int* getValues(FILE *fhandle, size_t *len, const char *name, 
	PRE_FUNC *preprocess)
{
	char buffer[LINE_MAX] = {0};	
	long int *vals = NULL;

	*len = 0;

	if ((fgets(buffer, LINE_MAX, fhandle) != NULL)
	&& (strncmp(buffer, name, strlen(name)) == 0))
	{
		if (preprocess != NULL)
		{
			preprocess(buffer);
		}

		vals = extractLineValues(buffer, len);
	}

	return vals;
}

/* Formula ends up being:
 * 	Distance = velocity * time_running
 * 	where: velocity     = time_held
 * 	where: time_running = time_total - time_held
 * And it suffices to find the bounds of where this becomes true because all
 * values contained in that range will also satisfy this condition 
 */
static AOC_BOOL isWinner(const long int time, const long int distance, 
	const long int held)
{
	return ((held * (time - held)) > distance) ? AOC_TRUE : AOC_FALSE;
}

static void getWinBound(const long int time, const long int dist, 
	long int *min, long int *max)
{
	for (*min = 0; 
		(*min <= time) && (isWinner(time, dist, *min) == AOC_FALSE); )
	{
		(*min)++;
	}

	for (*max = time; 
		(*max >= *min) && (isWinner(time, dist, *max) == AOC_FALSE); )
	{
		(*max)--;
	}
}

long int generalSolution(FILE *fhandle, PRE_FUNC *prepro)
{
	long int ret = 1;
	size_t i, num_times;
	long int *times = getValues(fhandle, &num_times, "Time:", prepro);
	long int *dists = getValues(fhandle, &num_times, "Distance:", prepro);

	if ((times == NULL) || (dists == NULL))
	{
		fprintf(stderr, "Error parsing input file\n");

		ret = -1;
	}
	else
	{
		long int min, max;

		for (i = 0; i < num_times; i++)
		{
			getWinBound(times[i], dists[i], &min, &max);
			ret *= (max - min + 1);
		}
	}

	if (times != NULL) 
	{
		free(times);
	}

	if (dists != NULL) 
	{
		free(dists);
	}

	return ret;
}

static void stripWhitespace(char *line)
{
	char *output = line;
	size_t i, j;

	for (i = 0, j = 0; line[i] != '\0'; i++, j++)
	{
		if (line[i] != ' ')
		{
			output[j] = line[i];
		}
		else
		{
			j--;
		}
	}

	output[j] = '\0';
}

int main(int argc, char **argv)
{
	FILE *fhandle = NULL;

	if ((argc > 1)
	&& ((fhandle = fopen(argv[1], "rb")) != NULL))
	{
		fprintf(stdout, 
			"Part 1) Product of number of ways to win: %ld\n",
			generalSolution(fhandle, NULL));
		rewind(fhandle);
		fprintf(stdout, 
			"Part 2) Product of number of ways to win: %ld\n",
			generalSolution(fhandle, stripWhitespace));
		fclose(fhandle);
	}

	return 0;
}

