
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
int debug = 0; // 0 = no debug output, 1 = some extra debug output
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

int is_empty(int range) {
    return (fresh_ranges[range].end == -1);
}

long range_size(int range) {
    if (is_empty(range)) {
        return 0L;
    }
    return fresh_ranges[range].end - fresh_ranges[range].start + 1;
}

int is_inside(long id, int range) {
    int inside = 0;

    long start = fresh_ranges[range].start;
    long end = fresh_ranges[range].end;

    if ((id >= start) && (id <= end)) {
        inside = 1;
    }

    return inside;
}

// fix ranges until 'range' so they don't overlay
// NOTE: when a range is completely overlapped, it is replaced by {0, -1}
void fix_ranges(int range) {
    struct range *r = fresh_ranges;
    for (int i = 0; i < range; i++) {
        int range1 = i;
        int range2 = range;
        long range1_size = range_size(range1);
        long range2_size = range_size(range2);

        // if one of the ranges is empty
        if (is_empty(range1) || is_empty(range2)) {
            continue;
        }

        // swap, so range1 is always the smallest
        if (range1_size > range2_size) {
            int temp = range1; range1 = range2; range2 = temp;
            range1_size = range_size(range1);
            range2_size = range_size(range2);
        }

        if (is_inside(r[range1].start, range2) && is_inside(r[range1].end, range2)) {
            // range1 is completely inside range2, make range1 empty
            r[range1].start = 0; r[range1].end = -1;
        } else {
            if (is_inside(r[range1].start, range2)) {
                r[range1].start = r[range2].end + 1;
            }
            if (is_inside(r[range1].end, range2)) {
                r[range1].end = r[range2].start - 1;
            }
        }
    }
}

void print_ranges(void) {
    if (debug) {
        printf("Ranges:\n");
        for (int i = 0; i < fresh_range_count; i++) {
            if (is_empty(i)) {
                printf("<>\n");
            } else {
                printf("<%ld, %ld>\n", fresh_ranges[i].start, fresh_ranges[i].end);
            }
        }
        printf("\n");
    }
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
    
    // adjust the ranges so they doesn't overlap
    print_ranges();
    for (int i = 0; i < fresh_range_count; i++) {
        fix_ranges(i);
        print_ranges();
    }

    long fresh_count = 0;
    for (int i = 0; i < fresh_range_count; i++) {
        fresh_count += range_size(i);
    }
    printf("Info: the number of calculated fresh ingredients are %ld.\n", fresh_count);


    printf("Info: the solution for the sample data should be %ld\n", 14L);
    printf("Info: the solution for the actual data should be %ld\n", 354149806372909L);
    return EXIT_SUCCESS;
}

