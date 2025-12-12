
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_MACHINES 200
#define MAX_COMBOS 100
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
        p++;

        // TODO: read the joltage settings

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
    printf("\n");
}

void print_machine2(int index) {
    int combo_count = machines[index].combo_count;
    printf("[0x%04X] #%d combos: ", machines[index].lights_mask, combo_count);
    for (int i = 0; i < combo_count; i++) {
        printf("(0x%04X) ", machines[index].button_masks[i]);
    }
    printf("\n");
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

int *presses_memo = NULL;
int memo_count = 0;
void init_presses_memo(void) {
    memo_count = 2048;
    if (presses_memo == NULL) {
        presses_memo = malloc(memo_count * sizeof(int));
    }
    assert(presses_memo != NULL);
    for (int i = 0; i < memo_count; i++) {
        presses_memo[i] = 0;
    }
}
//#define memo_get(lights) (presses_memo[lights])
int memo_get(int lights) {
    assert(lights < 2048);
    assert(presses_memo != NULL);
    return presses_memo[lights];
}
void memo_put(int lights, int presses) {
    assert(lights < 2048);
    assert(presses_memo != NULL);
    presses_memo[lights] = presses;
}

int min_presses = INT_MAX;
#define MAX_DEPTH 50
int do_it(int target, int current, int num_lights, int presses, int *button_masks, int combo_count) {
    if (debug) printf("[%s] [%s], %d\n", mask2str(target, num_lights), mask2str(current, num_lights), presses);

    // end recusion when found
    if (target == current) {
        if (presses <= min_presses) {
            if (debug) printf("==========> Found a result.\n");
            printf("==========> [%s] [%s], %d\n", mask2str(target, num_lights), mask2str(current, num_lights), presses);
            min_presses = presses;
        }
        return presses;
    }

    int memo_presses = memo_get(current);
    if (memo_presses != 0) return memo_presses;

    // end resucrsion when exceeding the current minimum
    if (presses > min_presses) return -1;
    if (presses > MAX_DEPTH) return -1;

    int len = num_lights;
    for (int i = 0; i < len; i++) {
        int mask = 1 << (len - 1 - i);
        // if (debug) printf("mask = [%s]0x%04X\n", mask2str(mask, num_lights), mask);
        if ((target & mask) != (current & mask)) {
            int local_min_presses = INT_MAX;
            for (int c = 0; c < combo_count; c++) {
                // check if combo can change 'current[i]'
                int button_mask = button_masks[c];
                if ((button_mask & mask) == mask) {
                    // apply the combo and recurse
                    int new_current = current;
                    new_current ^= button_mask;
                    if (debug) printf("button_mask: [%s] 0x%04X\n", mask2str(button_mask, num_lights), button_mask);
                    int new_presses = do_it(target, new_current, num_lights, presses + 1, button_masks, combo_count);
                    if (new_presses != -1) {
                        local_min_presses = MIN(local_min_presses, new_presses);
                    }
                }
            }
            presses = local_min_presses;
            break;
        }
    }
    memo_put(current, presses);
    return presses;
}

long find_presses(int machine) {
    int num_lights = machines[machine].num_lights;
    int target = machines[machine].lights_mask;
    int current = 0;

    if (debug) printf("target:  [%s]\n", mask2str(target, num_lights));
    if (debug) printf("current: [%s]\n", mask2str(current, num_lights));
    debug = 0;
    min_presses = INT_MAX;
    init_presses_memo();
    int result = do_it(target, current, num_lights, 0, machines[machine].button_masks, machines[machine].combo_count);
    if (result == -1) {
        printf("couldn't find a result in %d presses\n", MAX_DEPTH);
    }
    debug = 1;
    if (debug) printf("minimum presses was %d.\n", min_presses);

    return min_presses;
}

int find_presses2(int machine) {
    int num_lights = machines[machine].num_lights;
    int target = machines[machine].lights_mask;
    int current = 0;
    if (debug) printf("==========> [%s] [%s]\n", mask2str(target, num_lights), mask2str(current, num_lights));

    int *button_masks = machines[machine].button_masks;
    int button_count = machines[machine].combo_count;

    #define MAX_BUTTONS 50
    int buttons[MAX_BUTTONS];
    int num_buttons = 1;
    while (num_buttons < MAX_BUTTONS) {
        if (debug) printf("trying: %d\n", num_buttons);
        for (int i = 0; i < num_buttons; i++) {
            buttons[i] = 0;
        }

        do {
            // apply yhe button presses
            current = 0;
            for (int i = 0; i < num_buttons; i++) {
                current ^= button_masks[buttons[i]];
            }
            if (0 && debug) {
                printf("buttons: ");
                for (int i = 0; i < num_buttons; i++) {
                    printf("%d=0x%04X ", buttons[i], button_masks[buttons[i]]);
                }
                printf("[%s] 0x%04X", mask2str(current, num_lights), current);
                printf("\n");
            }

            if (current == target) {
                break;
            }

            // prepare the next button press combination
            int button = num_buttons - 1;
            while (button >= 0) {
                buttons[button] += 1;
                buttons[button] %= button_count;
                if (buttons[button] == 0) {
                    button--;
                } else {
                    break;
                }
            }
            if ((button == -1) && (buttons[0] == 0)) {
                if (debug) printf("exhausted button combinations\n");
                break;
            }

        } while (current != target);
        if (current == target) {
            break;
        }

        num_buttons++;
    }
    if (debug) printf("==========> [%s] [%s], %d\n", mask2str(target, num_lights), mask2str(current, num_lights), num_buttons);
    return num_buttons;
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
    printf("Info: there are %d machines.\n", machine_count);
    long total_presses = 0;
    int max_lights = 0;
    for (int i = 0; i < machine_count; i++) {
        if (machines[i].num_lights > max_lights) {
            max_lights = machines[i].num_lights;
        }
    }
    printf("Info: the maximum number of lights is %d\n", max_lights);
    build_masks();

    for (int i = 0; i < machine_count; i++) {
        if (debug) print_machine2(i);
        long presses = find_presses2(i);
        total_presses += presses;
    }
    printf("Info: the calculated total number of presses is %ld.\n", total_presses);

    printf("Info: the solution for the sample data should be %ld\n", 7L);
    printf("Info: the solution for the actual data should be %ld\n", 550L);
    return EXIT_SUCCESS;
}

