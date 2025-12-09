
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
#define MAX_TILES 1000
struct tile {
    int x, y;
} tiles[MAX_TILES];
int tile_count = 0;

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

        int x, y;
        if (sscanf(line, "%d,%d", &x, &y) == 2) {
            // store the data
            struct tile t = {x, y};
            tiles[tile_count++] = t;
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

long get_area(int i1, int i2) {
    long dx = abs(tiles[i1].x - tiles[i2].x) + 1;
    long dy = abs(tiles[i1].y - tiles[i2].y) + 1;

    return dx * dy;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    printf("Info: there are %d red tiles.\n", tile_count);
    long max_area = 0;
    for (int i1 = 0; i1 < tile_count; i1++) {
        for (int i2 = 0; i2 < tile_count; i2++) {
            long area = get_area(i1, i2);
            if (area > max_area) {
                max_area = area;
            }
        }
    }

    printf("Info: the calculated solution is %ld.\n", max_area);

    printf("Info: the solution for the sample data should be %ld\n", 50L);
    printf("Info: the solution for the actual data should be %ld\n", 4776100539L);
    return EXIT_SUCCESS;
}

