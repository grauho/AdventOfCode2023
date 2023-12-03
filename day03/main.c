#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

/* The length of the line buffers could be done dynamically but that is outside
 * of the purview of this particular puzzle solution */
#define LINE_MAX 2048 

#define IS_NODE_ACTIVE(node)			\
	(((node)->line[0] == '\0')		\
		? (AOC_FALSE) : (AOC_TRUE))

struct listNode
{
	char line[LINE_MAX];
	struct listNode *next;
	struct listNode *prev;
};

static AOC_STAT populateNode(struct listNode *node, FILE *fhandle)
{
	if ((node == NULL) || (fhandle == NULL))
	{
		return AOC_FAILURE;
	}

	if ((fgets(node->line, LINE_MAX, fhandle)) == NULL)
	{
		node->line[0] = '\0';
	}

	return AOC_SUCCESS;
}

/* I'm not sure exactly what AoC is considering a symbol, I am assuming
 * anything non-numeric and not a period but really this should have been
 * more explicit in the puzzle prompt. */
static AOC_BOOL isSymbol(const char glyph)
{
	if ((glyph == '.')
	|| (glyph == '\n') 
	|| (glyph == '\0') /* Should never come up with how fgets works */
	|| ((glyph >= '0') && (glyph <= '9')))
	{
		return AOC_FALSE;
	}

	return AOC_TRUE;
}

static AOC_BOOL isDigit(const char glyph)
{
	if ((glyph >= '0') && (glyph <= '9'))
	{
		return AOC_TRUE;
	}

	return AOC_FALSE;
}

static size_t seekDigit(char *line, size_t cursor)
{
	while ((line[cursor] != '\0') 
	&& (isDigit(line[cursor]) == AOC_FALSE))
	{
		cursor++;
	}

	return cursor;
}

static size_t seekGear(char *line, size_t cursor)
{
	while ((line[cursor] != '\0') 
	&& (line[cursor] != '*'))
	{
		cursor++;
	}

	return cursor;
}

static AOC_BOOL checkForSymbols(struct listNode *active, size_t i)
{
	if ((IS_NODE_ACTIVE(active->prev))
	&& (isSymbol(active->prev->line[i])))
	{
		return AOC_TRUE;
	}

	if (isSymbol(active->line[i])) /* By definition, active */
	{
		return AOC_TRUE;
	}

	if ((IS_NODE_ACTIVE(active->next))
	&& (isSymbol(active->next->line[i])))
	{
		return AOC_TRUE;
	}

	return AOC_FALSE;
}

static long int parseForNumbers(struct listNode *active)
{
	size_t cur = 0;
	long int line_sum = 0;

	for (cur = seekDigit(active->line, cur); active->line[cur] != '\0';
		cur = seekDigit(active->line, cur))
	{
		AOC_BOOL valid = AOC_FALSE;
		char tmp_buffer[32] = {0};
		char *buff_ptr = tmp_buffer;

		/* Check the characters immediately before the digit */
		if ((valid == AOC_FALSE) && (cur > 0))
		{
			valid = checkForSymbols(active, cur - 1);
		}

		/* Crawl the digit, checking other lines in situ */
		while (isDigit(active->line[cur]) == AOC_TRUE)
		{
			*buff_ptr++ = active->line[cur];	

			if (valid == AOC_FALSE)
			{
				valid = checkForSymbols(active, cur);
			}

			cur++;
		}

		/* Check the characters immediately after the digit, it's fine
		 * if it checks a null terminator */
		if (valid == AOC_FALSE)
		{
			valid = checkForSymbols(active, cur);
		}

		*buff_ptr = '\0';

		if (valid == AOC_TRUE)
		{
			line_sum += atol(tmp_buffer);	
			valid = AOC_FALSE;
		}
	}

	return line_sum;
}

