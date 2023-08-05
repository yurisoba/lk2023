#include "rbtree.h"

#include <assert.h>
#include <stddef.h> /* offsetof */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - (offsetof(type, member))))

#define treeint_entry(ptr) container_of(ptr, struct treeint, st_n)

#define max(x, y) (x > y) ? (x) : (y)

struct treeint {
	int value;
	struct rb_node st_n;
};

static struct rb_root *tree = {NULL};

int treeint_init()
{
	tree = calloc(sizeof(struct rb_root), 1);
	assert(tree);
	return 0;
}

struct treeint *treeint_insert(int a)
{
	struct rb_node **new = &(tree->rb_node), *parent = NULL;
	while (*new) {
		struct treeint *t = container_of(*new, struct treeint, st_n);
		// this means we do not insert if an existing node with the
		// same value exists
		if (a == t->value)
			return t;
		parent = *new;
		if (a < t->value)
			new = &((*new)->rb_left);
		else
			new = &((*new)->rb_right);
	}
	// Q: why calloc instead of malloc?
	struct treeint *i = calloc(sizeof(struct treeint), 1);
	i->value = a;
	rb_link_node(&i->st_n, parent, new);
	rb_insert_color(&i->st_n, tree);
	return i;
}

// Q: why static?
static void __treeint_dump(struct rb_node *n, int depth)
{
	if (!n)
		return;

	/* in-order traversal */
	__treeint_dump(n->rb_left, depth + 1);

	struct treeint *v = treeint_entry(n);
	printf("%d\n", v->value);

	__treeint_dump(n->rb_right, depth + 1);
}

void treeint_dump()
{
	__treeint_dump(tree->rb_node, 0);
}

static int __treeint_depth(struct rb_node *n, int depth)
{
	if (!n)
		return depth;
	int ld = __treeint_depth(n->rb_left, depth + 1);
	int rd = __treeint_depth(n->rb_right, depth + 1);
	return max(ld, rd);
}

static int __treeint_pretty_dump(struct rb_node *n, int depth, int offset,
	char **line_arr, int *line_len)
{
	/* post-order traversal */
	int left_w = offset;
	if (n->rb_left)
		left_w = __treeint_pretty_dump(n->rb_left, depth + 1,
			offset, line_arr, line_len);

	for (int i = line_len[depth]; i < left_w; i++)
		line_arr[depth][i] = ' ';

	struct treeint *v = treeint_entry(n);
	int w = sprintf(&line_arr[depth][left_w], "%3d ",
		v->value);

	int right_w = left_w + w;
	line_len[depth] = right_w;
	if (n->rb_right)
		right_w = __treeint_pretty_dump(n->rb_right, depth + 1,
			right_w, line_arr, line_len);

	return right_w;
}

void treeint_pretty_dump()
{
	/*
	 * Sprintf each line into a line array
	 */
	int md = __treeint_depth(tree->rb_node, 0);
	char **line_arr = malloc(md * sizeof(char *));
	int *line_len = malloc(md * sizeof(int));
	for (int i = 0; i < md; i++) {
		line_arr[i] = malloc(256 * sizeof(char));
		line_len[i] = 0;
	}
	__treeint_pretty_dump(tree->rb_node, 0, 0, line_arr,
		line_len);
	for (int i = 0; i < md; i++) {
		puts(line_arr[i]);
		free(line_arr[i]);
	}
	free(line_arr);
	free(line_len);
}

int main()
{
	srand(time(0));

	treeint_init();

	for (int i = 0; i < 100; ++i)
		treeint_insert(rand() % 99);

	printf("[ After insertions ]\n");
	treeint_pretty_dump();

	return 0;
}