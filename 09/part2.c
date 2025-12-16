
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

#define MAX_LINES MAX_TILES
struct line {
    int x1, y1;
    int x2, y2;
    long length;
} lines[MAX_LINES];
int line_count;

void make_ordered_lines(void) {
    // make sure the 0th tile element is copied at the end
    tiles[tile_count] = tiles[0];
    for (int i = 0; i < tile_count; i++) {
        struct line l;
        l.x1 = MIN(tiles[i].x, tiles[i+1].x);
        l.y1 = MIN(tiles[i].y, tiles[i+1].y);
        l.x2 = MAX(tiles[i].x, tiles[i+1].x);
        l.y2 = MAX(tiles[i].y, tiles[i+1].y);
        l.length = (l.x2 - l.x1) + (l.y2 - l.y1);
        lines[line_count++] = l;
    }
}

#define PIC_WIDTH 1024
void make_ppm(void) {
    char *pixels = malloc(PIC_WIDTH * PIC_WIDTH * sizeof(char));
    int maxx = 0, maxy = 0;
    for (int i = 0; i < tile_count; i++) {
        maxx = MAX(maxx, tiles[i].x);
        maxy = MAX(maxy, tiles[i].y);
    }
    printf("maxx = %d, maxy = %d\n", maxx, maxy);

    for (int i = 0; i < line_count; i++) {
        int ax = (lines[i].x1 * (PIC_WIDTH - 10) / maxx);
        int ay = (lines[i].y1 * (PIC_WIDTH - 10) / maxy);
        int bx = (lines[i].x2 * (PIC_WIDTH - 10) / maxx);
        int by = (lines[i].y2 * (PIC_WIDTH - 10) / maxy);

        pixels[ay * PIC_WIDTH + ax] = 1;
        pixels[by * PIC_WIDTH + bx] = 1;
        if (ay == by) {
            for (int x = ax+1; x < bx; x++) {
                pixels[ay * PIC_WIDTH + x] = 2;
            }
        } else {
            for (int y = ay+1; y < by; y++) {
                pixels[y * PIC_WIDTH + ax] = 2;
            }
        }
    }

    // write out the image
    FILE *fout= fopen("test.ppm", "wb");
    fprintf(fout, "P6\n");
    fprintf(fout, "%d %d\n", PIC_WIDTH, PIC_WIDTH);
    fprintf(fout, "255\n");
    for (int y = 0; y < PIC_WIDTH; y++) {
        for (int x = 0; x < PIC_WIDTH; x++) {
            int val = pixels[y * PIC_WIDTH + x];
            int color = 0x00000000;
            switch (val) {
                case 1:
                    color = 0x000000FF;
                    break;
                case 2:
                    color = 0x0000FF00;
                    break;
            }
            fwrite(&color, 1, 3, fout);
        }
    }
    fclose(fout);
    free(pixels);
}

char *compressed_tile_map = NULL;
int cmap_width = 0, cmap_height = 0;
#define GET_TILE(x,y) (compressed_tile_map[(y) * cmap_width + (x)])
#define SET_TILE(x,y,v) (compressed_tile_map[(y) * cmap_width + (x)] = (v))
int xs[MAX_TILES];
int ys[MAX_TILES];
int xs_count = 0, ys_count = 0;
int get_index(int *list, int count, int val) {
    int index = -1;
    for (int i = 0; i < count; i++) {
         if (val == list[i]) {
            index = i;
            break;
         }
    }
    return index;
}

int int_cmp(const void *p1, const void *p2) {
    return *(int *)p1 > *(int *)p2;
}

#define RED (1)
#define GREEN (2)
#define BLUE (3)
void output_map_ppm(int width, int height) {
    FILE *fout = fopen("cmap.ppm", "wb");
    assert(fout != NULL);
    fprintf(fout, "P6\n");
    fprintf(fout, "%d %d\n", width, height);
    fprintf(fout, "255\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int color = 0x00000000;
            switch (compressed_tile_map[y * width + x]) {
                case RED:
                    color = 0x000000FF;
                    break;
                case GREEN:
                    color = 0x0000FF00;
                    break;
                case BLUE:
                    color = 0x00FF0000;
                    break;
            }
            fwrite(&color, 1, 3, fout);
        }
    }
    fclose(fout);
}

struct direction {
    int dx, dy;
} directions[] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
void flood_fill(int x, int y, char color) {
    char tile = GET_TILE(x, y);
    if (tile == 0) {
        SET_TILE(x, y, color);
        for (unsigned d = 0; d < array_count(directions); d++) {
            int nx = x + directions[d].dx;
            int ny = y + directions[d].dy;
            if ((nx < 0) || (ny < 0) || (nx > cmap_width) || (ny > cmap_height)) continue;
            flood_fill(nx, ny, color);
        }
    }
}

