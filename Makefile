stree: a-stree/main.c
	$(CC) $^ -o $@

rbtest: a-stree/rbtest.c a-stree/rbtree.c
	$(CC) $^ -o $@