static long int extractNumber(const char *line, const size_t cursor)
{
	size_t i = cursor;
	char tmp_buffer[32] = {0};
	char *buff_ptr = tmp_buffer;

	/* Backtrack until either the start of the line is reached or the 
	 * start of the number is found */
	for (i = cursor; (isDigit(line[i]) == AOC_TRUE); i--)
	{
		if (i == 0) break;
	}

	/* A bit wonky to account for when the start of the digit is also the 
	 * start of the line being processed */
	i += (isDigit(line[i]) == AOC_FALSE);

	while (isDigit(line[i]) == AOC_TRUE)
	{
		*buff_ptr++ = line[i++];	
	}

	*buff_ptr = '\0';

	return atol(tmp_buffer);
}

static int getNumberLocations(const char *line, const size_t cur, size_t *locs)
{
	AOC_BOOL in_number = AOC_FALSE;
	int num_locations = 0;
	size_t tmp = (cur > 0) ? cur - 1 : 0;

	for ( ; (tmp < cur + 2) && (line[tmp] != '\0'); tmp++)
	{
		if (isDigit(line[tmp]) == AOC_TRUE)
		{
			if (in_number == AOC_FALSE)
			{ 
				in_number = AOC_TRUE;
				locs[num_locations++] = tmp;
			}
		}
		else
		{
			in_number = AOC_FALSE;
		}
	}

	return num_locations;
}

static void processLine(const struct listNode *node, const size_t cur, 
			long int *arr_out, int *arr_i)
{
	size_t locations[2] = {0};
	int num_locs, i;

	if (IS_NODE_ACTIVE(node) == AOC_FALSE) 
	{
		return;
	}

	num_locs = getNumberLocations(node->line, cur, locations);

	for (i = 0; i < num_locs; i++)
	{
		arr_out[*arr_i % 2] = extractNumber(node->line, locations[i]);
		(*arr_i)++;
	}
}

static long int parseForGears(struct listNode *active)
{
	size_t cur = 0;
	long int line_sum = 0;

	for (cur = seekGear(active->line, cur); active->line[cur] != '\0';
		cur = seekGear(active->line, cur))
	{
		long int digits[2]  = {0};
		int num_digits = 0;

		processLine(active->prev, cur, digits, &num_digits);
		processLine(active, cur, digits, &num_digits);
		processLine(active->next, cur, digits, &num_digits);

		if (num_digits == 2)
		{
			line_sum += (digits[0] * digits[1]);
		}

		cur++;
	}

	return line_sum;
}

typedef long int (PARSE_FUNC)(struct listNode *active);

long int generalSolution(FILE *fhandle, PARSE_FUNC *parser)
{
	/* Will only ever need three because need only concern oneself with the
	 * actively parsing line, the previous line, and the next line */
	int i;
	long int running_sum = 0;
	struct listNode nodes[3] = {0};
	struct listNode *active  = &nodes[1];

	/* Set up the circularly doubly linked list */ 
	for (i = 0; i < 3; i++)
	{
		nodes[i].next = &nodes[(i + 1) % 3];

		if (i == 0)
		{
			nodes[i].prev = &nodes[2];
		}
		else
		{
			nodes[i].prev = &nodes[i - 1];
		}
	}

	/* Preload the first active line */
	if (fgets(active->line, LINE_MAX, fhandle) == NULL)
	{
		fprintf(stderr, "Failure to fgets in setup\n");

		return -1;
	}

	while ((populateNode(active->next, fhandle) != AOC_FAILURE)
	&& (IS_NODE_ACTIVE(active) != AOC_FALSE))
	{
		running_sum += parser(active);
		active = active->next;
	}

	return running_sum;
}

int main(int argc, char **argv)
{
	FILE *fhandle = NULL;

	if ((argc > 1)
	&& ((fhandle = fopen(argv[1], "rb")) != NULL))
	{
		fprintf(stdout, "Part 1) Engine part number sum: %ld\n",
			generalSolution(fhandle, parseForNumbers));
		rewind(fhandle);
		fprintf(stdout, "Part 2) Total gear ratio sum: %ld\n",
			generalSolution(fhandle, parseForGears));
		fclose(fhandle);
	}

	return 0;
}

