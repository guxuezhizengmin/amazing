#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "common.h"

typedef struct AVLNode {
	CourseRecord data;
	struct AVLNode *left;
	struct AVLNode *right;
	int height;
} AVLNode;

typedef struct {
	AVLNode *root;
	int count;
} AVLTree;

AVLTree* avl_tree_create();
DSInterface avl_tree_get_interface();

#endif
