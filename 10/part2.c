
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_MACHINES 200
#define MAX_COMBOS 100
//typedef unsigned char joltage;
typedef int joltage;
struct machine {
    char *lights;
    int lights_mask;
    int num_lights;
    struct combo {
        int count;
        int *buttons;
    } combos[MAX_COMBOS];
    int button_masks[MAX_COMBOS];
    int combo_count;
    joltage *joltages;
} machines[MAX_MACHINES];
int machine_count;

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

        char *p = line;
        assert(*p == '[');
        while (*p && (*p != ']')) p++;
        int num_lights = p - line - 1;
        machines[machine_count].num_lights = num_lights;
        machines[machine_count].lights = malloc(num_lights + 1);
        strncpy(machines[machine_count].lights, line + 1, num_lights);
        machines[machine_count].lights[num_lights] = 0;
        p++;

        while (isblank(*p)) p++;
        machines[machine_count].combo_count = 0;
        while (*p == '(') {
            p++;
            machines[machine_count].combos[machines[machine_count].combo_count].buttons = malloc(num_lights * sizeof(int));
            int count = 0;
            while (*p != ')') {
                assert(isdigit(*p));
                int val = 0;
                while (isdigit(*p)) {
                    val = 10 * val + (*p - '0');
                    p++;
                }
                assert((*p == ',') || (*p == ')'));
                machines[machine_count].combos[machines[machine_count].combo_count].buttons[count] = val;
                count += 1;
                if (*p == ',') p++;
            }
            machines[machine_count].combos[machines[machine_count].combo_count].count = count;
            assert(*p == ')');
            p++;
            machines[machine_count].combo_count++;
            while (isblank(*p)) p++;
        }

        // TODO: read the joltage settings
        machines[machine_count].joltages = malloc(num_lights * sizeof(joltage));
        assert(machines[machine_count].joltages != NULL);
        assert(*p++ == '{'); 
        int count = 0;
        while (*p != '}') {
            assert(isdigit(*p));
            int val = 0;
            while (isdigit(*p)) {
                val = 10 * val + (*p - '0');
                p++;
            }
            assert((*p == ',') || (*p == '}'));
            machines[machine_count].joltages[count] = val;
            count += 1;
            if (*p == ',') p++;
        }

        machine_count++;
        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_machine(int index) {
    int combo_count = machines[index].combo_count;
    printf("[%s] #%d combos: ", machines[index].lights, combo_count);
    for (int i = 0; i < combo_count; i++) {
        printf("(");
        int count = machines[index].combos[i].count;
        for (int j = 0; j < count; j++) {
            printf("%d%s", machines[index].combos[i].buttons[j], (j == count -1) ? "" : ",");
        }
        printf(") ");
    }
    printf("{ ");
    for (int i = 0; i < machines[index].num_lights; i++) {
        printf("%d ", machines[index].joltages[i]);
    }
    printf("}\n");
}

void print_machine2(int index) {
    int combo_count = machines[index].combo_count;
    printf("[0x%04X] #%d combos: ", machines[index].lights_mask, combo_count);
    for (int i = 0; i < combo_count; i++) {
        printf("(0x%04X) ", machines[index].button_masks[i]);
    }
    printf(", {");
    for (int i = 0; i < machines[index].num_lights; i++) {
        printf("%d ", machines[index].joltages[i]);
    }
    printf("}\n");
}

void print_combo(int count, int *buttons) {
    printf("applying combo: ");
    for (int i = 0; i < count; i++) {
        printf("%d ", buttons[i]);
    }
    printf("\n");
}

