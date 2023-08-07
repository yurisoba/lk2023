#include "rbtree.h"

struct rb_node *rb_red_parent(struct rb_node *red)
{
	return (struct rb_node *)red->__rb_parent_color;
}

/*
 * rb_set_parent sets p to be the parent of rb and retains the current
 * color of rb.
 */
void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->__rb_parent_color = rb_color(rb) + (uintptr_t)p;
}

/*
 * rb_set_parent_color updates rb's parent pointer without changing its
 * color.
 */
void rb_set_parent_color(struct rb_node *rb, struct rb_node *p, int color)
{
	// https://lore.kernel.org/all/20230404221350.3806566-1-goldstein.w.n@gmail.com/T/#u
	rb->__rb_parent_color = (uintptr_t)p + color;
}

void rb_set_black(struct rb_node *rb)
{
	rb->__rb_parent_color |= RB_BLACK;
}

/*
 * rb_change_child replaces pointer to old with new, and if parent
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

struct rb_node *__rb_erase_stage1(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *child = node->rb_right;
	struct rb_node *tmp = node->rb_left;
	struct rb_node *parent, *rebalance;
	unsigned long pc;

	if (!tmp) {
		/*
		 * Case 1: node to erase has no more than 1 child (easy!)
		 *
		 * Note that if there is one child it must be red due to 5)
		 * and node must be black due to 4). We adjust colors locally
		 * so as to bypass __rb_erase_color() later on.
		 */
		pc = node->__rb_parent_color;
		parent = __rb_parent(pc);
		__rb_change_child(node, child, parent, root);
		if (child) {
			child->__rb_parent_color = pc;
			rebalance = NULL;
		} else
			rebalance = __rb_is_black(pc) ? parent : NULL;
	} else if (!child) {
		/* Still case 1, but this time the child is node->rb_left */
		tmp->__rb_parent_color = pc = node->__rb_parent_color;
		parent = __rb_parent(pc);
		__rb_change_child(node, tmp, parent, root);
		rebalance = NULL;
	} else {
		struct rb_node *successor = child, *child2;

		tmp = child->rb_left;
		if (!tmp) {
			/*
			 * Case 2: node's successor is its right child
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (s)  ->  (x) (c)
			 *        \
			 *        (c)
			 */
			parent = successor;
			child2 = successor->rb_right;
		} else {
			/*
			 * Case 3: node's successor is leftmost under
			 * node's right child subtree
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (y)  ->  (x) (y)
			 *      /            /
			 *    (p)          (p)
			 *    /            /
			 *  (s)          (c)
			 *    \
			 *    (c)
			 */
			do {
				parent = successor;
				successor = tmp;
				tmp = tmp->rb_left;
			} while (tmp); // Q: is this really a loop??
			child2 = successor->rb_right;
			parent->rb_left = child2;
			successor->rb_right = child;
			rb_set_parent(child, successor);
		}

		tmp = node->rb_left;
		successor->rb_left = tmp;
		rb_set_parent(tmp, successor);

		pc = node->__rb_parent_color;
		tmp = __rb_parent(pc);
		__rb_change_child(node, successor, tmp, root);

		if (child2) {
			rb_set_parent_color(child2, parent, RB_BLACK);
			rebalance = NULL;
		} else
			rebalance = rb_is_black(successor) ? parent : NULL;
		successor->__rb_parent_color = pc;
	}

	return rebalance;
}

