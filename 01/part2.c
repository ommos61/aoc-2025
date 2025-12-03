
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
#define MAX_ROTATIONS 10240
struct rotation {
    char direction;
    int count;
} rotations[MAX_ROTATIONS];
int rotation_count = 0;

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

        char c = line[0];
        int value = 0;
        if (sscanf(line + 1, "%d", &value) == 1) {
            // store the data
            rotations[rotation_count].direction = c;
            rotations[rotation_count].count = value;
            rotation_count += 1;
            assert(rotation_count < MAX_ROTATIONS);
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

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    int current = 50;
    int zero_count = 0;
    for (int i = 0; i < rotation_count; i++) {
        int add = (rotations[i].direction == 'L') ? -rotations[i].count : rotations[i].count;
        int new = current + add;
        if ((current == 0) && (new < 0)) {
            new += 100;
        }
        while (new < 0) {
            new += 100;
            zero_count += 1;
        }
        if (new == 0) {
            zero_count += 1;
        } else {
            while (new > 99) {
                new -= 100;
                zero_count += 1;
            }
        }
        // printf("%c%-4d => %2d (zero count = %d)\n", rotations[i].direction, rotations[i].count, new, zero_count);
        current = new;
        assert((current >= 0) && (current <= 99));
    }
    printf("Info: the calculated solution is %d\n", zero_count);

    printf("Info: the solution for the sample data should be %d\n", 6);
    printf("Info: the solution for the actual data should be %d\n", 6700);
    return EXIT_SUCCESS;
}

