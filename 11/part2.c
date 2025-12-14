
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_DEVICES 1024
#define MAX_CONNECTIONS 32
#define NAMELEN 4
struct device {
    char name[NAMELEN];
    char connections_txt[MAX_CONNECTIONS][NAMELEN];
    int connections[MAX_CONNECTIONS];
    int connection_count;
} devices[MAX_DEVICES];
int device_count;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    strcpy(devices[0].name, "out");
    devices[0].connection_count = 0;
    device_count++;

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        assert(device_count < MAX_DEVICES);
        strncpy(devices[device_count].name, line, 3);
        char *p = line + 5;
        int count = 0;
        while (*p) {
            strncpy(devices[device_count].connections_txt[count], p, 3);
            count += 1;
            assert(count < MAX_CONNECTIONS);
            p += 3;
            while (isblank(*p)) p++;
        }
        devices[device_count].connection_count = count;

        device_count++;
        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int find_device(char *name) {
    int device = -1;
    for (int i = 0; i < device_count; i++) {
        if (strcmp(name, devices[i].name) == 0) {
            assert(device == -1);
            device = i;
        }
    }
    return device;
}

long *memo = NULL;
void init_memo(void) {
    if (memo == NULL) {
        memo = malloc(device_count * device_count * sizeof(long));
        assert(memo != NULL);
    }
    for (int y = 0; y < device_count; y++) {
        for (int x = 0; x < device_count; x++) {
            memo[y * device_count + x] = -1;
        }
    }
}
long memo_get(int src, int dst) {
    return memo[src * device_count + dst];
}
void memo_set(int src, int dst, long val) {
    memo[src * device_count + dst] = val;
}

long path_count(int src, int dst) {
    if (src == dst) return 1;
    long sum = memo_get(src, dst);
    if (sum == -1) {
        sum = 0L;
        for (int i = 0; i < devices[src].connection_count; i++) {
            sum += path_count(devices[src].connections[i], dst);
        }
        memo_set(src, dst, sum);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    printf("Info: there are %d devices\n", device_count);
    // find the start
    int start = -1;
    debug = 0;
    for (int i = 0; i < device_count; i++) {
        if (strcmp(devices[i].name, "svr") == 0) {
            assert(start == -1);
            start = i;
        }
        if (debug) printf("%s: ", devices[i].name);
        for (int j = 0; j < devices[i].connection_count; j++) {
            int id = find_device(devices[i].connections_txt[j]);
            devices[i].connections[j] = id;
            if (id != -1) {
                if (debug) printf("%s ", devices[devices[i].connections[j]].name);
            } else {
                assert(strcmp(devices[i].connections_txt[j], "out") == 0);
                if (debug) printf("out ");
            }
        }
        if (debug) printf("\n");
    }
    debug = 1;
    int svr_id = find_device("svr");
    int dac_id = find_device("dac");
    int fft_id = find_device("fft");
    int out_id = find_device("out");
    assert(svr_id != -1);
    assert(dac_id != -1);
    assert(fft_id != -1);
    assert(out_id != -1);
    init_memo();
    long paths = path_count(svr_id, dac_id) * path_count(dac_id, fft_id) * path_count(fft_id, out_id) +
        path_count(svr_id, fft_id) * path_count(fft_id, dac_id) * path_count(dac_id, out_id);
    printf("Info: the calculated number of paths from '%s' to 'out' is %ld.\n", devices[svr_id].name, paths);

    printf("Info: the solution for the sample data should be %ld\n", 2L);
    printf("Info: the solution for the actual data should be %ld\n", 553204221431080L);
    return EXIT_SUCCESS;
}
