#include <vector>
#include <string>
#include <iostream>

#include "tree.h"


using namespace std;

namespace L3 {

    vector<string> argRegInTree = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

    inline bool isLabel(TreeNode *root) {
        return root != NULL && root->value[0] == ':';
    }

    inline bool isReturn(TreeNode *root) {
        return root != NULL && root->value == "return";
    }

    inline bool isBranch(TreeNode *root) {
        return root != NULL && root->value == "br";
    }

    inline bool isCall(TreeNode *root) {
        return root != NULL && root->value == "call";
    }

    inline bool isStore(TreeNode *root) {
        return root != NULL && root->value == "store";
    }

    inline bool isLoad(TreeNode *root) {
        return root != NULL && root->value == "load";
    }

    inline bool isAddOrSub(TreeNode *root) {
        return root != NULL && (root->value == "+" || root->value == "-");
    }

    inline bool isOp(TreeNode *root) {
        return root != NULL && (root->value == "+" || root->value == "-" ||
                                root->value == "*" || root->value == "&" ||
                                root->value == "<<" || root->value == ">>");
    }

    inline bool isCmp(TreeNode *root) {
        return root != NULL && (root->value == "<" || root->value == "<=" ||
                                root->value == "=" || root->value == ">=" || root->value == ">");
    }

    inline bool isVar(TreeNode *root) {
        return root != NULL &&
               ((root->value[0] >= 'A' && root->value[0] <= 'Z') || (root->value[0] >= 'a' && root->value[0] <= 'z')) &&
               !isLabel(root) && !isReturn(root) && !isBranch(root) && !isCall(root) && !isStore(root) && !isLoad(root);
    }

    inline bool isNumber(TreeNode *root) {
        return root != NULL &&
               (root->value[0] == '+' || root->value[0] == '-' || (root->value[0] >= '0' && root->value[0] <= '9'));
    }

    inline bool isT(TreeNode *root) {
        return isNumber(root) || isVar(root);
    }

    inline bool isS(TreeNode *root) {
        return isT(root) || isLabel(root);
    }

    inline bool isU(TreeNode *root) {
        return isVar(root) || isLabel(root);
    }

    inline bool isCallee(TreeNode *root) {
        return root != NULL &&
               (isU(root) || root->value == "print" || root->value == "allocate" || root->value == "array-error");
    }


    bool movMatch(TreeNode *root) {
        return isVar(root) && isS(root->firstChild);
    }

    bool movOpMatch(TreeNode *root) {
        return isVar(root) && isOp(root->firstChild) &&
               isT(root->firstChild->firstChild) &&
               isT(root->firstChild->firstChild->nextSibling);
    }

    bool movCmpMatch(TreeNode *root) {
        return isVar(root) && isCmp(root->firstChild) &&
               isT(root->firstChild->firstChild) &&
               isT(root->firstChild->firstChild->nextSibling);
    }

    bool movLoadMatch(TreeNode *root) {
        return isVar(root) && isLoad(root->firstChild) && isVar(root->firstChild->firstChild);
    }

    bool storeMatch(TreeNode *root) {
        return isStore(root) && isVar(root->firstChild) && isS(root->firstChild->nextSibling);
    }

    bool brLabelMatch(TreeNode *root) {
        return isBranch(root) && isLabel(root->firstChild);
    }

    bool labelMatch(TreeNode *root) {
        return isLabel(root);
    }

    bool brVarLabelMatch(TreeNode *root) {
        return isBranch(root) && isVar(root->firstChild) &&
               isLabel(root->firstChild->nextSibling) &&
               isLabel(root->firstChild->nextSibling->nextSibling);
    }

    bool retMatch(TreeNode *root) {
        return isReturn(root);
    }

    bool retVarMatch(TreeNode *root) {
        return isReturn(root) && isVar(root->firstChild);
    }

    bool callMatch(TreeNode *root) {
        return isCall(root) && isCallee(root->firstChild);
    }

    bool movCallMatch(TreeNode *root) {
        return isVar(root) && callMatch(root->firstChild);
    }


    bool numTimesEight(TreeNode *root) {
        if (root == NULL) {
            return false;
        } else {
            long num = stoll(root->value);
            return num % 8 == 0;
        }
    }

