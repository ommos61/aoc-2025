
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 10240
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_NUM_LINES 100
#define MAX_NUMBERS 2048
struct num_line {
    int numbers[MAX_NUMBERS];
} num_lines[MAX_NUM_LINES];
int number_count = 0;
int num_line_count = 0;

#define MAX_OPERATORS 1024
char operators[MAX_OPERATORS];
int operator_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int handling_numbers = 1;
    int number_count = 0;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        char *p = line;
        // skip spaces
        while (isblank(*p)) p++;
        if (!isdigit(*p)) handling_numbers = 0;
        if (debug) printf("handling line %d.\n", line_count);
        if (handling_numbers) {
            int num_index = 0;
            if (debug) printf("handling number %d.\n", num_index);
            while (*p) {
                assert(isdigit(*p));
                int num = 0;
                while (isdigit(*p)) {
                    num = num * 10 + (*p - '0');
                    p++;
                }
                while (isblank(*p)) p++;
                num_lines[num_line_count].numbers[num_index] = num;
                num_index++;
                assert(num_index < MAX_NUMBERS);
            }
            if (number_count == 0) {
                number_count = num_index;
            } else {
                assert(number_count == num_index);
            }
            num_line_count++;
        } else { // handling operators
            int op_index = 0;
            while (*p) {
                assert((*p == '+') || (*p == '*'));
                operators[op_index++] = *p++;
                while (isblank(*p)) p++;
            }
            operator_count = op_index;
            assert(operator_count == number_count);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    printf("Info: there are %d columns.\n", operator_count);
    long sum = 0L;
    for (int op_index = 0; op_index < operator_count; op_index++) {
        char operator = operators[op_index];
        long col_result = (operator == '*') ? 1L : 0L;
        if (debug) printf("%c ", operator);
        for (int line = 0; line < num_line_count; line++) {
            if (debug) printf("%d ", num_lines[line].numbers[op_index]);
            if (operator == '+') {
                col_result += num_lines[line].numbers[op_index];
            } else if (operator == '*') {
                col_result *= num_lines[line].numbers[op_index];
            } else {
                printf("Error: unknown operator '%c'.\n", operator);
            }
        }
        if (debug) printf("\nthe column result is %ld.\n", col_result);
        sum += col_result;
    }
    printf("Info: the calculated grand total is %ld.\n", sum);

    printf("Info: the solution for the sample data should be %ld\n", 4277556L);
    printf("Info: the solution for the actual data should be %ld\n", 4580995422905L);
    return EXIT_SUCCESS;
}

