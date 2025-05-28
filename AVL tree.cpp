#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura para os dados do terremoto
typedef struct {
    char date[20];
    char city[50];
    char mag[10];
} Registro;

// Estrutura do nó da árvore AVL
typedef struct AVLNode {
    Registro data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

// Funções auxiliares
int max(int a, int b) {
    return (a > b) ? a : b;
}

int height(AVLNode* node) {
    return node ? node->height : 0;
}

AVLNode* createNode(Registro data) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

// Rotações
AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

int getBalance(AVLNode* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

AVLNode* insert(AVLNode* node, Registro data) {
    if (!node) return createNode(data);

    double mag = atof(data.mag);
    double nodeMag = atof(node->data.mag);

    if (mag < nodeMag)
        node->left = insert(node->left, data);
    else
        node->right = insert(node->right, data);

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = getBalance(node);

    if (balance > 1 && mag < atof(node->left->data.mag))
        return rotateRight(node);
    if (balance < -1 && mag > atof(node->right->data.mag))
        return rotateLeft(node);
    if (balance > 1 && mag > atof(node->left->data.mag)) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && mag < atof(node->right->data.mag)) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

void searchAVL(AVLNode* node, const char* date, const char* region, double minMag, double maxMag) {
    if (!node) return;

    double nodeMag = atof(node->data.mag);
    int match = 1;

    if (date && strcmp(date, node->data.date) != 0)
        match = 0;
    if (region && strcmp(region, node->data.city) != 0)
        match = 0;
    if (minMag != 0.0 || maxMag != 0.0) {
        if (nodeMag < minMag || nodeMag > maxMag)
            match = 0;
    }

    if (match) {
        printf("Data: %s, Cidade: %s, Magnitude: %s\n", node->data.date, node->data.city, node->data.mag);
    }

    searchAVL(node->left, date, region, minMag, maxMag);
    searchAVL(node->right, date, region, minMag, maxMag);
}

