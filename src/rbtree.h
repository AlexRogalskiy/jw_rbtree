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

typedef struct r_rbtree_node {
	struct r_rbtree_node *link[2];
	struct r_rbtree_node *parent;
	uint32_t red;
	void *data;
} RRBNode;

typedef int (*RRBComparator) (void *incoming, void *in, void *user);
typedef void (*RRBFree) (void *data);

typedef struct r_rbtree_t {
	RRBNode *root;
	size_t size;
	RRBFree free;
} RRBTree;

R_API RBTree *r_rbtree_new(RRBFree freefn);
R_API void r_rbtree_clear(RRBTree *tree);
R_API void r_rbtree_free(RRBTree *tree);
R_API RRBNode *r_rbtree_find_node(RRBTree *tree, void *data, RRBComparator cmp, void *user);
R_API void *r_rbtree_find(RRBTree *tree, void *data, RRBComparator cmp, void *user);
R_API bool r_rbtree_insert(RRBTree *tree, void *data, RRBComparator cmp, void *user);
R_API bool r_rbtree_delete(RRBTree *tree, void *data, RRBComparator cmp, void *user);
R_API RRBNode *r_rbtree_first_node(RRBTree *tree);
R_API RRBNode *r_rbtree_last_node(RRBTree *tree);
R_API RRBNode *r_rbnode_next(RRBNode *node);
R_API RRBNode *r_rbnode_prev(RRBNode *node);

#endif /* RBTREE_H */
