#include <stdio.h>
#include <stdlib.h>

#include "../aocCommon.h"

#define LINE_MAX 1024
#define CARDS_PER_HAND 5

enum strength 
{
	HIGH_CARD = 0,		/* No values above 1 */
	ONE_PAIR,		/* One value above 1 & no other value above 1*/
	TWO_PAIR,		/* Two values above 1 */
	THREE_OF_A_KIND,	/* One value above 2 & no other value above 1*/
	FULL_HOUSE,		/* One value above 2 & one value above 1 */
	FOUR_OF_A_KIND,		/* One value above 3 */
	FIVE_OF_A_KIND,		/* One value above 4 */
	INVALID_HAND	
};

struct cardHand
{
	char cards[CARDS_PER_HAND + 1];
	enum strength worth;
	int bid;
};

const char card_values[]  = "23456789TJQKA";
const char joker_values[] = "J23456789TQKA";
const size_t num_values = (sizeof(card_values) / sizeof(card_values[0]));

static void printHand(struct cardHand *hand)
{
	const char *names[] = 
	{
		"High Card",
		"One Pair",
		"Two Pair",
		"Three of a Kind",
		"Full House",
		"Four of a Kind",
		"Five of a Kind",
		"INVALID HAND"
	};

	fprintf(stdout, "Hand: %s Worth: %-16s Bid: %d\n", 
		hand->cards, names[hand->worth], hand->bid);
}

/* Unused, but it does print the values rather nicely */
static void dumpHands(struct cardHand *hands, size_t num_hands)
{
	size_t i;

	for (i = 0; i < num_hands; i++)
	{
		fprintf(stdout, "%-4ld: ", i + 1);
		printHand(&hands[i]);
	}
}

static enum strength getHandStrength(const char *cards, AOC_BOOL use_joker)
{
	short int value_table[13] = {0};
	enum strength hand_stg = HIGH_CARD;
	size_t i, j; 
	int high = -1;
	int prev = -1;
	int tmp;
	const char *lookup_table 
		= (use_joker == AOC_TRUE) ? joker_values : card_values;

	/* assess each card in the hand */
	for (i = 0; i < CARDS_PER_HAND; i++)
	{
		for (j = 0; (j < num_values) && (lookup_table[j] != cards[i]); )
		{
			j++;
		}

		if (j == num_values) 
		{
			fprintf(stderr, "Something is awry\n");

			return INVALID_HAND;
		}

		/* Increase the value tables entry for that card */
		value_table[j]++;

		if (high == -1)
		{
			high = j;
		}
		else if (j != high) 
		{
			if (value_table[j] >= value_table[high])
			{
				prev = high;
				high = j;
			}
			else if (value_table[j] >= value_table[prev])
			{
				prev = j;
			}
		}
	}

	if (use_joker == AOC_TRUE)
	{
		if (high != 0)
		{
			tmp = value_table[high] + value_table[0];
		}
		else
		{
			if (prev != 0)
			{
				tmp = value_table[prev] + value_table[0];
			}
			else
			{
				tmp = value_table[prev];
			}
		}
	}
	else
	{
		tmp = value_table[high];
	}

	switch (tmp)
	{
		case 5:
			hand_stg = FIVE_OF_A_KIND;
			break;
		case 4:
			hand_stg = FOUR_OF_A_KIND;
			break;
		case 3:
			hand_stg = (value_table[prev] > 1) 
				? FULL_HOUSE : THREE_OF_A_KIND;
			break;
		case 2:
			hand_stg = (value_table[prev] > 1) 
				? TWO_PAIR : ONE_PAIR;
			break;
		case 1:
			hand_stg = HIGH_CARD;
			break;
		default: /* Should be unreachable */
			hand_stg = INVALID_HAND;
			break;
	}

	return hand_stg;
}

