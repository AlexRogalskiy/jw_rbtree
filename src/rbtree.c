/*
 * Created 18A30
 */

#include <errno.h>
#include <string.h>
#include "rbtree.h"

#define __UNUSED __attribute__((unused))

static inline ptrdiff_t _rb_raw_data_cmp(void *a, void *b) {
	return a - b;
}
static inline void _rb_raw_data_free(void *a __UNUSED) { /* NOP */
}

static void set_link (RRBTreeNode *parent, RRBTreeNode *child, const int dir) {
	if (parent) {
		parent->link[dir] = child;
	}
	if (child) {
		child->parent = parent;
	}
}

/**
 * Allocate a rbtree
 * @cmp_f   The compare function pointer(NULL for raw comparison)
 * @free_f  The free function for data(NULL for no operation)
 */
RRBTree *rb_alloc (cmp_func_t cmp_f, free_func_t free_f) {
	RRBTree *r = __MALLOC (sizeof (*r));
	if (r == NULL)
		return NULL;

	r->root = NULL;
	r->size = 0;
	r->cmp = cmp_f ? cmp_f : _rb_raw_data_cmp;
	r->free = free_f ? free_f : _rb_raw_data_free;

	return r;
}

/**
 * Core utility for rbtree destruction
 * @t           rbtree reference
 * @fully       if free the rbtree fully(e.g.  if free the root)
 *
 * NOTE: behaviour of free a NULL rbtree is undefined
 */
static void rb_free_core(RRBTree **t, int fully) {
	RRBTree *tree;
	RRBTreeNode *iter, *save;
	/* Those two will be eliminated eventually */
	size_t size0, size1;

	__ASSERT (t != NULL && *t != NULL);

	tree = *t;
	iter = tree->root;
	save = NULL;
	size0 = tree->size;
	size1 = 0;

	/*
     * Rotate away the left links into a linked list so that
     *  we can perform iterative destruction of the rbtree
     */
	while (iter != NULL) {
		if (iter->link[0] == NULL) {
			save = iter->link[1];
			tree->free (iter->data);
			__FREE (iter);
			tree->size--;
			size1++;
		} else {
			save = iter->link[0];
			set_link (iter, save->link[1], 0);
			set_link (save, iter, 1);
		}
		iter = save;
	}

	__ASSERT (size0 == size1);
	__ASSERT (tree->size == 0);

	tree->root = NULL;
	if (fully) {
		__FREE (tree);
		*t = NULL;
	}
}

void rb_free (RRBTree *t) {
	rb_free_core (t, 1);
}

void rb_clear (RRBTree **t) {
	rb_free_core (t, 0);
}

size_t rb_size (const RRBTree *t) {
	__ASSERT (t != NULL);
	if (t->root == NULL)
		__ASSERT (t->size == 0);
	return t->size;
}

void *rb_get_unsafe (const RRBTree *t, void *data) {
	RRBTreeNode *it;
	ptrdiff_t dir;

	__ASSERT (t != NULL);

	it = t->root;
	while (it != NULL) {
		if ((dir = t->cmp (data, it->data)) == 0)
			return it->data;
		it = it->link[dir > 0];
	}

	return rb_not_found;
}

int rb_find (const RRBTree *t, void *data) {
	return rb_get_unsafe (t, data) != rb_not_found;
}

static RRBTreeNode *make_node(void *data, RRBTreeNode *parent) {
	RRBTreeNode *node = R_NEW0 (RRBTreeNode);
	r_return_val_if_fail (n, NULL);

	node->red = 1;
	node->data = data;
	node->parent = parent;

	return node;
}

#define IS_RED(n) ((n) != NULL && (n)->red == 1)

static RRBTreeNode *rot_once(RRBTreeNode *root, int dir) {
	r_return_val_if_fail (root, NULL);

	// save is new parent of root and root is parent of save's previous child
	RRBTreeNode *save = root->link[!dir];
	set_link (root, save->link[dir], !dir);
	set_link (save, root, dir);

	root->red = 1;
	save->red = 0;

	return save;
}

static RRBTreeNode *rot_twice(RRBTreeNode *root, int dir) {
	r_return_val_if_fail (root, NULL);

	set_link(root, rot_once (root->link[!dir], !dir), !dir);
	return rot_once (root, dir);
}