void make_compressed_map(void) {
    // push 0 on the lists so we can have a border to floodfill
    xs[xs_count++] = 0;
    ys[ys_count++] = 0;

    // now push all x and y coordinates from the red tiles on the lists
    for (int i = 0; i < tile_count; i++) {
        int x_index = get_index(xs, xs_count, tiles[i].x);
        int y_index = get_index(ys, ys_count, tiles[i].y);
        if (x_index == -1) xs[xs_count++] = tiles[i].x;
        if (y_index == -1) ys[ys_count++] = tiles[i].y;
    }

    // sort the lists
    qsort(xs, xs_count, sizeof(int), int_cmp);
    qsort(ys, ys_count, sizeof(int), int_cmp);

    // allocate memory for the compressed map
    cmap_width = (xs_count - 1) * 2;
    cmap_height = (ys_count - 1) * 2;
    compressed_tile_map = malloc(cmap_width * cmap_height * sizeof(char));
    assert(compressed_tile_map != NULL);
    memset(compressed_tile_map, 0, cmap_width * cmap_height);

    // mark the points and lines on the compressed map
#define CINDEX(v) (2 * ((v) - 1) + 1)
    for (int i = 0; i < tile_count; i++) {
        int cx1 = CINDEX(get_index(xs, xs_count, tiles[i].x));
        int cy1 = CINDEX(get_index(ys, ys_count, tiles[i].y));
        assert((cx1 != -1) && (cy1 != -1));
        compressed_tile_map[cy1 * cmap_width + cx1] = RED;

        int cx2 = CINDEX(get_index(xs, xs_count, tiles[i + 1].x));
        int cy2 = CINDEX(get_index(ys, ys_count, tiles[i + 1].y));
        assert((cx2 != -1) && (cy2 != -1));
        for (int x = MIN(cx1, cx2) + 1; x < MAX(cx1, cx2); x++) {
            compressed_tile_map[cy1 * cmap_width + x] = GREEN;
        }
        for (int y = MIN(cy1, cy2) + 1; y < MAX(cy1, cy2); y++) {
            compressed_tile_map[y * cmap_width + cx1] = GREEN;
        }
    }

    // preform a floodfill on the outside, startint at (0, 0)
    flood_fill(0, 0, BLUE);

    // everything 0 is now inside, so make it green
    for (int y = 0; y < cmap_height; y++) {
        for (int x = 0; x < cmap_width; x++) {
            char v = GET_TILE(x, y);
            if (v == 0) SET_TILE(x, y, GREEN);
        }
    }

    // output the compressed map to ppm
    output_map_ppm(cmap_width, cmap_height);
}

int area_is_valid(int x1, int y1, int x2, int y2) {
    int result = 1;
    int cx1 = CINDEX(get_index(xs, xs_count, x1));
    int cy1 = CINDEX(get_index(ys, ys_count, y1));
    int cx2 = CINDEX(get_index(xs, xs_count, x2));
    int cy2 = CINDEX(get_index(ys, ys_count, y2));

    for (int x = cx1; result && (x <= cx2); x++) {
        int top_tile = GET_TILE(x, cy1);
        int bot_tile = GET_TILE(x, cy2);
        if ((top_tile == BLUE) || (bot_tile == BLUE)) {
            result = 0;
        }
    }
    for (int y = cy1; result && (y <= cy2); y++) {
        if ((GET_TILE(cx1, y) == BLUE) || (GET_TILE(cx2, y) == BLUE)) {
            result = 0;
        }
    }

    return result;
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
    // copy first element to one after last to not have to do modulo
    tiles[tile_count] = tiles[0];
    make_ordered_lines();
    make_ppm();
    make_compressed_map();

    // loop over the point combinations
    long max_area = 0L;
    int max_i1, max_i2;
    for (int i1 = 0; i1 < tile_count; i1++) {
        for (int i2 = i1 + 1; i2 < tile_count; i2++) {
            int minx = MIN(tiles[i1].x, tiles[i2].x);
            int maxx = MAX(tiles[i1].x, tiles[i2].x);
            int miny = MIN(tiles[i1].y, tiles[i2].y);
            int maxy = MAX(tiles[i1].y, tiles[i2].y);

            // skip if area is not bigger than current max_area
            long area = (long)(maxx - minx + 1) * (long)(maxy - miny + 1);
            if (area <= max_area) continue;

            if (area_is_valid(minx, miny, maxx, maxy)) {
                max_i1 = i1, max_i2 = i2;
                max_area = area;
            }
        }
    }
    printf("(%d, %d) -> (%d, %d), max_area = %ld\n", tiles[max_i1].x, tiles[max_i1].y, tiles[max_i2].x, tiles[max_i2].y, max_area);
    printf("Info: the calculated solution is %ld.\n", max_area);

    printf("Info: the solution for the sample data should be %ld\n", 24L);
    printf("Info: the solution for the actual data should be %ld\n", 1476550548L);
    return EXIT_SUCCESS;
}

