CC = gcc
CFLAGS = -Wall -Wextra -O2 -lm

TARGETS = generate_input dfs_tree bfs_tree

all: $(TARGETS)

generate_input: generate_input.c
	$(CC) $(CFLAGS) -o generate_input generate_input.c

dfs_tree: dfs_process_tree.c
	$(CC) $(CFLAGS) -o dfs_tree dfs_process_tree.c

bfs_tree: bfs_tree.c
	$(CC) $(CFLAGS) -o bfs_tree bfs_tree.c

clean:
	rm -f $(TARGETS) output.txt

