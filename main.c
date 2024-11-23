#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

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
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r")) != NULL) {
            insert_word_pair(token);
            word_count_per_process++;
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

//    pairs = malloc(sizeof(pair *));
    pairs = malloc(sizeof(pair));
    pairs->key = NULL;
    pairs->next = NULL;
    printf("rank %d\n", rank);
    printf("size %d\n", size);
    /// every process reads fair number of files and process them `DONE`

    int num_files = argc - 1;

    for (int i = 0; i < num_files; ++i) {
            int process_id = i % size;
            if (process_id == rank) {
                process_file(argv[i + 1]);
            }
        }
        char ** local_words = malloc(sizeof(char *) * word_count_per_process);
        int i = 0;
        pair *p = pairs;
        while (p != NULL && p->key != NULL) {
            local_words[i] = malloc(strlen(p->key) + 1);
            strcpy(local_words[i], p->key);
//            printf("%s\n", pairs->key);
            p = p->next;
            i++;
        }

//        printf("word count %d\n", word_count_per_process);
        p = pairs;
        while (p != NULL) {
            if (p->key != NULL) {
                count(p->key);
            }
            p = p->next;
        }


    /// TODO gather the results in process 0
    char **total_words;
    if (rank == 0) {
        /// array of words malloc
    }

    //// free the allocated memory
    while (pairs != NULL) {
        pair *temp = pairs;
        pairs = pairs->next;
        free(temp->key);
        free(temp);
    }

    MPI_Finalize();
    return 0;
}
