#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/sysinfo.h>

uint8_t fEntropyTest_GrabBit(FILE *f, uint64_t position, uint64_t size)
{
    static uint8_t byte;
    static uint8_t count = 0;
    if (count % 8 == 0)
    {
        byte = fgetc(f);
    }
    count++;
    if (position / 8 >= size) return 0;
    fseek(f, position / 8, SEEK_SET);
    return (byte >> (8 - (position % 8) - 1)) & 0b1;
}

uint8_t EntropyTest_GrabBit(uint8_t *from, uint64_t position, uint64_t size)
{
    if (position / 8 >= size) return 0;
    return (from[position / 8] >> (8 - (position % 8) - 1)) & 0b1;
}

uint16_t EntropyTest_GrabBits(uint8_t *from, uint64_t position, uint64_t size, uint8_t count)
{
    uint16_t r = 0;
    for (uint16_t i = 0; i < count; i++)
    {
        r <<= 1;
        r |= EntropyTest_GrabBit(from, position + i, size);
    }
    return r;
}

uint16_t fEntropyTest_GrabBits(FILE *f, uint64_t position, uint64_t size, uint8_t count)
{
    uint16_t r = 0;
    for (uint16_t i = 0; i < count; i++)
    {
        r <<= 1;
        r |= fEntropyTest_GrabBit(f, position + i, size);
    }
    return r;
}

double fEntropyTest_Once(FILE *f, uint64_t size, uint8_t level)
{
    uint16_t symbols = 1 << level;
    uint64_t totals[symbols];
    for (uint16_t i = 0; i < symbols; i++)
    {
        totals[i] = 0;
    }
    for (uint64_t i = 0; i < size * 8; i += level)
    {
        uint16_t index = fEntropyTest_GrabBits(f, i, size, level);
        totals[index]++;
    }
    double score = 0;
    uint64_t s = size * 8;
    s = s % level != 0 ? s + (level - (s % level)) : s; 
    s = s / level;

    for (uint16_t i = 0; i < symbols; i++)
    {
        double p = (double)totals[i] / (double)s;
        if (p > 0) score += p * -(log(p) / log(2));
    }
    score /= level;
    return score;
}

double EntropyTest_Once(uint8_t *b, uint64_t size, uint8_t level)
{
    uint16_t symbols = 1 << level;
    uint64_t totals[symbols];
    for (uint16_t i = 0; i < symbols; i++)
    {
        totals[i] = 0;
    }
    for (uint64_t i = 0; i < size * 8; i += level)
    {
        uint16_t index = EntropyTest_GrabBits(b, i, size, level);
        totals[index]++;
    }
    double score = 0;
    uint64_t s = size * 8;
    s = s % level != 0 ? s + (level - (s % level)) : s; 
    s = s / level;

    for (uint16_t i = 0; i < symbols; i++)
    {
        double p = (double)totals[i] / (double)s;
        if (p > 0) score += p * -(log(p) / log(2));
    }
    score /= level;
    return score;
}

double fEntropyTest(FILE *f, uint64_t size, uint8_t confidence)
{
    double lowest = 1;
    for (uint8_t i = 1; i <= confidence; i++)
    {
        double pct = fEntropyTest_Once(f, size, i);
        if (pct < lowest) lowest = pct;
    }
    return lowest;
}

double EntropyTest(uint8_t *buffer, uint64_t size, uint8_t confidence)
{
    double lowest = 1;
    for (uint8_t i = 1; i <= confidence; i++)
    {
        double pct = EntropyTest_Once(buffer, size, i);
        if (pct < lowest) lowest = pct;
    }
    return lowest;
}

/*
    These are calculated from the formula: ( (2^(N + 5))N ) / 8
    It is based on the notion that a fair coin landing on a
    single side 32 times has a near 0% probability, and so in
    order to have high confidence in our data sample, we should
    have at least as many samples as are necessary for each
    symbol to appear at least 32 times.
*/
const uint64_t SIZES[] =
{
    8,
    32,
    96,
    256,
    640,
    1536,
    3584,
    8192,
    18432,
    40960,
    90112,
    196608,
    425984,
    917504,
    1966080,
//    4194304,
//    8912896,
//    18874368,
//    39845888,
//    83886080,
//    176160768,
//    369098752,
//    771751936,
//    1610612736,
//    3355443200,
//    6979321856,
//    14495514624,
//    30064771072,
//    62277025792,
//    128849018880,
//    266287972352,
//    549755813888
};

void err(uint8_t showInvalid)
{
    if (showInvalid)
    {
        fprintf(stderr, "nent: Invalid option.\n");
    }
    fprintf(stderr, "nent --  Calculate normalized Shannon entropy of file.\n");
    fprintf(stderr, "         Call with nent [options] [input-file]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "         Options:   -L Force a specific confidence level.\n");
}
uint64_t main(uint64_t argc, uint8_t **args)
{
    FILE *f;
    if (argc != 2 && argc != 4)
    {
        if (argc == 1)
        {
            err(0);
            return 0;
        }
        else
        {
            err(1);
            return 1;
        }
    }
    if (strcmp(args[1], "--help") == 0)
    {
        err(0);
        return 0;
    }

    uint8_t forceLevel = 0;
    if (argc == 4)
    {
        if (strcmp(args[1], "-L") == 0)
        {
            forceLevel = atoi(args[2]);
            f = fopen(args[3], "r");
        }
        else if (strcmp(args[2], "-L") == 0)
        {
            forceLevel = atoi(args[3]);
            f = fopen(args[1], "r'");
        }
        else
        {
            err(1);
            return 1;
        }
        if (forceLevel > sizeof(SIZES) / sizeof(uint64_t) + 1)
        {
            fprintf(stderr, "The specified confidence level is too high!\n");
            return 1;
        }
    }
    else
    {
        f = fopen(args[1], "r");
    }

    if (!f)
    {
        fprintf(stderr, "Failed to open the specified file.\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    uint64_t flen = ftell(f);
    rewind(f);
    if (flen < SIZES[0])
    {
        fclose(f);
        fprintf(stderr, "This file is too small!\n");
        return 1;
    }

    uint8_t i;
    uint8_t level = 0;
    if (forceLevel == 0)
    {
        for (i = 0; i < sizeof(SIZES) / sizeof(uint64_t); i++)
        {
            if (flen < SIZES[i])
            {
                break;
            }
            level++;
        }
    }
    else
    {
        level = forceLevel;
    }

    //If it takes up less than an 8th of our free RAM,
    //  just load the whole file into RAM since it's
    //  way faster. Otherwise, take the slow route.
    struct sysinfo si;
    sysinfo(&si);
    if (si.freeram / flen >= 8)
    {
        uint8_t *b = malloc(flen);
        fread(b, 1, flen, f);
        printf("Normalized Shannon Entropy = %.2f%% at a confidence level of %i.\n", EntropyTest(b, flen, level) * 100.0, level);
        free(b);
    }
    else
    {
        printf("%.2f%% %i\n", fEntropyTest(f, flen, level) * 100.0, level);
    }
    fclose(f);
}
