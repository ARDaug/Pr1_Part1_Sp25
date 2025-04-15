# Pr1_Part1_Sp25
## Project Files
- generate_input.c:  
  Generates an `input.txt` file. In this file, L positive integers are generated, and 80 unique negative integers (hidden keys: -1 to -80) are inserted at random positions.
- dfs_process_tree.c:  
  Implements a DFS (chain-like) process tree where each process processes a chunk of the data, reports its local metrics, and passes the information up to its parent.
- bfs_tree.c:  
  Implements a BFS process tree where each internal process forks multiple child processes, partitions the data among them, and aggregates the results. This version displays the process tree using the `pstree` command.
- Makefile:  
  A Makefile to compile all above programs.
## How to compile
Run:

bash
make

This produces the executables: `generate_input`, `dfs_tree`, and `bfs_tree`.

## Running the Programs

### 1. Generate the Input File

using `generate_input.c`, supply two arguments:
- L: The number of integers to generate
- output_filename: `input.txt`.

Example, to generate 20,000 numbers:

./generate_input 20000 input.txt

Expected ouptut: 
Input file 'input.txt' generated with 20000 numbers, including 80 hidden keys.

Run BFS tree:
./bfs_tree 20000 40 7

Run DFS tree:
./dfs_tree input.txt 40 7

