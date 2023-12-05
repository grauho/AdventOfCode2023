#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

#define BASE_HOPPER_SIZE 5
#define BASE_NODE_LEN	 5
#define LINE_MAX 	 1024

struct treeNode
{
	unsigned int value;
	struct treeNode *left;
	struct treeNode *right;
};

struct nodeHopper
{
	struct treeNode *nodes;
	size_t avail_nodes;
	size_t used_nodes;
};

struct nodeHopper* newHopper(const size_t initial_size)
{
	struct nodeHopper *ret = NULL;
	size_t arr_len = (initial_size < 2) ? 2 : initial_size;

	if ((ret = malloc(sizeof(struct nodeHopper))) == NULL)
	{
		fprintf(stderr, "Hopper malloc error\n");

		return NULL;
	}

	if ((ret->nodes = malloc(sizeof(struct treeNode) * arr_len)) == NULL)
	{
		fprintf(stderr, "Hopper node array malloc error\n");
		free(ret);

		return NULL;
	}

	ret->avail_nodes = arr_len;
	ret->used_nodes  = 0;

	return ret;
}

static void freeHopper(struct nodeHopper *hopper)
{
	if (hopper != NULL)
	{
		if (hopper->nodes != NULL)
		{
			free(hopper->nodes);
		}

		free(hopper);
	}
}

static AOC_STAT loadHopper(struct nodeHopper *hopper, const unsigned int value)
{
	struct treeNode *new_node = NULL;

	if (hopper == NULL)
	{
		return AOC_FAILURE;;
	}

	if (hopper->avail_nodes <= hopper->used_nodes)
	{
		size_t new_len = hopper->avail_nodes * 3 / 2;
		struct treeNode *tmp_arr;

		if (new_len < 2) new_len = 2;

		tmp_arr = realloc(hopper->nodes, 
			sizeof(struct treeNode) * new_len);

		if (tmp_arr == NULL)
		{
			fprintf(stderr, "Load hopper realloc error\n");

			return AOC_FAILURE;
		}

		hopper->nodes = tmp_arr;
		hopper->avail_nodes = new_len;
	}

	new_node = &hopper->nodes[hopper->used_nodes++];
	new_node->left  = NULL;
	new_node->right = NULL;
	new_node->value = value;

	return AOC_SUCCESS;
}

static size_t seekGlyph(const char *line, const char glyph, size_t cur)
{
	while ((line[cur] != glyph) && (line[cur] != '\0'))
	{
		cur++;
	}

	return cur;
}

static char* getNextChunk(char *out, const char *line, size_t *cur)
{
	size_t i = 0;

	/* Skip any leading whitespace */
	while (line[*cur] == ' ') (*cur)++;

	while ((line[*cur] != '\0') && (line[*cur] != ' '))
	{
		out[i++] = line[(*cur)++];
	}

	out[i] = '\0';

	return (i == 0) ? NULL : out;
}

static struct treeNode* addNode(struct treeNode *tree, struct treeNode *node)
{
	struct treeNode *tmp = tree;

	if (tree == NULL)
	{
		tree = node;

		return tree;
	}

	for (;;)
	{
		if (tmp->value < node->value)
		{
			if (tmp->left == NULL)
			{
				tmp->left = node;

				break;
			}

			tmp = tmp->left;
		}
		else
		{
			if (tmp->right == NULL)
			{
				tmp->right = node;

				break;
			}

			tmp = tmp->right;
		}
	}

	return tree;
}

static struct treeNode* findNode(struct treeNode *tree, const unsigned int val)
{
	struct treeNode *found = tree;
	
	while ((found != NULL) && (found->value != val))
	{
		if (found->value < val)
		{
			found = found->left;
		}
		else
		{
			found = found->right;
		}
	}

	return found;
}

static struct treeNode* buildTree(struct nodeHopper *hopper)
{
	struct treeNode *tree = NULL;
	size_t i;

