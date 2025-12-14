
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
#define NUM_SHAPES 6
#define SHAPE_SIDE 3
char shapes[NUM_SHAPES][SHAPE_SIDE][SHAPE_SIDE];
int surfaces[NUM_SHAPES];
#define MAX_REGIONS 1024
struct region {
    int w, h;
    int package_counts[NUM_SHAPES];
} regions[MAX_REGIONS];
int region_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int doing_shapes = 1;
    int shape_id = 0, shape_line = 0;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (doing_shapes) {
            // read the shapes
            if (isdigit(line[0])) {
                shape_id = line[0] - '0';
                shape_line = 0;
            } else {
                memcpy(&shapes[shape_id][shape_line][0], line, 3);
                shape_line++;
            }

            if (*line == 0) {
                if (shape_id == 5) doing_shapes = 0;
                continue;
            }
        } else {
            int w, h, v0, v1, v2, v3, v4, v5 = 0;
            if (sscanf(line, "%dx%d: %d %d %d %d %d %d", &w, &h, &v0, &v1, &v2, &v3, &v4, &v5) == 8) {
                // store the data
                struct region r = {w, h, {v0, v1, v2, v3, v4, v5}};
                regions[region_count++] = r;
                assert(region_count < MAX_REGIONS);
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

void print_shapes(void) {
    for (int i = 0; i < NUM_SHAPES; i++) {
        printf("%d:\n", i);
        for (int y = 0; y < SHAPE_SIDE; y++) {
            for (int x = 0; x < SHAPE_SIDE; x++) {
                printf("%c", shapes[i][y][x]);
            }
            printf("\n");
        }
        printf("\n");
    }
}

void print_regions(void) {
    for (int i = 0; i < region_count; i++) {
         printf("%dx%d: ", regions[i].w, regions[i].h);
         for (int j = 0; j < NUM_SHAPES; j++) {
            printf("%d ", regions[i].package_counts[j]);
         }
         printf("\n");
    }
}

void calculate_surfaces(void) {
    for (int i = 0; i < NUM_SHAPES; i++) {
        int surface = 0;
        for (int y = 0; y < SHAPE_SIDE; y++) {
            for (int x = 0; x < SHAPE_SIDE; x++) {
                if (shapes[i][y][x] == '#') surface++;
            }
        }
        surfaces[i] = surface;
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
#if 0
    if (debug) print_shapes();
    if (debug) print_regions();
#endif
    calculate_surfaces();
    printf("Info: there are %d regions\n", region_count);
    int regions_fit = 0;
    for (int i = 0; i < region_count; i++) {
        int total_surface = 0;
        int total_surface2 = 0;
        for (int j = 0; j < NUM_SHAPES; j++) {
            total_surface += regions[i].package_counts[j] * surfaces[j];
            total_surface2 += regions[i].package_counts[j] * 9;
        }
        int region_surface = regions[i].w * regions[i].h;
        //if (debug) printf("region surface: %d, package surfaces total: %d\n", regions[i].w * regions[i].h, total_surface);
        if (total_surface <= region_surface) {
            if (debug) printf("region %d space remaining is %d.\n", i, region_surface - total_surface);
            if (debug) printf("region %d space remaining is %d.\n", i, region_surface - total_surface2);
            regions_fit += 1;
        }
    }
    printf("The calculated number of regions that fit the required presents are %d.\n", regions_fit);

    printf("Info: the solution for the sample data should be %d\n", 2);
    printf("Info: the solution for the actual data should be %d\n", 565);
    return EXIT_SUCCESS;
}

