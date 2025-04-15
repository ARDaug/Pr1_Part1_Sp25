#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#define MAX_KEYS 80

typedef struct {
    int max;
    double avg;
    int found_keys[MAX_KEYS];
    int num_found;
} Result;

//function to read array from file
int read_array(const char *filename, int **array) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("File open failed");
        return -1;
    }

    int size = 0, capacity = 1000;
    *array = malloc(capacity * sizeof(int));

    while (fscanf(fp, "%d", &(*array)[size]) == 1) {
        size++;
        if (size >= capacity) {
            capacity *= 2;
            *array = realloc(*array, capacity * sizeof(int));
        }
    }

    fclose(fp);
    return size;
}

Result process_chunk(int *array, int start, int end) {

    Result res;
    res.max = 0;
    res.avg = 0.0;
    res.num_found = 0;
    int sum = 0;

   for (int i = start; i < end; i++){
      if (array[i] > res.max)
          res.max = array[i];

      sum += array[i];

      if (array[i] < 0 && res.num_found < MAX_KEYS){ 
          res.found_keys[res.num_found++] = array[i];
      }
   }

   res.avg = sum/(end-start);

   return res;

}

void combine_parent_child(Result *parent, Result *child){

   if(child->max > parent->max)
      parent->max = child->max;


   parent->avg += child->avg;
   for (int i = 0; i < child->num_found; i++){
      parent->found_keys[parent->num_found++] = child->found_keys[i];      
   }

}


int main(int argc, char *argv[]){

   FILE *out;
out = fopen("output.txt", "a");
if (out) {
    fprintf(out, "Hi I’m process %d...\n", getpid());
    fclose(out);
}

   const char *filename = argv[1];
   int H = atoi(argv[2]);
   int PN = atoi(argv[3]);
   int *array;
   int size = read_array(filename, &array);
   int chunk_size = size/PN;

   int pipe_fd[2];
   pipe(pipe_fd);


   pid_t pid = fork();


   if (pid == 0) { //Child

      int current_id = 1;
      int start = 0;

      while (current_id < PN){

          int next_pipe[2];
          pipe(next_pipe);
          pid_t child_pid = fork();
           
                if(child_pid == 0){

                    close(next_pipe[1]);
                    pipe_fd[0] = next_pipe[0];
                    current_id++;

                    start = current_id * chunk_size;
                     
                     close(next_pipe[0]);
                     int end;
                         
                     if (current_id < PN-1)
                        end = size;
                     else
                        end = start + chunk_size;

                     Result res = process_chunk(array, start, end);

                     wait(NULL);


                     Result child_res;

                     read(pipe_fd[0], &child_res, sizeof(Result));
                  
                     combine_parent_child(&res, &child_res);

                     write(next_pipe[1], &res, sizeof(Result));
                     
                     FILE *out = fopen("output.txt", "a");                     

                     if (out) {
                        fprintf(out, "Hi I’m process %d with return arg %d and my parent is %d.\n", getpid(), current_id, getppid());

                        for (int i = 0; i < res.num_found && i < H; i++) {
                            fprintf(out, "Hi I am process %d and I found the hidden key in position A[%d].\n", getpid(), res.found_keys[i]);
                        }
                        fclose(out);
                     }
                     exit(current_id);
               }
      
      }
      int end;
      if(current_id < PN - 1)
         end = size;
      else
         end = start + chunk_size;

      Result res = process_chunk(array, start, end);
      write(pipe_fd[1], &res, sizeof(Result));

      FILE *out = fopen("output.txt", "a");
        if (out) {
            fprintf(out, "Hi I’m process %d with return arg %d and my parent is %d.\n", getpid(), current_id, getppid());

            for (int i = 0; i < res.num_found && i < H; i++) {
                fprintf(out, "Hi I am process %d and I found the hidden key in position A[%d].\n", getpid(), res.found_keys[i]);
            }

            fclose(out);
        }

      exit(current_id);

   } else{
     wait(NULL);
     Result final_res;
     read(pipe_fd[0], &final_res, sizeof(Result));
     FILE *out = fopen("output.txt", "a");
        if (out) {
            fprintf(out, "Max = %d, Avg = %.2f\n", final_res.max, final_res.avg);
            fclose(out);
        }
   }

free(array);

return 0;

}

