    bool numIsOne(TreeNode *root) {
        if (root == NULL) {
            return false;
        } else {
            long num = stoll(root->value);
            return num == 1 || num == -1;
        }
    }

    bool ciscNum(TreeNode *root) {
        if (root == NULL) {
            return false;
        } else {
            long num = stoll(root->value);
            return num == 0 || num == 2 || num == 4 || num == 8;
        }
    }

    bool addOrSubForMemMatch(TreeNode *root) {
        return isVar(root) && isAddOrSub(root->firstChild) &&
               ((isVar(root->firstChild->firstChild) && numTimesEight(root->firstChild->firstChild->nextSibling))
                || (numTimesEight(root->firstChild->firstChild) && isVar(root->firstChild->firstChild->nextSibling)));
    }

    bool movMemMatch(TreeNode *root) {
        return movLoadMatch(root) && addOrSubForMemMatch(root->firstChild->firstChild);
    }

    bool memMovMatch(TreeNode *root) {
        return storeMatch(root) && addOrSubForMemMatch(root->firstChild);
    }

    bool opMatch(TreeNode *root) {
        return (movOpMatch(root) || movCmpMatch(root)) && (root->value == root->firstChild->firstChild->value ||
                                                           root->value ==
                                                           root->firstChild->firstChild->nextSibling->value);
    }

    bool memAddOrSubMatch(TreeNode *root) {
        return false;
    }

    bool AddOrSubMemMatch(TreeNode *root) {
        return (opMatch(root) && isAddOrSub(root->firstChild)) && (addOrSubForMemMatch(root->firstChild->firstChild) ||
                                                                   addOrSubForMemMatch(
                                                                           root->firstChild->firstChild->nextSibling));
    }

    bool cjumpMatch(TreeNode *root) {
        return brVarLabelMatch(root) && movCmpMatch(root->firstChild);
    }

    bool gotoMatch(TreeNode *root) {
        return brLabelMatch(root);
    }

    bool incOrDecMatch(TreeNode *root) {
        return opMatch(root) && isAddOrSub(root->firstChild) && (numIsOne(root->firstChild->firstChild) ||
                                                                 numIsOne(root->firstChild->firstChild->nextSibling));
    }

    int getArgNum(TreeNode *root) {
        int n = 0;
        TreeNode *cursor = root == NULL || root->firstChild == NULL ? NULL : root->firstChild->nextSibling;
        while (cursor != NULL) {
            cursor = cursor->nextSibling;
            n++;
        }
        return n;
    }

    bool isRunTimeCall(TreeNode *root) {
        return callMatch(root) && (root->firstChild->value == "print" || root->firstChild->value == "allocate" ||
                root->firstChild->value == "array-error");
    }




