#include "avl_tree.h"
//返回两个整数的较大值，用于高度计算
static int max(int a, int b) { return a > b ? a : b; }
//获取节点的子树高度，空节点返回 0
static int get_height(AVLNode *node) { return node ? node->height : 0; }
//更新节点的高度值，等于左右子树高度最大值 + 1
static void update_height(AVLNode *node) {
	if (node) node->height = max(get_height(node->left), get_height(node->right)) + 1;
}
//计算节点的平衡因子（左子树高度 - 右子树高度），判断是否失衡
static int get_balance(AVLNode *node) {
	return node ? get_height(node->left) - get_height(node->right) : 0;
}
//执行右旋转，处理「左左型」失衡，返回旋转后的新子树根
static AVLNode* right_rotate(AVLNode *y) {
	AVLNode *x = y->left;
	AVLNode *T2 = x->right;
	x->right = y;
	y->left = T2;
	update_height(y);
	update_height(x);
	return x;
}
//执行左旋转，处理「右右型」失衡，与右旋逻辑对称
static AVLNode* left_rotate(AVLNode *x) {
	AVLNode *y = x->right;
	AVLNode *T2 = y->left;
	y->left = x;
	x->right = T2;
	update_height(x);
	update_height(y);
	return y;
}
/*功能：递归向子树插入节点，插入后自底向上平衡
逻辑：按二叉搜索树规则递归插入，更新高度、检测平衡，根据失衡类型执行单旋或双旋。*/
static AVLNode* insert_node(AVLNode *node, CourseRecord record, int *success) {
	if (!node) {
		AVLNode *new_node = (AVLNode*)malloc(sizeof(AVLNode));
		if (!new_node) { *success = -1; return NULL; }
		new_node->data = record;
		new_node->left = new_node->right = NULL;
		new_node->height = 1;
		*success = 0;
		return new_node;
	}
	
	char key_new[21], key_node[21];
	make_key(record.student_id, record.course_id, key_new);
	make_key(node->data.student_id, node->data.course_id, key_node);
	int cmp = strcmp(key_new, key_node);
	
	if (cmp < 0) node->left = insert_node(node->left, record, success);
	else if (cmp > 0) node->right = insert_node(node->right, record, success);
	else { *success = -1; return node; }
	
	update_height(node);
	int balance = get_balance(node);
	
	if (balance > 1 && cmp < 0) return right_rotate(node);
	if (balance < -1 && cmp > 0) return left_rotate(node);
	if (balance > 1 && cmp > 0) {
		node->left = left_rotate(node->left);
		return right_rotate(node);
	}
	if (balance < -1 && cmp < 0) {
		node->right = right_rotate(node->right);
		return left_rotate(node);
	}
	return node;
}
/*功能：查找子树中最左侧节点（最小值节点），用于删除双子树节点时的替换。
*/
static AVLNode* find_min_node(AVLNode *node) {
	while (node && node->left) node = node->left;
	return node;
}
/*功能：递归删除指定主键的节点，删除后自动重新平衡
逻辑：分叶子节点、单子树节点、双子树节点三种情况处理，删除后校验平衡并旋转调整。
*/
static AVLNode* delete_node(AVLNode *node, const char *key, int *success) {
	if (!node) { *success = -1; return NULL; }
	
	char key_node[21];
	make_key(node->data.student_id, node->data.course_id, key_node);
	int cmp = strcmp(key, key_node);
	
	if (cmp < 0) node->left = delete_node(node->left, key, success);
	else if (cmp > 0) node->right = delete_node(node->right, key, success);
	else {
		*success = 0;
		if (!node->left || !node->right) {
			AVLNode *temp = node->left ? node->left : node->right;
			if (!temp) { temp = node; node = NULL; }
			else *node = *temp;
			free(temp);
		} else {
			AVLNode *temp = find_min_node(node->right);
			node->data = temp->data;
			char temp_key[21];
			make_key(temp->data.student_id, temp->data.course_id, temp_key);
			node->right = delete_node(node->right, temp_key, success);
		}
	}
	
	if (!node) return node;
	update_height(node);
	int balance = get_balance(node);
	
	if (balance > 1 && get_balance(node->left) >= 0) return right_rotate(node);
	if (balance > 1 && get_balance(node->left) < 0) {
		node->left = left_rotate(node->left);
		return right_rotate(node);
	}
	if (balance < -1 && get_balance(node->right) <= 0) return left_rotate(node);
	if (balance < -1 && get_balance(node->right) > 0) {
		node->right = right_rotate(node->right);
		return left_rotate(node);
	}
	return node;
}
/*功能：递归二分查找指定主键的节点，O (logn) 复杂度。
*/
static AVLNode* find_node(AVLNode *node, const char *key) {
	if (!node) return NULL;
	char key_node[21];
	make_key(node->data.student_id, node->data.course_id, key_node);
	int cmp = strcmp(key, key_node);
	if (cmp < 0) return find_node(node->left, key);
	else if (cmp > 0) return find_node(node->right, key);
	else return node;
}
/*功能：中序遍历子树，调用回调处理每条记录，输出结果天然按主键升序。*/
static void inorder(AVLNode *node, void (*cb)(CourseRecord, void*), void *arg) {
	if (!node) return;
	inorder(node->left, cb, arg);
	cb(node->data, arg);
	inorder(node->right, cb, arg);
}
/*功能：后序递归销毁子树所有节点，先释放子树再释放当前节点。*/
static void destroy_tree(AVLNode *node) {
	if (!node) return;
	destroy_tree(node->left);
	destroy_tree(node->right);
	free(node);
}

AVLTree* avl_tree_create() {
	AVLTree *tree = (AVLTree*)malloc(sizeof(AVLTree));
	if (!tree) return NULL;
	tree->root = NULL;
	tree->count = 0;
	return tree;
}

static int avl_insert(void *ds, CourseRecord record) {
	AVLTree *tree = (AVLTree*)ds;
	int success = -1;
	tree->root = insert_node(tree->root, record, &success);
	if (success == 0) tree->count++;
	return success;
}

static int avl_remove(void *ds, const char *sid, const char *cid) {
	AVLTree *tree = (AVLTree*)ds;
	char key[21];
	make_key(sid, cid, key);
	int success = -1;
	tree->root = delete_node(tree->root, key, &success);
	if (success == 0) tree->count--;
	return success;
}

static int avl_update(void *ds, const char *sid, const char *cid, int new_score) {
	char key[21];
	make_key(sid, cid, key);
	AVLNode *node = find_node(((AVLTree*)ds)->root, key);
	if (!node) return -1;
	node->data.score = new_score;
	return 0;
}

static CourseRecord* avl_find(void *ds, const char *sid, const char *cid) {
	char key[21];
	make_key(sid, cid, key);
	AVLNode *node = find_node(((AVLTree*)ds)->root, key);
	return node ? &(node->data) : NULL;
}

static void avl_traverse(void *ds, void (*cb)(CourseRecord, void*), void *arg) {
	inorder(((AVLTree*)ds)->root, cb, arg);
}

static int avl_size(void *ds) {
	return ((AVLTree*)ds)->count;
}

static void avl_destroy(void *ds) {
	AVLTree *tree = (AVLTree*)ds;
	destroy_tree(tree->root);
	free(tree);
}

DSInterface avl_tree_get_interface() {
	DSInterface iface;
	iface.insert   = avl_insert;
	iface.remove   = avl_remove;
	iface.update   = avl_update;
	iface.find     = avl_find;
	iface.traverse = avl_traverse;
	iface.size     = avl_size;
	iface.destroy  = avl_destroy;
	return iface;
}
