
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
struct num_line {
    char *numbers;
} num_lines[MAX_NUM_LINES];
int num_line_count = 0;
unsigned num_line_length = 0;

char *operator_line;

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
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (debug) printf("line is %lu characters long.\n", strlen(line));
        if (debug) printf("handling line %d.\n", line_count);
        if ((line[0] == '+') || (line[0] == '*')) handling_numbers = 0;
        if (handling_numbers) {
            if (num_line_length == 0) {
                num_line_length = strlen(line);
            } else {
                assert(strlen(line) == num_line_length);
            }

            num_lines[num_line_count].numbers = malloc(num_line_length + 1);
            strcpy(num_lines[num_line_count].numbers, line);
            num_line_count++;
        } else { // handling operators
            operator_line = malloc(strlen(line) + 1);
            strcpy(operator_line, line);
            assert(strlen(operator_line) == num_line_length);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int get_number(int index) {
    int number = 0;

    for (int i = 0; i < num_line_count; i++) {
        char c = num_lines[i].numbers[index];
        if (isdigit(c)) {
            number = number * 10 + (c - '0');
        } else {
            assert(isblank(c));
        }
    }

    return number;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    long sum = 0L;
    int index = num_line_length;
    int number_count = 0;
    while (index >= 0) {
        index--;

        #define MAX_NUMBERS 20
        int numbers[MAX_NUMBERS];

        int number = get_number(index);
        if (debug) printf("number: %5d.\n", number);
        numbers[number_count++] = number;
        assert(number_count <= MAX_NUMBERS);

        char operator = operator_line[index];
        if (!isblank(operator)) {
            if (debug) printf("operator is '%c', number count is %d.\n", operator, number_count);
            assert((operator == '+') || (operator == '*'));

            long result = (operator == '*') ? 1L : 0L;
            for (int i = 0; i < number_count; i++) {
                if (operator == '+') {
                    result += numbers[i];
                } else if (operator == '*') {
                    result *= numbers[i];
                if (debug) printf("result = %ld.\n", result);
                }
            }
            if (debug) printf("result = %ld.\n", result);
            sum += result;
            number_count = 0;
            // skip the column with spaces
            index--;
        }
    }
    printf("Info: the calculated grand total is %ld.\n", sum);

    printf("Info: the solution for the sample data should be %ld\n", 3263827L);
    printf("Info: the solution for the actual data should be %ld\n", 10875057285868L);
    return EXIT_SUCCESS;
}

