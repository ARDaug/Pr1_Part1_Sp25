/*
 *usage:
 *./bfs_tree L H PN
 *where
 *L = Number of integers in the file (must be at least 20,000)
 *H = Number of hidden keys to report (between 40 and 80)
 *PN = Maximum allowed processes (used to determine the process tree depth)
 *
 *NOTE: generate_input.c creates the file "input.txt". It writes exactly L numbers.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 #include <math.h>
 #include <errno.h>
 #include <string.h> 
 #define THRESHOLD 1000
 #define NUM_CHILDREN 2          
 #define MAX_HIDDEN_KEYS 80 
 typedef struct {
     int max;//max positive integer in segment.
     long long sum;//sum of positive integers in segment.
     int count;// Count of positive integers processed.
     int hidden_count;//number of hidden keys found in segment.
     int hidden_positions[MAX_HIDDEN_KEYS]; //global positions where hidden keys were found.
 } result_t;

 result_t processSegment(int *arr, int start, int end, int depth, int max_depth, int unique_code) {
     //process start message
     printf("Hi I'm process %d with return arg %d and my parent is %d.\n",
            getpid(), unique_code, getppid());
     fflush(stdout); 
     result_t res;
     res.max = 0;//positive values > 0.
     res.sum = 0;
     res.count = 0;
     res.hidden_count = 0;     
     int segment_size = end - start; 
     //fork children or to process the segment directly.
     if ((depth < max_depth) && (segment_size > THRESHOLD)) {
         int num_children = NUM_CHILDREN;
         int partition_size = segment_size / num_children;         
         result_t combined;
         combined.max = 0;
         combined.sum = 0;
         combined.count = 0;
         combined.hidden_count = 0;         
         for (int i = 0; i < num_children; i++) {
             int c_start = start + i * partition_size;
             int c_end = (i == num_children - 1) ? end : c_start + partition_size;             
             int pipefd[2];
             if (pipe(pipefd) == -1) {
                 perror("pipe");
                 exit(EXIT_FAILURE);
             }             
             pid_t pid = fork();
             if (pid < 0) {
                 perror("fork");
                 exit(EXIT_FAILURE);
             }             
             if (pid == 0) {
                 //child
                 int child_code = unique_code * 10 + (i + 1);
                 result_t child_res = processSegment(arr, c_start, c_end, depth + 1, max_depth, child_code);                 
                 //write computed results to parent via pipe
                 close(pipefd[0]); //close unused read end
                 if (write(pipefd[1], &child_res, sizeof(result_t)) != sizeof(result_t)) {
                     perror("write");
                     exit(EXIT_FAILURE);
                 }
                 close(pipefd[1]);
                 //sleep to make the process tree visible before termination
                 sleep(2);
                 exit(child_code);  //unique return code
             } else {
                 //parent process: read child's results
                 close(pipefd[1]); //close unused write end.
                 result_t child_res;
                 ssize_t bytesRead = read(pipefd[0], &child_res, sizeof(result_t));
                 if (bytesRead != sizeof(result_t)) {
                     fprintf(stderr, "Warning: read %zd bytes instead of %zu from child %d.\n",
                             bytesRead, sizeof(result_t), pid);
                 }
                 close(pipefd[0]);                 
                 //wait for child to terminate and print exit
                 int status;
                 if (waitpid(pid, &status, 0) == -1) {
                     perror("waitpid");
                     exit(EXIT_FAILURE);
                 }
                 if (WIFEXITED(status)) {
                     int exit_code = WEXITSTATUS(status);
                     printf("Child process %d terminated with return code %d.\n", pid, exit_code);
                     fflush(stdout);
                 } else {
                     printf("Child process %d did not terminate normally.\n", pid);
                     fflush(stdout);
                 }                 
                 //combine child's results into the parent
                 if ((child_res.count > 0) && (child_res.max > combined.max))
                     combined.max = child_res.max;
                 else if (combined.count == 0)
                     combined.max = child_res.max;
                 
                 combined.sum += child_res.sum;
                 combined.count += child_res.count;
                 for (int j = 0; j < child_res.hidden_count; j++) {
                     if (combined.hidden_count < MAX_HIDDEN_KEYS) {
                         combined.hidden_positions[combined.hidden_count++] = child_res.hidden_positions[j];
                     }
                 }
             }
         }  //end for each child         
         printf("Process %d at depth %d combining children results: Max=%d, Sum=%lld, Count=%d, Hidden Keys=%d.\n",
                getpid(), depth, combined.max, combined.sum, combined.count, combined.hidden_count);
         fflush(stdout);
         sleep(2);
         return combined;
     } else {
         //process the segment directly.
         for (int i = start; i < end; i++) {
             if (arr[i] > 0) {  // Process positive numbers
                 res.sum += arr[i];
                 res.count++;
                 if (arr[i] > res.max)
                     res.max = arr[i];
             } else {  //negative number indicates hidden key
                 if (res.hidden_count < MAX_HIDDEN_KEYS) {
                     res.hidden_positions[res.hidden_count] = i;  //global index in array
                     res.hidden_count++;
                 }
                 printf("Hi I am process %d with return arg %d and I found the hidden key in position A[%d].\n",
                        getpid(), unique_code, i);
                 fflush(stdout);
             }
         }
         sleep(2);
         return res;
     }
 } 
 int main(int argc, char *argv[]) {
     if (argc < 4) {
         fprintf(stderr, "Usage: %s L H PN\n", argv[0]);
         exit(EXIT_FAILURE);
     }     
     int L = atoi(argv[1]);  //total number of integers in file
     int H = atoi(argv[2]);  //number of hidden keys
     int PN = atoi(argv[3]); //max number of processes     
     if (L < 20000) {
         fprintf(stderr, "L must be at least 20000. You passed: %d\n", L);
         exit(EXIT_FAILURE);
     }
     if ((H < 40) || (H > 80)) {
         fprintf(stderr, "H must be between 40 and 80. You passed: %d\n", H);
         exit(EXIT_FAILURE);
     }     
     //compute max tree depth based on PN and  branching factor
     int max_depth = 0;
     if (PN > 1)
         max_depth = (int)floor(log2(PN + 1)) - 1;
     //in PN == 1,  array is processed in a single process 
     //L directly used --generate_input.c writes exactly L numbers
     int total_numbers = L;
     
     int *data = malloc(total_numbers * sizeof(int));
     if (data == NULL) {
         perror("malloc");
         exit(EXIT_FAILURE);
     }     
     //open the input file "input.txt" that was generated by generate_input.c
     FILE *fp = fopen("input.txt", "r");
     if (fp == NULL) {
         fprintf(stderr, "Error opening input file 'input.txt': %s\n", strerror(errno));
         free(data);
         exit(EXIT_FAILURE);
     }     
     //readexactly L integers from the file
     for (int i = 0; i < total_numbers; i++) {
         if (fscanf(fp, "%d", &data[i]) != 1) {
             fprintf(stderr, "Error reading integer number %d from the file.\n", i);
             fclose(fp);
             free(data);
             exit(EXIT_FAILURE);
         }
     }
     fclose(fp);     
     //start processing the array with the entire segment
     //root process uses a unique code of 1
     result_t finalRes = processSegment(data, 0, total_numbers, 0, max_depth, 1);     
     //overall average from positive numbers only.
     double average = (finalRes.count > 0) ? ((double)finalRes.sum / finalRes.count) : 0.0;     
     printf("\nFinal Metrics from Root Process %d:\n", getpid());
     printf("Max = %d, Avg = %.2f\n", finalRes.max, average);     
     // Print the positions of the first H hidden keys (if available).
     int keysToPrint = (finalRes.hidden_count >= H) ? H : finalRes.hidden_count;
     printf("Hidden keys found (first %d positions):\n", keysToPrint);
     for (int i = 0; i < keysToPrint; i++) {
         printf("A[%d]  ", finalRes.hidden_positions[i]);
     }
     printf("\n");     
     //display the process tree
     printf("\nProcess tree (pstree output):\n");
     char pstreeCommand[100];
     snprintf(pstreeCommand, sizeof(pstreeCommand), "pstree -p %d", getpid());
     system(pstreeCommand);     
     //allow inspection of the process tree.
     sleep(6);     
     free(data);     
     return 0;
 }
 