    void getInstFromTree(vector <string> &insts, TreeNode *root) {
        if (root == NULL) {
            return;
        }
        if (!labelMatch(root) && !retMatch(root) && root->firstChild == NULL) {
            return;
        }
        if (labelMatch(root)) {
            insts.push_back(root->value);
        } else if (retVarMatch(root)) {
            getInstFromTree(insts, root->firstChild);
            insts.push_back("(rax <- " + root->value + ")");
            insts.push_back("(return)");
        } else if (retMatch(root)) {
            insts.push_back("(return)");
        } else if (brLabelMatch(root)) {
            insts.push_back("(goto " + root->firstChild->value + ")");
        } else if (brVarLabelMatch(root)) {
            TreeNode *var = root->firstChild, *llb = var->nextSibling, *rlb = llb->nextSibling;
            if (cjumpMatch(root)) {
                TreeNode *cmp = var->firstChild, *l = cmp->firstChild, *r = l->nextSibling;
                getInstFromTree(insts, l);
                getInstFromTree(insts, r);
                insts.push_back("(cjump " + l->value + " " + cmp->value + " " + r->value + " " + llb->value + " " + rlb->value + ")");
            } else {
                getInstFromTree(insts, root->firstChild);
                insts.push_back("(cjump " + var->value + " = 0 " + rlb->value + " " + llb->value + ")");
            }
        } else if (storeMatch(root)) {
            TreeNode *var = root->firstChild, *s = root->firstChild->nextSibling;
            if (memMovMatch(root)) {
                TreeNode *op = var->firstChild, *l = op->firstChild, *r = l->nextSibling;
                getInstFromTree(insts, l);
                getInstFromTree(insts, r);
                insts.push_back("((mem " + l->value + " " + r->value + ") <- " + s->value + ")");
            } else {
                getInstFromTree(insts, var);
                if (!isLabel(s)) {
                    getInstFromTree(insts, s);
                }
                insts.push_back("((mem " + var->value + " 0) <- " + s->value + ")");
            }
        } else if (callMatch(root)) {
            int n = getArgNum(root);
            TreeNode *callee = root->firstChild, *argCursor = callee->nextSibling;
            for (; argCursor != NULL; argCursor = argCursor->nextSibling) {
                getInstFromTree(insts, argCursor);
            }
            if (!isRunTimeCall(root)) {
                insts.push_back("((mem rsp -8) <- " + callee->value + "_ret)");
            }
            argCursor = callee->nextSibling;
            for (int i = 0; i < 6 && argCursor != NULL; i++, argCursor = argCursor->nextSibling) {
                insts.push_back("(" + argRegInTree[i] + " <- " + argCursor->value + ")");
            }
            if (!isRunTimeCall(root)) {
                for (int sp = -16; argCursor != NULL; sp -= 8, argCursor = argCursor->nextSibling) {
                    insts.push_back("((mem rsp " + to_string(sp) + ") <- " + argCursor->value + ")");
                }
            }
            insts.push_back("(call " + callee->value + " " + to_string(n) + ")");
            if (!isRunTimeCall(root)) {
                insts.push_back(callee->value + "_ret");
            }
        } else if (movCallMatch(root)) {
            getInstFromTree(insts, root->firstChild);
            insts.push_back("(" + root->value + " <- rax)");
        } else if (movLoadMatch(root)) {
            TreeNode *var = root->firstChild->firstChild;
            if (movMemMatch(root)) {
                TreeNode *op = var->firstChild, *l = op->firstChild, *r = l->nextSibling;
                getInstFromTree(insts, l);
                getInstFromTree(insts, r);
                if (isNumber(l)) {
                    insts.push_back("(" + root->value + " <- (mem " + r->value + " " + l->value + "))");
                } else {
                    insts.push_back("(" + root->value + " <- (mem " + l->value + " " + r->value + "))");
                }
            } else {
                getInstFromTree(insts, var);
                insts.push_back("(" + root->value + " <- (mem " + var->value + " 0))");
            }
        } else if (movOpMatch(root)) {
            TreeNode *op = root->firstChild, *l = op->firstChild, *r = l->nextSibling;
            if (incOrDecMatch(root)) {
                if (root->firstChild->value == "+") {
                    insts.push_back("(" + root->value + "++)");
                } else {
                    insts.push_back("(" + root->value + "--)");
                }
            } else if (opMatch(root)) {
                if (l->value == root->value) {
                    getInstFromTree(insts, r);
                    insts.push_back("(" + root->value + " " + op->value + "= " + r->value + ")");
                } else {
                    getInstFromTree(insts, l);
                    insts.push_back("(" + root->value + " " + op->value + "= " + l->value + ")");
                }
            } else {
                getInstFromTree(insts, l);
                getInstFromTree(insts, r);
                insts.push_back("(" + root->value + " <- " + l->value + ")");
                insts.push_back("(" + root->value + " " + op->value + "= " + r->value + ")");
            }
        } else if (movCmpMatch(root)) {
            TreeNode *cmp = root->firstChild, *l = cmp->firstChild, *r = l->nextSibling;
            getInstFromTree(insts, l);
            getInstFromTree(insts, r);
            insts.push_back("(" + root->value + " <- " + l->value + " " + cmp->value + " " + r->value + ")");
        } else if (movMatch(root)) {
            getInstFromTree(insts, root->firstChild);
            insts.push_back("(" + root->value + " <- " + root->firstChild->value + ")");
        } else {
            cerr << "error matching" << endl;
        }
    }

    vector <string> getInstFromTree(TreeNode *tree) {
        vector <string> insts;
        getInstFromTree(insts, tree);
        return insts;
    }
}