static struct cardHand* readHandsFromFile(FILE *fhandle, size_t *len, 
	AOC_BOOL use_jokers)
{
	struct cardHand *hands = NULL;
	char line_buffer[LINE_MAX] = {0};
	size_t avail = 0;

	*len = 0;

	while (fgets(line_buffer, LINE_MAX, fhandle) != NULL)
	{
		if (avail == *len)
		{
			struct cardHand *tmp;
			avail = (avail == 0) ? 2 : avail * 3 / 2;

			tmp = realloc(hands, avail * sizeof(struct cardHand));

			if (tmp == NULL)
			{
				if (hands != NULL)
				{
					free(hands);
				}

				return NULL;
			}

			hands = tmp;
		}

		if (sscanf(line_buffer, "%s %d", hands[(*len)].cards, 
			&hands[(*len)].bid) != 2)
		{
			if (hands != NULL)
			{
				free(hands);
			}

			return NULL;
		}

		hands[(*len)].worth 
			= getHandStrength(hands[(*len)].cards, use_jokers);

		if (hands[(*len)].worth == INVALID_HAND)
		{
			fprintf(stderr, "%s is not a valid hand\n", 
				hands[(*len)].cards);

			if (hands != NULL)
			{
				free(hands);
			}

			return NULL;
		}

		(*len)++;
	}

	return hands;
}

/* Annoying to have two versions of essentially the same function save for 
 * using a different lookup table but due to needing to be able to pass the
 * function pointer into qsort there isn't a convenient way around it that 
 * doesn't end up being equally messy */
static int cardCmp(const char a, const char b)
{
	size_t i;
	size_t val_a = 0;
	size_t val_b = 0;

	for (i = 0; i < num_values; i++)
	{
		if (card_values[i] == a) val_a = i;
		if (card_values[i] == b) val_b = i;
	}

	return val_a - val_b;
}

static int jokerCardCmp(const char a, const char b)
{
	size_t i;
	size_t val_a = 0;
	size_t val_b = 0;

	for (i = 0; i < num_values; i++)
	{
		if (joker_values[i] == a) val_a = i;
		if (joker_values[i] == b) val_b = i;
	}

	return val_a - val_b;
}

int handCmp(const void *hand_a, const void *hand_b)
{
	const struct cardHand *left  = (struct cardHand *) hand_a;
	const struct cardHand *right = (struct cardHand *) hand_b;

	if (left->worth == right->worth)
	{
		size_t i;

		for (i = 0; i < CARDS_PER_HAND; i++)
		{
			if (left->cards[i] != right->cards[i])
			{
				return cardCmp(left->cards[i], 
					right->cards[i]);
			}
		}
	}

	return left->worth - right->worth;
}

int jokerCmp(const void *hand_a, const void *hand_b)
{
	const struct cardHand *left  = (struct cardHand *) hand_a;
	const struct cardHand *right = (struct cardHand *) hand_b;

	if (left->worth == right->worth)
	{
		size_t i;

		for (i = 0; i < CARDS_PER_HAND; i++)
		{
			if (left->cards[i] != right->cards[i])
			{
				return jokerCardCmp(left->cards[i], 
					right->cards[i]);
			}
		}
	}

	return left->worth - right->worth;
}

unsigned long int calculateScore(FILE *fhandle, AOC_BOOL enable_jokers)
{
	struct cardHand *hands = NULL;
	size_t i, num_hands = 0;
	unsigned long int running_score = 0;

	if ((hands = readHandsFromFile(fhandle, &num_hands, enable_jokers))
		!= NULL)
	{
		qsort(hands, num_hands, sizeof(struct cardHand), 
			(enable_jokers == AOC_TRUE) ? jokerCmp : handCmp);

		for (i = 0; i < num_hands; i++)
		{
			running_score += (hands[i].bid * (i + 1));
		}

		free(hands);
	}

	return running_score;
}

int main(int argc, char **argv)
{
	FILE *fhandle = NULL;

	if ((argc > 1)
	&& ((fhandle = fopen(argv[1], "rb")) != NULL))
	{
		fprintf(stdout, "Part 1) Sum of hand scores: %lu\n",
			calculateScore(fhandle, AOC_FALSE));
		rewind(fhandle);
		fprintf(stdout, 
			"Part 2) Sum of hand scores considering jokers: %lu\n", 
			calculateScore(fhandle, AOC_TRUE));
		fclose(fhandle);
	}

	return 0;
}

