#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BENCHMARK
#include <time.h>
#endif

static bool SAVE_FLAG = false;
static char* SAVE_FILENAME = "matrix.txt";

void print_matrix(unsigned char** matrix, int y_len, int x_len) {
    for (int y = 0; y < y_len; y++) {
        for (int x = 0; x < x_len; x++) {
            printf("%d ", (matrix[y][x / 8] >> (x % 8)) & 1);
        }
        printf("\n");
    }
}

unsigned char** init_matrix(int y_len, int x_len) {
    unsigned char** matrix =
        (unsigned char**)(malloc(y_len * sizeof(unsigned char*)));
    for (int y = 0; y < y_len; y++) {
        matrix[y] = (unsigned char*)(malloc(x_len / 8 + 1));
        for (int i = 0; i < x_len / 8 + 1; i++) {
            matrix[y][i] = 0;
        }
    }
    return matrix;
}

void print_bits(unsigned char byte) {
    for (int i = 0; i < 8; i++) {
        printf("%d", (byte >> i) & 1);
    }
}

void free_matrix(unsigned char** matrix, int y_len) {
    for (int y = 0; y < y_len; y++) {
        free(matrix[y]);
    }
    free(matrix);
}

inline bool get_bit_from_byte(unsigned char byte, int i) {
    return (byte >> i) & 1;
}
inline void set_bit_in_byte(unsigned char* byte_ptr, int i) {
    *byte_ptr |= 1 << i;
}

// these functions find the appropriate byte in the matrix, and then call the
// functions above to get/set the bit
inline bool get_matrix_bit(unsigned char** matrix, int y, int x) {
    return get_bit_from_byte(matrix[y][x / 8], x % 8);
}
inline void set_matrix_bit(unsigned char** matrix, int y, int x) {
    // pass the adress of the byte to set_bit_in_byte
    // so that it can modify the byte
    set_bit_in_byte(&matrix[y][x / 8], x % 8);
}

// saves the matrix to a file
void save_matrix(unsigned char** matrix, int y_len, int x_len, char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file: %d \n", errno);
        return;
    }

    for (int y = 0; y < y_len; y++) {
        for (int x = 0; x < x_len; x++) {
            fprintf(file, "%d ", get_matrix_bit(matrix, y, x));
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Saved matrix to %s \n", filename);
}

int partition(int* arr, int arr_len, int* set, int* set1_len) {
    unsigned int sum = 0;
    for (int i = 0; i < arr_len; i++) {
        sum += arr[i];
    }

    if (sum % 2 != 0) {
        return -1;
    }

    int y_len = (sum / 2) + 1;
    int x_len = arr_len + 1;

    unsigned char** matrix = init_matrix(y_len, x_len);

    // set first row to 1
    for (int x = 0; x < x_len; x++) {
        set_matrix_bit(matrix, 0, x);
    }

    for (int y = 1; y < y_len; y++) {
        for (int x = 1; x < x_len; x++) {
            // get the bit from the previous column
            bool bit = get_matrix_bit(matrix, y, x - 1);
            if (bit) {
                set_matrix_bit(matrix, y, x);
            }

            int new_element = arr[x - 1];

            if (!bit && new_element <= y) {
                bit = get_matrix_bit(matrix, y - new_element, x - 1);
                if (bit) {
                    set_matrix_bit(matrix, y, x);
                }
            }
        }
    }

#ifndef BENCHMARK
    print_matrix(matrix, y_len, x_len);

    if (SAVE_FLAG) {
        save_matrix(matrix, y_len, x_len, SAVE_FILENAME);
    }
#endif

    // get the bottom right element
    int is_partitionable = get_matrix_bit(matrix, y_len - 1, x_len - 1);

    // use on array to store both sets
    // set1 goes from beggining to end
    // set2 goes from end to beggining
    *set1_len = 0;
    int set2_len = 0;

    if (is_partitionable && set != NULL && set1_len != NULL) {
        // Start from last element in dp table.
        int i = arr_len;
        int curr_sum = sum / 2;

        while (i > 0 && curr_sum >= 0) {
            // If current element does not
            // contribute to k, then it belongs
            // to set 2.
            if (get_matrix_bit(matrix, curr_sum, i - 1)) {
                i--;
                // set2 is filled from the end
                set[(arr_len - 1) - set2_len++] = arr[i];
            }

            // If current element contribute
            // to k then it belongs to set 1.
            else if (get_matrix_bit(matrix, curr_sum - arr[i - 1], i - 1)) {
                i--;
                curr_sum -= arr[i];
                set[(*set1_len)++] = arr[i];
            }
        }
    }

    free_matrix(matrix, y_len);

    return is_partitionable;
}

int main(int argc, char* argv[]) {
#ifdef BENCHMARK

    // get number of elements from the first argument
    int elements = atoi(argv[1]);

    if (elements % 4 != 0) {
        // stderror
        fprintf(stderr,
                "Error: ammount of elements has to be a multiple of 4, "
                "otherwise the sum is odd\n");
        return 1;
    }

    int arr[elements];

    for (int i = 0; i < elements; i++) {
        arr[i] = i;
    }

    int arr_len = sizeof(arr) / sizeof(arr[0]);

    clock_t start = clock();
    int result = partition(arr, arr_len);

    // print the time it took to run the algorithm in microseconds
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000000;
    printf("%f\n", time_spent);
#else
    // check if the user provided any arguments
    if (argc < 2) {
        printf("Usage: %s [--save FILENAME] n1 n2 n3 ...\n", argv[0]);
        return 1;
    }

    char* filename = "";
    if (strcmp(argv[1], "--save") == 0) {
        // check if the user provided enough arguments
        if (argc < 4) {
            printf("Usage: %s [--save FILENAME] n1 n2 n3 ...\n", argv[0]);
            return 1;
        }
        SAVE_FLAG = true;
        SAVE_FILENAME = argv[2];
        argv += 2;
        argc -= 2;
    }

    // convert the arguments to integers and store them in an array
    // using VLAs (variable length arrays)
    int arr[argc - 1];
    char* endptr;
    for (int i = 1; i < argc; i++) {
        errno = 0;  // reset errno before calling strtol
        long val = strtol(argv[i], &endptr, 10);  // base 10
        if (errno != 0 || *endptr != '\0') {
            printf("Error: argument %d is not an integer (errno=%d)\n", i,
                   errno);
            return errno;
        }
        arr[i - 1] = (int)val;
    }

    // print the arguments
    // for (int i = 0; i < argc - 1; i++) {
    //     printf("%d ", n[i]);
    // }
    // printf("\n");

    // call the partition function and print the result
    int arr_len = sizeof(arr) / sizeof(arr[0]);

    // create a an array which will hold the two sets
    int set[arr_len];
    // the length of the first set, used to figure out where the first
    // set ends and the second set begins
    int set1_len = 0;

    int result = partition(arr, arr_len, set, &set1_len);

    printf("\n");
    if (result == 1) {
        printf("The set can be partitioned\n");

        printf("Set 1: {");
        for (int i = 0; i < set1_len - 1; i++) {
            printf("%d, ", set[i]);
        }
        printf("%d}\n", set[set1_len - 1]);

        printf("Set 2: {");
        for (int i = set1_len; i < arr_len - 1; i++) {
            printf("%d, ", set[i]);
        }
        printf("%d}\n", set[arr_len - 1]);
    } else if (result == 0) {
        printf("The set cannot be partitioned\n");
    } else if (result == -1) {
        printf("The set cannot be partitioned (sum is odd)\n");
    }
#endif

    return 0;
}