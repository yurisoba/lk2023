#include "rbtree.h"

struct rb_node *rb_red_parent(struct rb_node *red)
{
	return (struct rb_node *)red->__rb_parent_color;
}

/*
 * rb_set_parent_color sets p to be the parent of rb and pack the color
 * of rb into the parent pointer.
 */
void rb_set_parent_color(struct rb_node *rb, struct rb_node *p, int color)
{
	// https://lore.kernel.org/all/20230404221350.3806566-1-goldstein.w.n@gmail.com/T/#u
	rb->__rb_parent_color = (unsigned long)p + color;
}

/*
 * rb_change_child replace pointer to old with new, and if parent
 * does not exist, update the root instead. Only one of the parent
 * children can be old.
 */
void __rb_change_child(struct rb_node *old, struct rb_node *new,
	struct rb_node *parent, struct rb_root *root)
{
	if (parent) {
		if (parent->rb_left == old)
			parent->rb_left = new;
		else
			parent->rb_right = new;
	} else
		root->rb_node = new;
}

void __rb_rotate_set_parents(struct rb_node *old, struct rb_node *new,
	struct rb_root *root, int color)
{
	struct rb_node *parent = rb_parent(old);
	new->__rb_parent_color = old->__rb_parent_color;
	rb_set_parent_color(old, new, color);
	__rb_change_child(old, new, parent, root);
}

void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *parent = rb_red_parent(node), *gparent, *tmp;

	while (1) {
		/*
		 * Loop invariant: node is red.
		 */
		if (!parent) {
			rb_set_parent_color(node, NULL, RB_BLACK);
			break;
		}

		/*
		 * If there is a black parent, we are done.
		 * Otherwise, take some corrective action as,
		 * per 4), we don't want a red root of two
		 * consecutive red nodes.
		 */
		if (rb_is_black(parent))
			break;

		gparent = rb_red_parent(parent);

		/*
		 * Note for graph in comments:
		 * - upper case: BLACK node
		 * - lower case: RED node
		 */
		tmp = gparent->rb_right;
		if (parent != tmp) {    /* parent == gparent->rb_left */
			if (tmp && rb_is_red(tmp)) {
				/*
				 * Case 1 - node's uncle is red (color flips).
				 *
				 *       G            g
				 *      / \          / \
				 *     p   u  -->   P   U
				 *    /            /
				 *   n            n
				 *
				 * However, since g's parent might be red, and
				 * 4) does not allow this, we need to recurse
				 * at g.
				 */
				rb_set_parent_color(tmp, gparent, RB_BLACK);
				rb_set_parent_color(parent, gparent, RB_BLACK);
				node = gparent;
				parent = rb_parent(node);
				rb_set_parent_color(node, parent, RB_RED);
				continue;
			}

			tmp = parent->rb_right;
			if (node == tmp) {
				/*
				 * Case 2 - node's uncle is black and node is
				 * the parent's right child (left rotate at parent).
				 *
				 *      G             G
				 *     / \           / \
				 *    p   U  -->    n   U
				 *     \           /
				 *      n         p
				 *
				 * This still leaves us in violation of 4), the
				 * continuation into Case 3 will fix that.
				 */
				tmp = node->rb_left;
				parent->rb_right = tmp;
				node->rb_left = parent;
				if (tmp)
					rb_set_parent_color(tmp, parent,
						RB_BLACK);
				rb_set_parent_color(parent, node, RB_RED);
				parent = node;
				tmp = node->rb_right;
			}
			/*
			 * Case 3 - node's uncle is black and node is
			 * the parent's left child (right rotate at gparent).
			 *
			 *        G           P
			 *       / \         / \
			 *      p   U  -->  n   g
			 *     /                 \
			 *    n                   U
			 */
			gparent->rb_left = tmp; /* == parent->rb_right */
			parent->rb_right = gparent;
			if (tmp)
				rb_set_parent_color(tmp, gparent, RB_BLACK);
			__rb_rotate_set_parents(gparent, parent, root, RB_RED);
			break;
		} else { /* do everything above but on rb_left instead */
			tmp = gparent->rb_left;
			if (tmp && rb_is_red(tmp)) {
				/* exactly Case 1 */
				rb_set_parent_color(tmp, gparent, RB_BLACK);
				rb_set_parent_color(parent, gparent, RB_BLACK);
				node = gparent;
				parent = rb_parent(node);
				rb_set_parent_color(node, parent, RB_RED);
				continue;
			}

			tmp = parent->rb_left;
			if (node == tmp) {
				/* Case 2, right rotate at parent */
				tmp = node->rb_right;
				parent->rb_left = tmp;
				node->rb_right = parent;
				if (tmp)
					rb_set_parent_color(tmp, parent,
						RB_BLACK);
				rb_set_parent_color(parent, node, RB_RED);
				parent = node;
				tmp = node->rb_left;
			}

			/* Case 3, left rotate at gparent */
			gparent->rb_right = tmp; /* == parent->rb_left */
			parent->rb_left = gparent;
			if (tmp)
				rb_set_parent_color(tmp, gparent, RB_BLACK);
			__rb_rotate_set_parents(gparent, parent, root, RB_RED);
			break;
		}
	}
}
