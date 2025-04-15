#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NUM_HIDDEN_KEYS 80
#define MAX_POSITIVE_VALUE 1000  //adjust this upper bound for positive numbers if needed

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <L> <output_filename>\n", argv[0]);
        return 1;
    }
    int L = atoi(argv[1]);
    const char *filename = argv[2];

    if (L < NUM_HIDDEN_KEYS) {
        printf("Error: L must be at least %d\n", NUM_HIDDEN_KEYS);
        return 1;
    }

    int *array = malloc(sizeof(int) * L);
    if (!array) {
        perror("Memory allocation failed");
        return 1;
    }

    srand(time(NULL));

    //fill with random positive numbers
    for (int i = 0; i < L; i++) {
        array[i] = rand() % MAX_POSITIVE_VALUE + 1;  //ensure > 0
    }
    //insert 80 unique hidden negative integers (-1 to -80) at random positions
    for (int i = 0; i < NUM_HIDDEN_KEYS; i++) {
        int pos;
        do {
            pos = rand() % L;
        } while (array[pos] < 0);  //ensure not overwriting another hidden key

        array[pos] = -(i + 1);  //insert -1 to -80
    }

    //write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("File opening failed");
        free(array);
        return 1;
    }

    for (int i = 0; i < L; i++) {
        fprintf(fp, "%d\n", array[i]);
    }
    fclose(fp);
    free(array);

    printf("Input file '%s' generated with %d numbers, including %d hidden keys.\n",
           filename, L, NUM_HIDDEN_KEYS);
    return 0;
}
