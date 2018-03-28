#define _POSIX_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>

#include "twister.c"

/*
 * Have the OS generate a suitable random number for our seed. Since we are not
 * doing cryptography we don't need to block on `/dev/random' so it can
 * generate entropy. We use the non-blocking `/dev/urandom` for speed.
 */
uint32
random_seed ()
{
    static const char *random_device = "/dev/urandom";
    uint32 seed;
    FILE *fp;

    if ((fp = fopen(random_device, "r")) == NULL)
        goto error;
    if (fread(&seed, sizeof(uint32), 1, fp) < 0)
        goto error;
    fclose(fp);

    /* Mersenne Twister works best when seeded with odd numbers */
    return seed | 1;

error:
    perror(random_device);
    exit(1);
}

/*
 * A random number on a uniform distribution of (1..n), n being `num_buckets'.
 */
uint32
roll (int num_buckets)
{
    uint32 max, num;

    /* the maximum value that can be split into `num_buckets` evenly */
    max = UINT_MAX - (UINT_MAX % num_buckets);

    /* if `num' overflows the distributable range (`max') then try another */
    num = randomMT();
    while (num >= max)
        num = randomMT();

    /* put `num' into a bucket and add one to keep it in range */
    return 1 + num % num_buckets;
}

/*
 * Recursive Descent Parser
 */

static int LOOK = EOF - 1;

void
parse_error (const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    putc('\n', stderr);
    exit(1);
}

int
is_reserved (int c)
{
    return (c == '+' || c == 'd' || c == '\n');
}

int
nextc (FILE *input)
{
    int c;
    /* skip all whitespace and stop at reserved chars and non-whitespace */
    do {
        c = fgetc(input);    
    } while (!is_reserved(c) && isspace(c));
    LOOK = c;
    return LOOK;
}

int
look (FILE *input)
{
    if (LOOK == EOF - 1)
        return nextc(input);
    else
        return LOOK;
}

/*
 * <number> := [0-9]+
 */
int
parse_num (FILE *input)
{
    char num[12] = {0};
    int i = 0;

    if (!isdigit(look(input)))
        parse_error("Expected number got `%c' instead", look(input));

    do {
        if (i >= 11) /* need last character to be null terminator */
            parse_error("Overflowed the maximum integer size!");
        num[i++] = look(input);
        nextc(input);
    } while (isdigit(look(input)));

    return atoi(num);
}

/*
 * <roll> := <number> | <number>d<number> [+ <roll>]
 */
int
parse_roll (FILE *input)
{
    int rolls, die_size;
    int total;
    int i;

    rolls = parse_num(input);

    /* if there's no 'd' then this is probably addition */
    if (look(input) != 'd') {
#ifdef SHOW_ROLLS
        printf("%d (bonus)", rolls);
#endif
        return rolls;
    }
    nextc(input);

    die_size = parse_num(input);

    for (i = 0, total = 0; i < rolls; i++)
        total += roll(die_size);

#ifdef SHOW_ROLLS
    printf("%d (%dd%d)", total, rolls, die_size);
#endif

    /* we can chain dice rolls, e.g 2d6 slashing + 1d4 posion damage */
    if (look(input) == '+') {
        nextc(input);
#ifdef SHOW_ROLLS
        printf(" + ");
#endif
        total += parse_roll(input);
    }

    return total;
}

void
parse (FILE *input)
{
#ifdef SHOW_ROLLS
    printf(" => %d\n", parse_roll(input)); /* outside-in execution */
#else
    printf("%d\n", parse_roll(input));
#endif
}

/*
 * Pipe all command line arguments to a pipe and return the read end of that
 * pipe. This lets us pass a FILE* to the rest of the program even when parsing
 * from argv.
 */
FILE *
pipe_argv (char *arg)
{
    /* [1] is write end, [0] is read end */
    int fd[2];

    if (pipe(fd) != 0)
        goto error;

    if (write(fd[1], arg, strlen(arg) + 1) < 0)
        goto error;

    /* if we don't close the write end, program will hang */
    close(fd[1]);

    /* if `pipe' didn't fail, `fdopen' won't either */
    return fdopen(fd[0], "r");

error:
    perror(NULL);
    exit(1);
}

int
main (int argc, char **argv)
{
    FILE *input;

    seedMT(random_seed());

    if (argc > 1)
        input = pipe_argv(argv[1]);
    else
        input = stdin;

    parse(input);
    fclose(input);
    return 0;
}