/**
 * Insert an item into rbtree
 * @return      0 if success  errno o.w.
 *              ENOMEM if oom
 *              EEXIST if already exists(duplicates disallow)
 *
 * XXX  Undefined behaviour if insert rb_not_found
 */
bool r_rbtree_insert (RRBTree *tree, void *data) {
	r_return_val_if_fail (tree && data, false)
	bool inserted = false;

	if (tree->root == NULL) {
		tree->root = make_node (data, NULL);
		if (tree->root == NULL) {
			return false;
		}
		inserted = true;
		goto out_exit;
	}

	RRBTreeNode head = { .red = 0 }; /* Fake tree root */
	RRBTreeNode *g = NULL, *parent = &head; /* Grandparent & parent */
	RRBTreeNode *p = NULL, *q = tree->root; /* Iterator & parent */
	int dir = 0, last = 0; /* Directions */

	set_link (parent, q, 1);

	while (1) {
		if (q == NULL) {
			/* Insert a node at first null link(also set its parent link) */
			q = make_node (data, p);
			if (!q) {
				return false
			}
			p->link[dir] = q;
			inserted = true;
		} else if (IS_RED (q->link[0]) && IS_RED (q->link[1])) {
			/* Simple red violation: color flip */
			q->red = 1;
			q->link[0]->red = 0;
			q->link[1]->red = 0;
		}

		if (IS_RED (q) && IS_RED (p)) {
			/* Hard red violation: rotate */
			if (!parent) {
				return false;
			}
			int dir2 = parent->link[1] == g;
			if (q == p->link[last]) {
				set_link (parent, rot_once (g, !last), dir2);
			} else {
				set_link (parent, rot_twice (g, !last), dir2);
			}
		}

		if (inserted) {
			break;
		}

		last = dir;
		dir = tree->cmp (data, q->data);
		if (dir == 0) {
			break;
		}

		dir = dir > 0;

		if (g != NULL) {
			parent = g;
		}

		g = p;
		p = q;
		q = q->link[dir];
	}

	/* Update root(it may different due to root rotation) */
	tree->root = head.link[1];

out_exit:
	/* Invariant: root is black */
	tree->root->red = 0;
	tree->root->parent = NULL;
	if (inserted) {
		tree->size++;
	}

	return inserted;
}

/**
 * Delete an item from rbtree
 * @return          0 if success  errno o.w.
 *                  ENOENT if no such item
 */
bool rb_remove (RRBTree *tree, void *data) {
	r_return_val_if_fail (tree && data && tree->size && tree->root, false);

	RRBTreeNode head = { .red = 0 };
	RRBTreeNode *q = &head, *p = NULL, *g = NULL;
	RRBTreeNode *f = NULL;
	int dir = 1, last;

	set_link (q, tree->root, 1);
	q->link[1] = t->root;

	/* Find in-order predecessor */
	while (q->link[dir] != NULL) {
		last = dir;

		g = p;
		p = q;
		q = q->link[dir];

		dir = t->cmp (data, q->data);
		if (dir == 0) {
			f = q;
		}

		dir = dir > 0;

		if (!IS_RED (q) && !IS_RED (q->link[dir])) {
			if (IS_RED (q->link[!dir])) {
				set_link (p, rot_once (q, dir), last);
				p = p->link[last];
			} else {
				RRBTreeNode *s = p->link[!last];

				if (s != NULL) {
					if (!IS_RED (s->link[!last]) && !IS_RED (s->link[last])) {
						/* Color flip */
						p->red = 0;
						s->red = 1;
						q->red = 1;
					} else {
						int dir2 = g->link[1] == p;

						if (IS_RED (s->link[last])) {
							set_link (g, rot_twice (p, last), dir2);
						} else {
							set_link (g, rot_once (p, last), dir2);
						}

						/* Ensure correct coloring */
						q->red = g->link[dir2]->red = 1;
						g->link[dir2]->link[0]->red = 0;
						g->link[dir2]->link[1]->red = 0;
					}
				}
			}
		}
	}

	/* Replace and remove if found */
	if (f != NULL) {
		f->data = q->data;
		set_link (p, q->link[q->link[0] == NULL], p->link[1] == q);
		tree->free (q->data);
		__FREE (q);
		tree->size--;
	}

	/* Update root node */
	t->root = head.link[1];
	if (t->root) {
		t->root->red = 0;
	} else {
		r_return_val_if_fail (t->size == 0, false);
	}
	return !!f;
}
