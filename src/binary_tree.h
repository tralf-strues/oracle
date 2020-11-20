#pragma once

#include <stdarg.h>

typedef char* BTElem_t;

struct BTNode;
struct BinaryTree;

static const bool BT_TRAVERSE_RUN = true;

BinaryTree* newTree    ();
void        deleteTree (BinaryTree* tree);

BTNode*     newNode    ();
BTNode*     newNode    (BTElem_t value);
void        deleteNode (BTNode* node);

void        preOrderTraverse  (BTNode* subRoot, bool (*function)(BTNode* node, va_list args), ...);
void        inOrderTraverse   (BTNode* subRoot, bool (*function)(BTNode* node, va_list args), ...);
void        postOrderTraverse (BTNode* subRoot, bool (*function)(BTNode* node, va_list args), ...);

BTNode*     findNode (BinaryTree* tree, BTElem_t value);

BTNode*     getRoot (BinaryTree* tree);
void        setRoot (BinaryTree* tree, BTNode* root);

BTElem_t    getValue  (BTNode* node);
BTNode*     getParent (BTNode* node);
BTNode*     getLeft   (BTNode* node);
BTNode*     getRight  (BTNode* node);
bool        isLeft    (BTNode* node);

void        setValue  (BTNode* node, BTElem_t value);
void        setParent (BTNode* node, BTNode* parent);
void        setLeft   (BTNode* node, BTNode* left);
void        setRight  (BTNode* node, BTNode* right);

