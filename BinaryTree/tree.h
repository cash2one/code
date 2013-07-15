#include <iostream>

using namespace std;

struct b_tree_node
{
    int key;
    b_tree_node *p_left;
    b_tree_node *p_right;
}

int get_node_num(b_tree_node *p_root)
{
    if(p_root == NULL)
        return 0;

    return get_node_num(p_root->p_left) + get_node_num(p_root->p_right) + 1;
}

int get_depth(b_tree_node *p_root)
{
    if(p_root == NULL)
        return 0;
    int left_depth = get_depth(p_root->p_left);
    int right_depth = get_depth(p_root->p_right);

    return left_depth > right_depth ? (left_depth + 1) : (left_depth + 1);
}