void build_masks(void) {
    for (int m = 0; m < machine_count; m++) {
        int lights_mask = 0;
        char *lights = machines[m].lights;
        while (*lights) {
            lights_mask <<= 1;
            if (*lights == '#') {
                lights_mask |= 1;
            }
            lights++;
        }
        machines[m].lights_mask = lights_mask;

        for (int c = 0; c < machines[m].combo_count; c++) {
            int combo_mask = 0;
            int num_lights = machines[m].num_lights;
            int button_count = machines[m].combos[c].count;
            int *buttons = machines[m].combos[c].buttons;
            for (int b = 0; b < button_count; b++) {
                combo_mask |= (1 << (num_lights - buttons[b] - 1));
            }
            machines[m].button_masks[c] = combo_mask;
        }
    }
}

char mask_str1[16], mask_str2[16];
char *mask_str = mask_str1;
char *mask2str(int mask, int num_bits) {
    if (mask_str == mask_str1) mask_str = mask_str2; else mask_str = mask_str1;
    memset(mask_str, 0, sizeof(mask_str1));
    for (int b = 0; b < num_bits; b++) {
        if ((mask & (1 << b)) != 0) {
            mask_str[num_bits - 1 - b] = '#';
        } else {
            mask_str[num_bits - 1 - b] = '.';
        }
    }

    return mask_str;
}

void print_joltages(char *info, joltage *joltages, int count) {
    printf("%s { ", info);
    for (int i = 0; i < count; i++) {
        printf("%d, ", joltages[i]);
    }
    printf("}\n");
}

void handle_combs(int *nums, int count, int sum, int index, void (*handle)(int *, int, void *), void *data) {
    if (index >= count) {
        if (sum == 0) handle(nums, count, data);
    } else {
        for (int s = sum; s >= 0; s--) {
            nums[index] = s;
            handle_combs(nums, count, sum - s, index + 1, handle, data);
        }
    }
}

struct info {
    joltage *joltages;
    int joltage_count;
    int machine;
    int *buttons;
    int min_presses;
    int presses;
    int call_count;
};

void print_seq(int *nums, int count, void *data) {
    ((struct info *)data)->call_count += 1;;
    for (int i = 0; i < count; i++) {
        printf("%d ", nums[i]);
    }
    printf("\n");
}

int min_presses(joltage *joltages, int count, int machine);

void handle_seq(int *nums, int count, void *data) {
    struct info *info = (struct info *)data;
    joltage *new_joltages = malloc(info->joltage_count * sizeof(joltage));
    memcpy(new_joltages, info->joltages, info->joltage_count * sizeof(joltage));
    for (int b = 0; b < count; b++) {
        int button_mask = machines[info->machine].button_masks[info->buttons[b]];
        if (debug) printf("%d x 0x%04X\n", nums[b], button_mask);
        for (int i = info->joltage_count - 1; i >= 0; i--) {
            if ((button_mask & 0x01) == 0x01) {
                new_joltages[i] -= nums[b];
            }
            button_mask >>= 1;
        }
        if (debug) print_joltages("new_joltages = ", new_joltages, info->joltage_count);
    }
    if (debug) print_joltages("new_joltages = ", new_joltages, info->joltage_count);
    int presses = min_presses(new_joltages, info->joltage_count, info->machine);
    if (presses != -1) {
        info->min_presses = MIN(info->min_presses, info->presses + presses);
    }

    free(new_joltages);
}

