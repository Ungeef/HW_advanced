tree *findKey(tree *node, int key) 
{
    if (node == NULL) {
        return NULL;
    }
    if(node->key == key)
    {
        return node;
    }

    tree *found_in_left = findKey(node->left, key);// Всегда идем налево 
    if(found_in_left != NULL)
    {
        return found_in_left;
    }
    
    return findKey(node->right, key);// Если ничего не нашли делаем шаг вправо 
    
}


tree *findBrother(tree *root, int key)
{
    tree *found = findKey(root, key);
    if(found == NULL)//Ничего не нашли
    {
        return NULL;
    }
    if (found->parent == NULL) // Проверка на корень
    {
        return NULL;
    }
    if(found->parent->left != found)//Проверка на NULL не нужна по условию
    {
        return found->parent->left;
    }
    else
    {
        return found->parent->right;
    }
}