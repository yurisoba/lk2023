CFLAGS := -Wall -Werror -Wpedantic

stree: a-stree/main.c
	$(CC) $(CFLAGS) $^ -o $@

rbtest: a-stree/rbtest.c a-stree/rbtree.c
	$(CC) $(CFLAGS) $^ -o $@