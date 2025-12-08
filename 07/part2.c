
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

long *memo = NULL;
void init_memo(void) {
    memo = malloc(line_length * line_count * sizeof(long));
    memset(memo, 0, line_length * line_count * sizeof(long));
}
#define MEMO(x,y) (memo[(y) * line_length + (x)])

long do_beam(int x, int y) {
    long splits = 0;
    
    if (memo == NULL) init_memo();

    if (y < line_count) {
        if (MEMO(x, y) != 0) {
            return MEMO(x, y);
        }
        char c = lines[y].line[x];
        //if (debug) printf("(%d, %d) = %c\n", x, y, c);
        if (c  == '^') {
            assert(lines[y].line[x - 1] != '^');
            assert(lines[y].line[x + 1] != '^');
            long left = do_beam(x - 1, y);
            long right = do_beam(x + 1, y);
            splits += (left == 0) ? 1 : left;
            splits += (right == 0) ? 1 : right;
            MEMO(x, y) = splits;
        } else if (c == '.') {
            splits = do_beam(x, y + 1);
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
    long splits = do_beam(cur_x, cur_y + 1);
    printf("Info: the calculated number of splits is %ld.\n", splits);

    printf("Info: the solution for the sample data should be %d\n", 40);
    printf("Info: the solution for the actual data should be %ld\n", 5748679033029L);
    return EXIT_SUCCESS;
}

