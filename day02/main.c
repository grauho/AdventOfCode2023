#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

enum
{
	RED   = 0,
	GREEN = 1,
	BLUE  = 2,
};

static char* getColorAndMag(char *in, char *color, int *mag)
{
	AOC_BOOL is_finished = AOC_FALSE;
	char tmp_buffer[5]   = {0};
	char *buff_ptr       = tmp_buffer;
	char *color_ptr      = color;

	while (*in != ' ') 
	{
		*buff_ptr++ = *in++;
	}

	in++;
	*mag = atol(tmp_buffer);

	while (is_finished == AOC_FALSE)
	{
		switch (*in)
		{
			case ',': /* fallthrough */
			case ';': /* fallthrough */
			case '\n':
				is_finished = AOC_TRUE;

				break;
			/* Shouldn't happen as fgets retains newline chars so
			 * if this does somehow happen it'll be treated as a 
			 * fatal error */
			case '\0': 
				return NULL;
			default:
				*color_ptr++ = *in++;

				break;
		}
	}

	*color_ptr = '\0';

	return in;
}

static AOC_BOOL validColors(int *colors, int r_lim, int g_lim, int b_lim)
{
	if ((colors == NULL)
	|| (colors[RED] > r_lim)
	|| (colors[GREEN] > g_lim)
	|| (colors[BLUE] > b_lim))
	{
		return AOC_FALSE;	
	}
	else
	{
		colors[RED]   = 0;
		colors[GREEN] = 0;
		colors[BLUE]  = 0;
	}

	return AOC_TRUE;
}

int partOne(FILE *fhandle, int r_lim, int g_lim, int b_lim)
{
	size_t line_i = 0;
	size_t running_score = 0;
	char line_buffer[1024] = {0};
	
	while (fgets(line_buffer, 1024, fhandle) != NULL)
	{
		int color_vals[3] = {0}; /* reset with each line */
		char *s_ptr = line_buffer;

		line_i++;

		while ((*s_ptr) != ':') 
		{
			s_ptr++;
		}

		s_ptr += 2;

		while ((*s_ptr) != '\n')
		{
			char color[7] = {0};
			int magnitude = 0;

			if ((s_ptr = getColorAndMag(s_ptr, color, &magnitude)) 
				== NULL)
			{
				fprintf(stderr, 
					"Fatal error parsing, dying\n");

				return -1;
			}

			if (strcmp(color, "red") == 0)
			{
				color_vals[RED] += magnitude;
			}
			else if (strcmp(color, "green") == 0)
			{
				color_vals[GREEN] += magnitude;
			}
			else if (strcmp(color, "blue") == 0)
			{
				color_vals[BLUE] += magnitude;
			}

			switch (*s_ptr)
			{
				case ';':
					/* Tally up, if impossible just 
					 * continue to the next line */

					s_ptr += 2;

					if (validColors(color_vals, r_lim, 
						g_lim, b_lim) == AOC_FALSE)
					{
						*s_ptr = '\n';	
					}

					break;
				case ',':
					s_ptr += 2;

					break;
				case '\n':
					if (validColors(color_vals, r_lim, 
						g_lim, b_lim) == AOC_TRUE)
					{
						running_score += line_i;
					}

					break;
				default:
					fprintf(stderr, 
						"Formatting error, "
						"encountered: %c\n", 
						*s_ptr);
					break;
			}
		}
	}

	return running_score;
}

long int partTwo(FILE *fhandle)
{
	size_t line_i = 0;
	long int running_power = 0;
	char line_buffer[1024] = {0};
	
	while (fgets(line_buffer, 1024, fhandle) != NULL)
	{
		int color_max[3] = {0}; /* reset with each line */
		char *s_ptr = line_buffer;

		line_i++;

		while ((*s_ptr) != ':') 
		{
			s_ptr++;
		}

		s_ptr += 2;

		while ((*s_ptr) != '\n')
		{
			char color[7] = {0};
			int magnitude = 0;

			if ((s_ptr = getColorAndMag(s_ptr, color, &magnitude)) 
				== NULL)
			{
				fprintf(stderr, 
					"Fatal error parsing, dying\n");

				return -1;
			}

			if ((strcmp(color, "red") == 0)
			&& (magnitude > color_max[RED]))
			{
				color_max[RED] = magnitude;
			}
			else if ((strcmp(color, "green") == 0)
			&& (magnitude > color_max[GREEN]))
			{
				color_max[GREEN] = magnitude;
			}
			else if ((strcmp(color, "blue") == 0)
			&& (magnitude > color_max[BLUE]))
			{
				color_max[BLUE] = magnitude;
			}

			switch (*s_ptr)
			{
				case ';': /* fallthrough */
				case ',':
					s_ptr += 2;

					break;
				case '\n':
					running_power += (color_max[RED] 
						* color_max[GREEN]
						* color_max[BLUE]);
							
					break;
				default:
					fprintf(stderr, 
						"Formatting error, "
						"encountered: %c\n", 
						*s_ptr);
					break;
			}
		}
	}

	return running_power;
}

int main(int argc, char **argv)
{
	FILE *fhandle = NULL;

	if ((argc > 1)
	&& ((fhandle = fopen(argv[1], "rb")) != NULL))
	{
		fprintf(stdout, "Part 1) Sum of possible game IDs: %d\n",
			partOne(fhandle, 12, 13, 14));
		rewind(fhandle);
		fprintf(stdout, "Part 2) Power of minimum cube sets: %ld\n",
			partTwo(fhandle));
		fclose(fhandle);
	}

	return 0;
}

