
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
#define MAX_LINES 1000
struct map_line {
    char *line;
} lines[MAX_LINES];
int line_count = 0;
unsigned line_length = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            lines[line_count].line = malloc(strlen(line) + 1);
            strcpy(lines[line_count].line, line);
            if (line_length == 0) {
                line_length = strlen(line);
            } else {
                assert(line_length == strlen(line));
            }
            line_count++;
        }
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int do_beam(int x, int y) {
    int splits = 0;
    
    if (y + 1 < line_count) {
        char c = lines[y + 1].line[x];
        if (debug) printf("(%d, %d) = %c\n", x, y + 1, c);
        if (c  == '^') {
            splits += 1;
            lines[y + 1].line[x - 1] = '|';
            splits += do_beam(x - 1, y + 1);
            lines[y + 1].line[x + 1] = '|';
            splits += do_beam(x + 1, y + 1);
        } else if (c == '.') {
            lines[y + 1].line[x] = '|';
            splits += do_beam(x, y + 1);
        }
    }

    return splits;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    // find start
    int cur_y = 0;
    int cur_x = 0;
    for (unsigned i = 0; i < line_length; i++) {
        if (lines[cur_y].line[i] == 'S') {
            cur_x = i;
            break;
        }
    }
    printf("start is (%d, %d).\n", cur_x, cur_y);
    int splits = do_beam(cur_x, cur_y);
    printf("Info: the calculated number of splits is %d.\n", splits);

    printf("Info: the solution for the sample data should be %d\n", 21);
    printf("Info: the solution for the actual data should be %d\n", 1587);
    return EXIT_SUCCESS;
}

