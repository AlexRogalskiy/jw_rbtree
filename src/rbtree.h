/*
 * Created 18A30
 *
 * Red-black tree implementation based on Julienne Walker's solution
 *
 * TRYME: using BSD internal rbtree implementation in <libkern/tree.h>
 *          instead of this crappy shit
 *
 * see:
 *  eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
 *  github.com/mirek/rb_tree
 *  github.com/sebhub/rb-bench
 *  en.wikipedia.org/wiki/Red–black_tree
 */

#ifndef RBTREE_H
#define RBTREE_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#define __MALLOC malloc
#define __FREE   free
#define __ASSERT assert
#define PCONV(x) (void *)((uintptr_t)(x))

typedef struct r_rbtree_node {
	struct r_rbtree_node *link[2];
	struct r_rbtree_node *parent;
	uint32_t red;
	void *data;
} RRBTreeNode;

typedef ptrdiff_t (*cmp_func_t) (void *, void *);
typedef void (*free_func_t) (void *);

typedef struct r_rbtree_t {
	RRBTreeNode *root;
	size_t size;
	cmp_func_t cmp;
	free_func_t free;
} RRBTree;

extern void *rb_not_found;

struct RRBTree *rb_alloc (cmp_func_t, free_func_t);
void rb_free (RRBTree *);
void rb_clear (struct RRBTree **);
size_t rb_size (const struct RRBTree *);
void *rb_get_unsafe (const struct RRBTree *, void *);
int rb_find (const struct RRBTree *, void *data);
bool rb_insert (struct RRBTree *, void *data);
bool rb_remove (struct RRBTree *, void *);

#endif /* RBTREE_H */