	for (i = 0; i < hopper->used_nodes; i++)
	{
		tree = addNode(tree, &hopper->nodes[i]);
	}

	return tree;
}

static unsigned long int processLine(const char *line, 
	struct nodeHopper *hopper)
{
	unsigned long int num_matches  = 0;
	AOC_BOOL is_loading   = AOC_TRUE;
	struct treeNode *tree = NULL;
	char tmp[5] = {0};
	size_t curs = 0;

	curs = seekGlyph(line, ':', curs);

	if (line[curs] == '\0')
	{
		fprintf(stderr, "Invalid line: %s\n", line);

		return 0;
	}

	/* Really could have just used sscanf here in retrospect */
	while (getNextChunk(tmp, line, &curs) != NULL)
	{
		if (tmp[0] == '|')
		{
			is_loading = AOC_FALSE;
			tree = buildTree(hopper);
		}
		else if (tmp[0] == ':')
		{
			continue;
		}
		else
		{
			unsigned int val = (unsigned int) atol(tmp);

			if (is_loading == AOC_TRUE)
			{
				if (loadHopper(hopper, val) == AOC_FAILURE)
				{
					fprintf(stderr, 
						"Hopper realloc error\n");

					return 0;
				}
			}
			else if (findNode(tree, val) != NULL)
			{
				num_matches++;
			}
		}
	}

	return num_matches;
}

unsigned long int calcScore(const unsigned long int matches)
{
	unsigned long int i;
	unsigned long int score = 0;

	for (i = 0; i < matches; i++)
	{
		if (score == 0)
		{
			score++;
		}
		else
		{
			score *= 2;
		}
	}

	return score;
}

unsigned long int partOne(FILE *fhandle)
{
	struct nodeHopper *hopper = NULL;
	unsigned long int running_sum = 0;
	char line[LINE_MAX] = {0};

	if ((hopper = newHopper(BASE_HOPPER_SIZE)) == NULL)
	{
		fprintf(stderr, "Allocation failure\n");

		return 0;
	}

	while (fgets(line, LINE_MAX, fhandle) != NULL)
	{
		unsigned long int matches = processLine(line, hopper);
		unsigned long int line_score = calcScore(matches);

		running_sum += line_score;
		hopper->used_nodes = 0;
	}

	freeHopper(hopper);

	return running_sum;
}

/* Bear in mind we're now just calculating the total number of cards and
 * no longer care about the score total, I'll admit I got lazy with the 
 * repeat table. It really should be something like an expandable ring buffer 
 * or have some kind of shift happen when it reaches it's line limit */
unsigned long int partTwo(FILE *fhandle)
{
	struct nodeHopper *hopper  	    = NULL;
	unsigned int repeat_table[LINE_MAX] = {0};
	char line[LINE_MAX]        	    = {0};
	unsigned long int line_i	    = 0;
	unsigned long int running_sum 	    = 0;

	if ((hopper = newHopper(BASE_HOPPER_SIZE)) == NULL)
	{
		fprintf(stderr, "Allocation failure\n");

		return 0;
	}

	while (fgets(line, LINE_MAX, fhandle) != NULL)
	{
		const unsigned long int matches = processLine(line, hopper);
		unsigned long int i;

		/* Update the repeat table */
		for (i = 1; i <= matches; i++)
		{
			repeat_table[line_i + i] += repeat_table[line_i] + 1;
		}

		running_sum += repeat_table[line_i] + 1;
		hopper->used_nodes = 0;
		line_i++;
	}

	freeHopper(hopper);

	return running_sum;
}

int main(int argc, char **argv)
{
	FILE *fhandle = NULL;

	if ((argc > 1)
	&& ((fhandle = fopen(argv[1], "rb")) != NULL))
	{
		fprintf(stdout, "Part 1) Total points: %ld\n", 
			partOne(fhandle));
		rewind(fhandle);
		fprintf(stdout, "Part 2) Total points: %ld\n",
			partTwo(fhandle));
		fclose(fhandle);
	}

	return 0;
}

