
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_BANKS 1024
struct bank {
    char *batteries;
} banks[MAX_BANKS];
int bank_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (isdigit(line[0])) {
            // parse the data
            banks[bank_count].batteries = malloc(strlen(line) + 1);
            strcpy(banks[bank_count].batteries, line);
            bank_count++;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

#define DIGIT_COUNT 12
long max_joltage(char *batteries) {
    int max[DIGIT_COUNT] = {0};
    int max_loc = 0;

    for (int digit = 0; digit < DIGIT_COUNT; digit++) {
        for (unsigned i = max_loc; i < strlen(batteries) - DIGIT_COUNT + digit + 1; i++) {
            int val = batteries[i] - '0';
            if (val > max[digit]) {
                max[digit] = val;
                max_loc = i + 1;
                if (debug) printf("max = %d, remaining '%s'\n", val, batteries + max_loc);
            }
        }
    }

    long value = 0;
    for (int i = 0; i < DIGIT_COUNT; i++) {
        value = value * 10 + max[i];
    }
    return value;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    printf("Info: there are %d battery banks.\n", bank_count);
    long sum = 0;
    for (int i = 0; i < bank_count; i++) {
        long maxval = max_joltage(banks[i].batteries);
        if (debug) printf("max = %ld\n", maxval);
        sum += maxval;
    }
    printf("Info: the calculated sum of joltage is %ld.\n", sum);

    printf("Info: the solution for the sample data should be %ld\n", 3121910778619L);
    printf("Info: the solution for the actual data should be %ld\n", 169709990062889L);
    return EXIT_SUCCESS;
}

