#pragma once

#include <vector>
#include <string>

using namespace std;


struct TreeNode;

struct TreeNode {
    string value;
    TreeNode *firstChild;
    TreeNode *nextSibling;

    TreeNode(const string &v) {
        value = v;
        firstChild = NULL;
        nextSibling = NULL;
    }
};

