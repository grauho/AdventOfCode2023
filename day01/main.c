#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aocCommon.h"

int partOne(FILE *fhandle)
{
	AOC_BOOL is_first = AOC_TRUE;
	int running_sum   = 0;
	int first_digit   = 0;
	int second_digit  = 0;
	int tmp;

	while ((tmp = fgetc(fhandle)) != EOF)
	{
		if (tmp == '\n')
		{
			running_sum += (first_digit * 10) + second_digit;
			is_first = AOC_TRUE;
		}
		else if ((tmp >= '0') && (tmp <= '9'))
		{
			if (is_first == AOC_TRUE)
			{
				first_digit = tmp - 48;
				is_first = AOC_FALSE;
			}

			second_digit = tmp - 48;
		}
	}

	return running_sum;
}

static AOC_STAT parseForDigit(char *input, int *output)
{
	size_t i = 0;
	const char *digit_arr[] = 
	{
		"zero", "one", "two", "three", "four", "five", 
		"six", "seven", "eight", "nine"
	};
	const size_t digit_arr_len 
		= (sizeof(digit_arr)) / (sizeof(digit_arr[0]));

	for (i = 0; i < digit_arr_len; i++)
	{
		if (strncmp(input, digit_arr[i], strlen(digit_arr[i])) == 0)
		{
			*output = i;

			return AOC_SUCCESS;
		}
	}

	return AOC_FAILURE;
}

/* Quite inefficient due to having to parse out the written digits */
int partTwo(FILE *fhandle)
{
	AOC_BOOL is_first	= AOC_TRUE;
	char line_buffer[512]	= {0};
	int first_digit		= 0;
	int second_digit 	= 0;
	int running_sum 	= 0;

	while (fgets(line_buffer, 512, fhandle) != NULL)
	{
		char *tmp;

		is_first = AOC_TRUE;

		for (tmp = line_buffer; *tmp != '\0'; tmp++)
		{
			if ((*tmp >= '0') && (*tmp <= '9'))
			{
				if (is_first == AOC_TRUE)
				{
					first_digit = *tmp - 48;
					is_first = AOC_FALSE;
				}

				second_digit = *tmp - 48;
			}
			else if (((*tmp >= 'A') && (*tmp <= 'Z'))
			|| ((*tmp >= 'a') && (*tmp <= 'z')))
			{
				int parsed_digit;

				if (parseForDigit(tmp, &parsed_digit) 
					== AOC_SUCCESS)
				{
					if (is_first == AOC_TRUE)
					{
						first_digit = parsed_digit;
						is_first = AOC_FALSE;
					}

					second_digit = parsed_digit;
				}
			}
		}

		running_sum += (first_digit * 10) + second_digit;
	}

	return running_sum;
}

int main(int argc, char **argv)
{
	FILE *fhandle = NULL;

	if ((argc > 1)
	&& ((fhandle = fopen(argv[1], "rb")) != NULL))
	{
		fprintf(stdout, "1: Calibration sum: %d\n", partOne(fhandle));
		rewind(fhandle);
		fprintf(stdout, "2: Calibration sum: %d\n", partTwo(fhandle));
		fclose(fhandle);
	}

	return 0;
}

