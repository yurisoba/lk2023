CFLAGS := -Wall -Werror -Wpedantic

stree: a-stree/main.c
	$(CC) $(CFLAGS) $^ -o $@

rbtest: a-stree/rbtest.c a-stree/rbtree.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: tree.png
tree.png: rbtest
	./rbtest || true
	dot -Tdot tree.gv | gvpr -c -f bintree.gvpr | neato -n -Tpng > $@
	dot -Tpng tree.gv > t.png