void __rb_erase_color(struct rb_node *parent, struct rb_root *root)
{
	struct rb_node *node = NULL, *sibling, *tmp1, *tmp2;

	while (1) {
		/*
		* Loop invariants:
		* - node is black (or NULL on first iteration)
		* - node is not the root (parent is not NULL)
		* - All leaf paths going through parent and node have a
		*   black node count that is 1 lower than other leaf paths.
		*/
		sibling = parent->rb_right;
		if (node != sibling) { /* node == parent->rb_left  */
			if (rb_is_red(sibling)) {
				/*
				 * Case 1 - left rotate at parent
				 *
				 *     P               S
				 *    / \             / \
				 *   N   s    -->    p   Sr
				 *      / \         / \
				 *     Sl  Sr      N   Sl
				 */
				tmp1 = sibling->rb_left;
				parent->rb_right = tmp1;
				sibling->rb_left = parent;
				rb_set_parent_color(tmp1, parent, RB_BLACK);
				__rb_rotate_set_parents(parent, sibling,
					root, RB_RED);
				sibling = tmp1;
			}
			tmp1 = sibling->rb_right;
			if (!tmp1 || rb_is_black(tmp1)) {
				tmp2 = sibling->rb_left;
				if (!tmp2 || rb_is_black(tmp2)) {
					/*
					 * Case 2 - sibling color flip
					 * (p could be either color here)
					 *
					 *    (p)           (p)
					 *    / \           / \
					 *   N   S    -->  N   s
					 *      / \           / \
					 *     Sl  Sr        Sl  Sr
					 *
					 * This leaves us violating 5) which
					 * can be fixed by flipping p to black
					 * if it was red, or by recursing at p.
					 * p is red when coming from Case 1.
					 */
					rb_set_parent_color(sibling, parent,
						RB_RED);
					if (rb_is_red(parent))
						rb_set_black(parent);
					else {
						node = parent;
						parent = rb_parent(node);
						if (parent)
							continue;
					}
					break;
				}
				/*
				 * Case 3 - right rotate at sibling
				 * (p could be either color here)
				 *
				 *   (p)           (p)
				 *   / \           / \
				 *  N   S    -->  N   sl
				 *     / \             \
				 *    sl  Sr            S
				 *                       \
				 *                        Sr
				 *
				 * Note: p might be red, and then both
				 * p and sl are red after rotation(which
				 * breaks property 4). This is fixed in
				 * Case 4 (in __rb_rotate_set_parents()
				 *         which set sl the color of p
				 *         and set p RB_BLACK)
				 *
				 *   (p)            (sl)
				 *   / \            /  \
				 *  N   sl   -->   P    S
				 *       \        /      \
				 *        S      N        Sr
				 *         \
				 *          Sr
				 */
				tmp1 = tmp2->rb_right;
				sibling->rb_left = tmp1;
				tmp2->rb_right = sibling;
				parent->rb_right = tmp2;
				if (tmp1)
					rb_set_parent_color(tmp1, sibling,
						RB_BLACK);
				tmp1 = sibling;
				sibling = tmp2;
			}
			/*
			 * Case 4 - left rotate at parent + color flips
			 * (p and sl could be either color here.
			 *  After rotation, p becomes black, s acquires
			 *  p's color, and sl keeps its color)
			 *
			 *      (p)             (s)
			 *      / \             / \
			 *     N   S     -->   P   Sr
			 *        / \         / \
			 *      (sl) sr      N  (sl)
			 */
			tmp2 = sibling->rb_left;
			parent->rb_right = tmp2;
			sibling->rb_left = parent;
			rb_set_parent_color(tmp1, sibling, RB_BLACK);
			if (tmp2)
				rb_set_parent(tmp2, parent);
			__rb_rotate_set_parents(parent, sibling, root,
				RB_BLACK);
			break;
		} else { /* same thing, but on the right */
			sibling = parent->rb_left;
			if (rb_is_red(sibling)) {
				/* Case 1 - right rotate at parent */
				tmp1 = sibling->rb_right;
				parent->rb_left = tmp1;
				sibling ->rb_right = parent;
				rb_set_parent_color(tmp1, parent, RB_BLACK);
				__rb_rotate_set_parents(parent, sibling,
					root, RB_RED);
				sibling = tmp1;
			}
			tmp1 = sibling->rb_left;
			if (!tmp1 || rb_is_black(tmp1)) {
				tmp2 = sibling->rb_right;
				if (!tmp2 || rb_is_black(tmp2)) {
					/* Case 2 - sibling color flip */
					rb_set_parent_color(sibling, parent,
						RB_RED);
					if (rb_is_red(parent))
						rb_set_black(parent);
					else {
						node = parent;
						parent = rb_parent(node);
						if (parent)
							continue;
					}
					break;
				}
				/* Case 3 - left rotate at sibling */
				tmp1 = tmp2->rb_left;
				sibling->rb_right = tmp1;
				tmp2->rb_left = sibling;
				parent->rb_left = tmp2;
				if (tmp1)
					rb_set_parent_color(tmp1, sibling,
						RB_BLACK);
				tmp1 = sibling;
				sibling = tmp2;
			}
			/* Case 4 - right rotate at parent + color flips */
			tmp2 = sibling->rb_right;
			parent->rb_left = tmp2;
			sibling->rb_right = parent;
			rb_set_parent_color(tmp1, sibling, RB_BLACK);
			if (tmp2)
				rb_set_parent(tmp2, parent);
			__rb_rotate_set_parents(parent, sibling, root,
				RB_BLACK);
			break;
		}
	}
}

void rb_erase(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *rebalance;
	rebalance = __rb_erase_stage1(node, root);
	if (rebalance)
		__rb_erase_color(rebalance, root);
}
