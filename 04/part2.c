
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
struct line {
    char *data;
} grid[200];
int width = 0;
int height = 0;

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

        if (line[0]) {
            // store the data
            int line_width = strlen(line);
            if (width == 0) {
                width = line_width;
            } else {
                assert(width == line_width);
            }
            grid[height].data = malloc(line_width + 1);
            strcpy(grid[height].data, line);
            height++;
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

struct dir { int dx, dy; } dirs[] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
int is_accessable(int x, int y) {
    int occupied_count = 0;
    for (unsigned d = 0; d < array_count(dirs); d++) {
        int newx = x + dirs[d].dx;
        int newy = y + dirs[d].dy;

        if ((newx < 0) || (newx >= width) || (newy < 0) || (newy >= height)) {
        } else {
            if ((grid[newy].data[newx] == '@') || (grid[newy].data[newx] == 'x')) {
                occupied_count += 1;
            }
        }
    }

    return occupied_count < 4;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    printf("Info: grid is %dx%d.\n", width, height);
    int total_removed = 0;
    int removed;
    do {
        removed = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if ((grid[y].data[x] == '@') && is_accessable(x, y)) {
                    // mark removable
                    grid[y].data[x] = 'x';
                    removed += 1;
                    if (debug) printf("x");
                } else {
                    if (debug) printf("%c", grid[y].data[x]);
                }
            }
            if (debug) printf("\n");
        }
        if (debug) printf("removed: %d\n", removed);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (grid[y].data[x] == 'x') grid[y].data[x] = '.';
            }
        }
        total_removed += removed;
    } while (removed != 0);
    printf("Info: the calcualted number of accesable rolls is %d.\n", total_removed);

    printf("Info: the solution for the sample data should be %d\n", 43);
    printf("Info: the solution for the actual data should be %d\n", 8722);
    return EXIT_SUCCESS;
}

