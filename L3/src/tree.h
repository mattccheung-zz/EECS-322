#pragma once

#include <vector>
#include <string>

using namespace std;


struct TreeNode;

struct TreeNode {
    string value;
    vector<TreeNode *> children;

    TreeNode(const string &v) {
        value = v;
    }
};