
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_FRESH_RANGES 500
struct range {
    long start, end;
} fresh_ranges[MAX_FRESH_RANGES];
int fresh_range_count = 0;
#define MAX_IDS 2000
long ids[MAX_IDS];
int id_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int ranges = 1;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (ranges) {
            if (strlen(line) == 0) {
                ranges = 0;
                continue;
            }
            long start, end;
            if (sscanf(line, "%ld-%ld", &start, &end) == 2) {
                // store a range
                fresh_ranges[fresh_range_count].start = start;
                fresh_ranges[fresh_range_count].end = end;
                fresh_range_count++;
            } else if (errno != 0) {
                perror("sscanf");
            } else {
                fprintf(stderr, "Unexpected input format '%s'.\n", line);
            }
        } else {
            long id;
            if (sscanf(line, "%ld", &id) == 1) {
                // store an id
                ids[id_count] = id;
                id_count++;
            } else if (errno != 0) {
                perror("sscanf");
            } else {
                fprintf(stderr, "Unexpected input format '%s'.\n", line);
            }
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int is_fresh(long id) {
    int fresh = 0;

    for (int i = 0; i < fresh_range_count; i++) {
        if ((id >= fresh_ranges[i].start) && (id <= fresh_ranges[i].end)) {
            fresh = 1;
            break;
        }
    }

    return fresh;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    printf("Info: there are %d ranges of fresh ids.\n", fresh_range_count);
    printf("Info: there are %d IDs to check\n", id_count);
    int fresh_count = 0;
    for (int i = 0; i < id_count; i++) {
        if (is_fresh(ids[i])) {
            fresh_count += 1;
        }
    }
    printf("Info: the number of calculated fresh ingredients are %d.\n", fresh_count);


    printf("Info: the solution for the sample data should be %d\n", 3);
    printf("Info: the solution for the actual data should be %d\n", 567);
    return EXIT_SUCCESS;
}

