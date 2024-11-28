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

int count_and_return(char *key) {
    int count = count_and_get_next(key);
    return count;
}

void count_and_print(char *key) {
    int count = count_and_get_next(key);
    printf("%s %d\n", key, count);
}

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int num_files = argc - 1;

    if(rank == 0){
        int operation_number = 1;
        int file_to_process = -1;
        while (operation_number != 0) {
            printf("\nEnter 0 to exit\n1, to count number of words in each file separately\n2, to count the number of word in one specific file\n"
                   "3, to count the number of words in all files\n4, get number of words in one specific file\n"
                   "5, print summary and GENERATE a file\n\n");
            scanf("%d", &operation_number);
            printf("\n");
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
                MPI_Reduce(&word_count_per_process, &global_word_count_per_process, 1, MPI_INT,  MPI_SUM, 0, MPI_COMM_WORLD);
                printf("Total number of words in all files is %d\n", global_word_count_per_process);
            } else if (operation_number == 5){
                MPI_Bcast(&operation_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
            } else if (operation_number == 0) {
                MPI_Bcast(&operation_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
                break;
            } else {
                printf("Invalid operation number\n");
                continue;
            }

            if (operation_number > 0 && operation_number < 7) {
                MPI_Barrier(MPI_COMM_WORLD);
            }
        }
    }
    int operation_number = -1;
    int file_to_process = -1;
    while (operation_number != 0 && rank != 0) {
        if (operation_number != -1) {
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Bcast(&operation_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (operation_number == -1) {
            break;
        }
        if (operation_number == 2 || operation_number == 4) {
            MPI_Bcast(&file_to_process, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
        if (operation_number == 0) {
            break;
        }
        pairs = malloc(sizeof(pair));
        pairs->key = NULL;
        pairs->next = NULL;

        if (operation_number == 1 || operation_number == 3 || operation_number == 5) {
            // 10 file
            // 3 process
            // 0
            // 1 -> 0, 1, 3, 4, 7, 9
            // 2 -> 2, 5, 8
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
        pair *p;
        if (operation_number != 5) {
            p = pairs;
            while (p != NULL) {
                if (p->key != NULL) {
                    count_and_print(p->key);
                }
                p = p->next;
            }
        }

        if (operation_number == 5) {
            char *local_words = malloc(100 * word_count_per_process * sizeof(char));
            int i = 0;
            p = pairs;
            while (p != NULL) {
                if (p->key == NULL) {
                    p = p->next;
                    continue;
                }
                strcat(local_words, p->key);
                strcat(local_words, ",");
                char *count = malloc(10 * sizeof(char));
                sprintf(count, "%d", count_and_return(p->key));
                strcat(local_words, count);
                strcat(local_words, " ");
                if (i + 1 % 10 == 0) {
                    strcat(local_words, "\n");
                }
                p = p->next;
                i++;
            }
            if (word_count_per_process > 0) {
                puts(local_words);

                int temp_file_name = 3137 + rank;
                char *temp_file_name_str = malloc(10 * sizeof(char));
                sprintf(temp_file_name_str, "%d", temp_file_name);
                strcat(temp_file_name_str, ".txt");
                FILE *file = fopen(temp_file_name_str, "w");
                fprintf(file, "%s\n", local_words);
                fclose(file);
            }
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

    MPI_Finalize();
    return 0;
}
