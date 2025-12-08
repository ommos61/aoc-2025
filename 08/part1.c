
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_BOXES 2000
struct box {
    int x, y, z;
    int circuit;
    int connected;
} boxes[MAX_BOXES];
int box_count = 0;

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

        int x = 0, y = 0, z = 0;
        if (sscanf(line, "%d,%d,%d", &x, &y, &z) == 3) {
            // store the data
            struct box b = {x, y, z, box_count, -1};
            boxes[box_count++] = b;
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

long distance2(int i1, int i2) {
    long dx = boxes[i1].x - boxes[i2].x;
    long dy = boxes[i1].y - boxes[i2].y;
    long dz = boxes[i1].z - boxes[i2].z;
    long distance2 = dx * dx + dy * dy + dz * dz;

    return distance2;
}
long *distances = NULL;
#define DISTANCE(x,y) (distances[(y) * box_count + (x)])
void build_distances(void) {
    distances = malloc(box_count * box_count * sizeof(long));
    assert(distances != NULL);
    for (int y = 0; y < box_count; y++) {
        for (int x = 0; x <= y; x++) {
            long distance = distance2(x, y);
            DISTANCE(x, y) = distance;
            DISTANCE(y, x) = distance;
        }
    }
}
void print_distances(void) {
    for (int y = 0; y < box_count; y++) {
        for (int x = 0; x < box_count; x++) {
            printf("%10ld ", DISTANCE(x, y));
        }
        printf("\n");
    }
}

int sizes[MAX_BOXES];
void init_sizes(void) {
    for (int i = 0; i < box_count; i++) sizes[i] = 1;
}
void print_sizes(void) {
    for (int i = 0; i < box_count; i++) {
        printf("%d ", sizes[i]);
    }
    printf("\n");
}
int size_comp(const void *p1, const void *p2) {
    return *(int *)p1 < *(int *)p2;
}

void print_box(int i) {
    printf("(%d,%d,%d,%d)", boxes[i].x, boxes[i].y, boxes[i].z, boxes[i].circuit);
}

long last_shortest = 0;
int connect_shortest(void) {
    int connected = 0;
    long shortest = LONG_MAX;
    int index1 = 0, index2 = 0;

    for (int y = 0; y < box_count; y++) {
        for (int x = 0; x < y; x++) {
            if (1 || (boxes[x].circuit != boxes[y].circuit)) {
                long d = DISTANCE(x, y);
                if ((d > last_shortest) && (d < shortest)) {
                    shortest = d;
                    index1 = x; index2 = y;
                }
            } else {
                // already in same circuit
            }
        }
    }
    if (debug) {
        printf("Connection ");
        print_box(index1);
        printf(" and ");
        print_box(index2);
        printf("\n");
    }
    assert(shortest < LONG_MAX);
    int circuit1 = boxes[index1].circuit;
    int circuit2 = boxes[index2].circuit;
    if (circuit1 != circuit2) {
        assert(circuit1 != circuit2);
        sizes[circuit1] += sizes[circuit2];
        sizes[circuit2] = 0;
        for (int i = 0; i < box_count; i++) {
            if (boxes[i].circuit == circuit2) {
                boxes[i].circuit = circuit1;
            }
        }
        connected = 1;
    }
    last_shortest = DISTANCE(index1, index2);
    return connected;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    int loops = 10;
    if (strcmp(fname, "input.txt") == 0) loops = 1000;

    printf("Info: there are %d junction boxes.\n", box_count);
    init_sizes();
    build_distances();
    //if (debug) print_distances();
    for (int i = 0; i < loops; i++) {
        int connected = connect_shortest();
        if (debug) if (!connected) printf("Skipped\n");
        if (debug) print_sizes();
    }
    // sort on sizes
    qsort(sizes, box_count, sizeof(int), size_comp);
    if(debug) print_sizes();
    printf("Info: the calculated result is %d.\n", sizes[0] * sizes[1] * sizes[2]);

    printf("Info: the solution for the sample data should be %d\n", 40);
    printf("Info: the solution for the actual data should be %d\n", 50760);
    return EXIT_SUCCESS;
}

