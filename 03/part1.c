
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output
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

int max_joltage(char *batteries) {
    int max_first = 0;
    int max_second = 0;

    for (unsigned i = 0; i < strlen(batteries) - 1; i++) {
        max_first = MAX(max_first, batteries[i] - '0');
    }
    for (unsigned i = strchr(batteries, '0' + max_first) - batteries + 1; i < strlen(batteries); i++) {
        max_second = MAX(max_second, batteries[i] - '0');
    }
    return max_first * 10 + max_second;
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
    int sum = 0;
    for (int i = 0; i < bank_count; i++) {
        sum += max_joltage(banks[i].batteries);
    }
    printf("Info: the calculated sum of joltage is %d.\n", sum);

    printf("Info: the solution for the sample data should be %d\n", 357);
    printf("Info: the solution for the actual data should be %d\n", 17113);
    return EXIT_SUCCESS;
}

