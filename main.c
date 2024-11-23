#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pair {
    char *key;
    struct pair *next;
} pair;

pair *pairs;
int word_count_per_process = 0;

void insert_word_pair(char *key) {

    if (pairs->key == NULL) {
        pairs->key = malloc(sizeof(key));
        strcpy(pairs->key, key);
        pairs->next = NULL;
    } else {
        pair *p = malloc(sizeof(pair));
        p->key = malloc(strlen(key) + 1);
        strcpy(p->key, key);
        p->next = pairs->next;
        pairs->next = p;
    }
}

void process_file(char *file_name) {
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1) {
        char *dummy = line;
        char *token = strtok(dummy, " \t\n\r");
        while (token != NULL) {
            insert_word_pair(token);
            word_count_per_process++;
            token = strtok(NULL, " \t\n\r");
        }
    }
    free(line);
    fclose(fp);
}

int count_and_get_next(char *key) {
    pair *p = pairs;
    int count = 0;
    while (p != NULL) {
        if (p->key != NULL && strcmp(p->key, key) == 0) {
            p->key = NULL;
            count++;
        }
        p = p->next;
    }
    return count;
}

void count(char *key) {
    int count = count_and_get_next(key);
    printf("%s %d\n", key, count);
}

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    //    printf("argc %d\n", argc);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int num_files = argc - 1;

    if(rank == 0){
        int operation_number = 1;
        int file_to_process = -1;
        while (operation_number != 0) {
            printf("\nEnter 0 to exit, 1 to count number of words in every file, 2 to count the number of word in one specific file "
                   ",3 to count the number of words in all files, 4 get number of words in one specific file\n");
            scanf("%d", &operation_number);
            if (operation_number == 1) {
              MPI_Bcast(&operation_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
            } else if (operation_number == 2 || operation_number == 4) {
                printf("\nEnter the file number to process\n");
                scanf("%d", &file_to_process);
                if (file_to_process < 1 || file_to_process > num_files) {
                    printf("Invalid file number\n");
                    continue;
                }
                MPI_Bcast(&operation_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
                MPI_Bcast(&file_to_process, 1, MPI_INT, 0, MPI_COMM_WORLD);
            } else if (operation_number == 3) {
                MPI_Bcast(&operation_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
                int global_word_count_per_process = 0;
                MPI_Reduce(&word_count_per_process, &global_word_count_per_process, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
                printf("Total number of words in all files is %d\n", global_word_count_per_process);
            }

            if (operation_number > 0 && operation_number < 5) {
                MPI_Barrier(MPI_COMM_WORLD);
            }
        }
    }
    int operation_number = -1;
    int file_to_process = -1;
    while (operation_number != 0) {
        if (operation_number != -1) {
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Bcast(&operation_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (operation_number == 2 || operation_number == 4) {
            MPI_Bcast(&file_to_process, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
        if (operation_number == 0) {
            break;
        }
        pairs = malloc(sizeof(pair));
        pairs->key = NULL;
        pairs->next = NULL;

        if (operation_number == 1 || operation_number == 3) {
            for (int i = 0; i < num_files; ++i) {
                int process_id = i % size;
                if (process_id == rank || (process_id == 0 && rank == 1)) {
                    process_file(argv[i + 1]);
                }
            }
        } else if (operation_number == 2 || operation_number == 4) {
            if (rank == 1) {
                process_file(argv[file_to_process]);
                if (operation_number == 4) {
                    printf("Number of words in the file %d is %d\n", file_to_process, word_count_per_process);
                    word_count_per_process = 0;
                    continue;
                }
            } else {
                continue;
            }
        }

        if (operation_number == 3) {
            MPI_Reduce(&word_count_per_process, &word_count_per_process, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            word_count_per_process = 0;
            continue;
        }
        char **local_words = malloc(sizeof(char *) * word_count_per_process);
        int i = 0;
        pair *p = pairs;
        while (p != NULL && p->key != NULL) {
            local_words[i] = malloc(strlen(p->key) + 1);
            strcpy(local_words[i], p->key);
            p = p->next;
            i++;
        }

        p = pairs;
        while (p != NULL) {
            if (p->key != NULL) {
                count(p->key);
            }
            p = p->next;
        }

        //// free the allocated memory
        while (pairs != NULL) {
            pair *temp = pairs;
            pairs = pairs->next;
            free(temp->key);
            free(temp);
        }

        word_count_per_process = 0;
    }

    /// TODO gather the results in process 0
    char **total_words;
    if (rank == 0) {
        /// array of words malloc
    }

    MPI_Finalize();
    return 0;
}
