static void printLeftReverseUtil(tree *node) 
{
    if (node == NULL) {
        return;
    }
    printLeftReverseUtil(node->left);

    printf("%d ", node->key);
}

void btUpView(tree *root) 
{
    if (root == NULL) {
        return;
    }

    printLeftReverseUtil(root->left);


    printf("%d ", root->key);

    tree *p = root->right; 
    while (p != NULL) {
        printf("%d ", p->key);
        p = p->right;
    }
    
    printf("\n");
}