int min_presses(joltage *joltages, int count, int machine) {
    if (debug) print_joltages("joltages =", joltages, count);
    int lowest_non_zero = -1;
    int non_zero_mask = 0;
    int non_zero_bits = 0;
    if (debug) printf(" joltage count is: %d\n", count);
    for (int i = count - 1; i >= 0; i--) {
        if (joltages[i] != 0) {
            non_zero_mask |= 1 << (count - i - 1);
            non_zero_bits += 1;
            if ((lowest_non_zero == -1) || (joltages[i] < joltages[lowest_non_zero])) {
                lowest_non_zero = i;
            }
        }
    }
    if (non_zero_bits == 0) return 0;

    #define MAX_BUTTONS 100
    int buttons_to_try[MAX_BUTTONS] = {0};
    int button_count = 0;
    if (debug) printf(" lowest non-zero: %d\n", lowest_non_zero);
    if (debug) printf(" non-zero mask: 0x%04X\n", non_zero_mask);
    for (int i = 0; i < machines[machine].combo_count; i++) {
        int button_mask = machines[machine].button_masks[i];
        if (debug) printf(" trying button_mask: 0x%04X\n", button_mask);
        if ((button_mask & (1 << (count - lowest_non_zero - 1))) == 0) continue;
        if ((non_zero_mask & button_mask) == button_mask) {
            if (debug) printf(" button should be fine.\n");
            buttons_to_try[button_count++] = i;
        }
    }
    if (button_count == 0) {
        if (debug) printf("no more buttons to try.\n");
        return -1;
#if 0
    } else if (button_count == 1) {
        // only one button possible (easy case)
        if (debug) printf(" trying only one button.\n");
        int presses = joltages[lowest_non_zero];
        joltage *new_joltages = malloc(count * sizeof(joltage));
        int button_mask = machines[machine].button_masks[buttons_to_try[0]];
        if (debug) printf("%d x 0x%04X\n", presses, button_mask);
        for (int i = count - 1; i >= 0; i--) {
            if ((button_mask & 0x01) != 0) {
                new_joltages[i] = joltages[i] - presses;
            } else {
                new_joltages[i] = joltages[i];
            }
            button_mask >>= 1;
        }
        int new_presses = min_presses(new_joltages, count, machine);
        free(new_joltages);
        if (new_presses >= 0) {
            return presses + new_presses;
        } else {
            return -1;
        }
#endif
    } else {
        // try all permutations of the possible buttons
        if (debug) printf(" found %d possible buttons.\n", button_count);
        struct info info;
        info.joltages = joltages;
        info.joltage_count = count;
        info.machine = machine;
        info.buttons = buttons_to_try;
        info.min_presses = INT_MAX;
        info.presses = joltages[lowest_non_zero];
        info.call_count = 0;
        int *press_counts = malloc(count * sizeof(int));
        memset(press_counts, 0, count * sizeof(int));

        int sum = joltages[lowest_non_zero];
        handle_combs(press_counts, button_count, sum, 0, handle_seq, &info);

        free(press_counts);
        if (info.min_presses != INT_MAX) {
            return info.min_presses;
        }
    }

    return -1;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    printf("Info: machines storage takes %ld bytes.\n", sizeof(machines));
    printf("Info: min there are %d machines.\n", machine_count);
    int total_presses = 0;
    int max_lights = 0;
    for (int i = 0; i < machine_count; i++) max_lights = MAX(max_lights, machines[i].num_lights);
    printf("Info: the maximum number of lights is %d\n", max_lights);
    int max_joltage = 0;
    for (int i = 0; i < machine_count; i++) {
        for (int j = 0; j < machines[i].num_lights; j++) {
            max_joltage = MAX(max_joltage, machines[i].joltages[j]);
        }
    }
    printf("Info: the maximum joltage for any light is %d\n", max_joltage);
    build_masks();

    time_t start_time = time(NULL);
    for (int machine = 0; machine < machine_count; machine++) {
        if (debug) { printf("min(%d, ", machine); print_machine(machine); }
        joltage *joltages = malloc(machines[machine].num_lights * sizeof(joltage));
        for (int j = 0; j < machines[machine].num_lights; j++) {
            joltages[j] = machines[machine].joltages[j];
        }
        int presses = min_presses(joltages, machines[machine].num_lights, machine);
        assert(presses != -1);
        time_t end_time = time(NULL);
        printf("min_presses = %d, time = %ld\n", presses, end_time - start_time);
        free(joltages);
        total_presses += presses;
    }
    printf("Info: the calculated total number of presses is %d.\n", total_presses);

    printf("Info: the solution for the sample data should be %d\n", 33);
    printf("Info: the solution for the actual data should be %d\n", 0);
    return EXIT_SUCCESS;
}

