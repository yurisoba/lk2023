#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Code is yanked out from <linux/rbtree.h>, with thread-safe facilities
 * stripped out for simplicity, making this an inferior version of linux
 * rbtree implementation. This is NOT thread-safe.
 * All functions are not augmented as well so dummy_* callbacks are not
 * needed. Functions marked as always-inlined become regular functions
 * for simplicity.
 * Literal comments are added to explain the obvious, this goes against
 * the kernel coding style, but this is also not kernel code and exists
 * only to educate. This approach provides mental check for any reader
 * so that any inconsistency will be made explicit and can then be dealt
 * with.
 */

struct rb_node {
	// parent color packs both parent and color
	uintptr_t __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
}; // __attribute__((aligned(sizeof(long))));
// the alignment is only needed for CRIS and we are not CRIS

struct rb_root {
	struct rb_node *rb_node;
};

enum rb_dir {
	LEFT, RIGHT
};

/*
 * rb_link_node updates pointers of a node and update rb_link value to
 * address of current node.
 * The rb_link parameter exists probably to maintain a consistent
 * signature with RCU-variant of rb_link_node.
 */
static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
	struct rb_node **rb_link)
{
	node->__rb_parent_color = (uintptr_t)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

void rb_insert_color(struct rb_node *node, struct rb_root *root);
void rb_erase(struct rb_node *node, struct rb_root *root);

#define RB_RED   0
#define RB_BLACK 1

#define rb_parent(r)       ((struct rb_node *)((r)->__rb_parent_color & ~3))
#define __rb_parent(pc)    ((struct rb_node *)(pc & ~3))

#define __rb_color(pc)     ((pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))
#define rb_color(rb)       __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)      __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)    __rb_is_black((rb)->__rb_parent_color)
