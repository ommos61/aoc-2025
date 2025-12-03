
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
struct range {
    char *start;
    char *end;
} ranges[200];
int range_count = 0;

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

        char number[50];
        char *p = line;
        while (*p) {
            // parse the data
            int i = 0;
            while (isdigit(*p)) {
                number[i++] = *p;
                p++;
            }
            number[i] = 0;
            ranges[range_count].start = malloc(strlen(number) + 1);
            strcpy(ranges[range_count].start, number);

            assert(*p++ == '-');
            i = 0;
            while (isdigit(*p)) {
                number[i++] = *p;
                p++;
            }
            number[i] = 0;
            ranges[range_count].end = malloc(strlen(number) + 1);
            strcpy(ranges[range_count].end, number);

            assert((*p == 0) || (*p++ == ',') );
            range_count++;
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int check_valid(long value) {
    int valid = 1;
    char number[50];
    sprintf(number, "%ld", value);
    if ((strlen(number) % 2) == 0) {
        int half_len = strlen(number) / 2;
        if (strncmp(number, number + half_len, half_len) == 0) {
            valid = 0;
        }
    }

    return valid;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    long sum = 0L;
    printf("There are %d ranges.\n", range_count);
    for (int i = 0; i < range_count; i++) {
        if (debug) printf("%s-%s\n", ranges[i].start, ranges[i].end);
        long start = strtol(ranges[i].start, NULL, 10);
        long end = strtol(ranges[i].end, NULL, 10);
        for (long l = start; l <= end; l++) {
            if (!check_valid(l)) {
                if (debug) printf("Invalid ID: %ld\n", l);
                sum += l;
            }
        }
    }
    printf("The calculated sum of invalid IDs is %ld.\n", sum);

    printf("Info: the solution for the sample data should be %ld\n", 1227775554L);
    printf("Info: the solution for the actual data should be %ld\n", 13108371860L);
    return EXIT_SUCCESS;
}

