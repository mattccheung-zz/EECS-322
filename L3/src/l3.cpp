#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stack>

#include "l3.h"

using namespace std;

namespace L3 {

    vector <string> argReg = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

    TreeNode *mergeTwoTree(TreeNode *up, TreeNode *down) {
        /*
         * This is a depth-first-search algorithm.
         * It will search leaf node of the up side tree, and tries to match the root node
         * of down side tree. If matched, it will delete the root of down tree, and link
         * down tree to the up tree, then return.
         *
         * It will just match once, even though it is possible to have multiple shared
         * variables in the same instruction.
         * */
        stack<TreeNode *> st;
        st.push(up);
        while (!st.empty()) {
            TreeNode *node = st.top();
            st.pop();
            if (node->nextSibling != NULL) {
                st.push(node->nextSibling);
            }
            if (node->firstChild != NULL) {
                st.push(node->firstChild);
            } else {
                if (node->value == down->value) {
                    node->firstChild = down->firstChild;
                    delete down;
                    return up;
                }
            }
        }
        return NULL;
    }

    ostream &operator<<(ostream &os, Instruction &inst) {
        inst.print(os);
        return os;
    }


    AssignInst::~AssignInst() {}

    void AssignInst::print(ostream &os) {
        os << var << " <- " << s;
    }

    vector <string> AssignInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        ss << "(" << var << " <- " << s << ")";
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *AssignInst::getInstTree() {
        TreeNode *varNode = new TreeNode(var), *sNode = new TreeNode(s);
        varNode->firstChild = sNode;
        return varNode;
    }


    AssignOpInst::~AssignOpInst() {}

    void AssignOpInst::print(ostream &os) {
        os << var << " <- " << lt << " " << opToString(op) << " " << rt;
    }

    vector <string> AssignOpInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        ss << "(" << var << " <- " << lt << ")";
        l2.push_back(ss.str());
        ss.str(string());
        ss << "(" << var << " " << opToString(op) << "= " << rt << ")";
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *AssignOpInst::getInstTree() {
        TreeNode *varNode = new TreeNode(var), *ltNode = new TreeNode(lt), *rtNode = new TreeNode(rt);
        TreeNode *opNode = new TreeNode(opToString(op));
        varNode->firstChild = opNode;
        opNode->firstChild = ltNode;
        ltNode->nextSibling = rtNode;
        return varNode;
    }


    AssignCmpInst::~AssignCmpInst() {}

    void AssignCmpInst::print(ostream &os) {
        os << var << " <- " << lt << " " << cmpToString(cmp) << " " << rt;
    }

    vector <string> AssignCmpInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        ss << "(" << var << " <- ";
        if (cmp == LT || cmp == LE || cmp == EQ) {
            ss << lt << " " << cmpToString(cmp) << " " << rt;
        } else if (cmp == GT) {
            ss << rt << " < " << lt;
        } else {
            ss << rt << " <= " << lt;
        }
        ss << ")";
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *AssignCmpInst::getInstTree() {
        TreeNode *varNode = new TreeNode(var), *ltNode = new TreeNode(lt), *rtNode = new TreeNode(rt);
        TreeNode *cmpNode = new TreeNode(cmpToString(cmp));
        varNode->firstChild = cmpNode;
        cmpNode->firstChild = ltNode;
        ltNode->nextSibling = rtNode;
        return varNode;
    }


    LoadInst::~LoadInst() {}

    void LoadInst::print(ostream &os) {
        os << lvar << " <- load " << rvar;
    }

    vector <string> LoadInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        ss << "(" << lvar << " <- (mem " << rvar << " 0))";
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *LoadInst::getInstTree() {
        TreeNode *lvarNode = new TreeNode(lvar), *rvarNode = new TreeNode(rvar), *loadNode = new TreeNode("load");
        lvarNode->firstChild = loadNode;
        loadNode->firstChild = rvarNode;
        return lvarNode;
    }


    StoreInst::~StoreInst() {}

    void StoreInst::print(ostream &os) {
        os << "store " << var << " <- " << s;
    }

    vector <string> StoreInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        ss << "((mem " << var << " 0) <- " << s << ")";
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *StoreInst::getInstTree() {
        TreeNode *storeNode = new TreeNode("store"), *varNode = new TreeNode(var), *sNode = new TreeNode(s);
        storeNode->firstChild = varNode;
        varNode->nextSibling = sNode;
        return storeNode;
    }


    BranchInst::~BranchInst() {}

    void BranchInst::print(ostream &os) {
        if (var.length() == 0) {
            os << "br " << llabel;
        } else {
            os << "br " << var << " " << llabel << " " << rlabel;
        }
    }

    vector <string> BranchInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        if (var.length() == 0) {
            ss << "(goto " << llabel << ")";
        } else {
            ss << "(cjump " << var << " = 0 " << rlabel << " " << llabel << ")";
        }
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *BranchInst::getInstTree() {
        TreeNode *brNode = new TreeNode("br"), *llNode = new TreeNode(llabel);
        if (var.length() == 0) {
            brNode->firstChild = llNode;
        } else {
            TreeNode *varNode = new TreeNode(var), *rlNode = new TreeNode(rlabel);
            brNode->firstChild = varNode;
            varNode->nextSibling = llNode;
            llNode->nextSibling = rlNode;
        }
        return brNode;
    }


    LabelInst::~LabelInst() {}

    void LabelInst::print(ostream &os) {
        os << label;
    }

    vector <string> LabelInst::toL2(string &suffix) {
        vector <string> l2;
        l2.push_back(label);
        return l2;
    }

    TreeNode *LabelInst::getInstTree() {
        return new TreeNode(label);
    }


    ReturnInst::~ReturnInst() {}

    void ReturnInst::print(ostream &os) {
        if (var.length() == 0) {
            os << "return";
        } else {
            os << "return " << var;
        }
    }

    vector <string> ReturnInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        if (var.length() != 0) {
            ss << "(rax <- " << var << ")";
            l2.push_back(ss.str());
            ss.str(string());
        }
        ss << "(return)";
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *ReturnInst::getInstTree() {
        TreeNode *retNode = new TreeNode("return");
        if (var.length() != 0) {
            retNode->firstChild = new TreeNode(var);
        }
        return retNode;
    }


    CallInst::~CallInst() {}

    void CallInst::print(ostream &os) {
        os << "call " << callee << " (";
        for (int i = 0; i < args.size(); i++) {
            if (i == 0) {
                os << args[i];
            } else {
                os << ", " << args[i];
            }
        }
        os << ")";
    }

    vector <string> CallInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        for (int i = 0; i < args.size() && i < argReg.size(); i++) {
            ss << "(" << argReg[i] << " <- " << args[i] << ")";
            l2.push_back(ss.str());
            ss.str(string());
        }
        bool isRunTime = callee == "print" || callee == "allocate" || callee == "array-error";
        if (!isRunTime) {
            ss << "((mem rsp -8) <- " << callee[0] == ':' ? callee : ":" + callee << "_ret" << suffix << ")";
            l2.push_back(ss.str());
            ss.str(string());
            for (int i = 6, sp = -16; i < args.size(); i++, sp -= 8) {
                ss << "(mem rsp " << sp << ") <- " << args[i] << ")";
                l2.push_back(ss.str());
                ss.str(string());
            }
        }
        ss << "(call " << callee << " " << args.size() << ")";
        l2.push_back(ss.str());
        ss.str(string());
        if (!isRunTime) {
            ss << callee[0] == ':' ? callee : ":" + callee << "_ret" << suffix;
            l2.push_back(ss.str());
            ss.str(string());
        }
        return l2;
    }

    TreeNode *CallInst::getInstTree() {
        TreeNode *callNode = new TreeNode("call"), *calleeNode = new TreeNode(callee);
        callNode->firstChild = calleeNode;
        TreeNode *prevNode = calleeNode;
        for (auto const &arg : args) {
            prevNode->nextSibling = new TreeNode(arg);
            prevNode = prevNode->nextSibling;
        }
        return callNode;
    }


    AssignCallInst::~AssignCallInst() {};

    void AssignCallInst::print(ostream &os) {
        os << var << " <- call " << callee << " (";
        for (int i = 0; i < args.size(); i++) {
            if (i == 0) {
                os << args[i];
            } else {
                os << ", " << args[i];
            }
        }
        os << ")";
    }

    vector <string> AssignCallInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        for (int i = 0; i < args.size() && i < argReg.size(); i++) {
            ss << "(" << argReg[i] << " <- " << args[i] << ")";
            l2.push_back(ss.str());
            ss.str(string());
        }
        bool isRunTime = callee == "print" || callee == "allocate" || callee == "array-error";
        if (!isRunTime) {
            ss << "((mem rsp -8) <- " << callee << "_ret" << suffix << ")";
            l2.push_back(ss.str());
            ss.str(string());
            for (int i = 6, sp = -16; i < args.size(); i++, sp -= 8) {
                ss << "(mem rsp " << sp << ") <- " << args[i] << ")";
                l2.push_back(ss.str());
                ss.str(string());
            }
        }
        ss << "(call " << callee << " " << args.size() << ")";
        l2.push_back(ss.str());
        ss.str(string());
        if (!isRunTime) {
            ss << callee << "_ret" << suffix;
            l2.push_back(ss.str());
            ss.str(string());
        }
        ss << "(" << var << " <- rax)";
        l2.push_back(ss.str());
        return l2;
    }

    TreeNode *AssignCallInst::getInstTree() {
        TreeNode *varNode = new TreeNode(var), *callNode = new TreeNode("call"), *calleeNode = new TreeNode(callee);
        varNode->firstChild = callNode;
        callNode->firstChild = calleeNode;
        TreeNode *prevNode = calleeNode;
        for (auto const &arg : args) {
            prevNode->nextSibling = new TreeNode(arg);
            prevNode = prevNode->nextSibling;
        }
        return varNode;
    }


    Function::~Function() {
        for (auto const &i : instructions) {
            delete i;
        }
    }

    vector<TreeNode *> Function::getInstTrees() {
        vector<TreeNode *> trees;
        TreeNode *mergedTree = NULL;
        TreeNode *prevTree = instructions[0]->getInstTree();
        for (int i = 1; i < instructions.size(); i++) {
            TreeNode *currTree = instructions[i]->getInstTree();
            mergedTree = mergeTwoTree(currTree, prevTree);
            if (mergedTree == NULL) {
                trees.push_back(prevTree);
                prevTree = currTree;
            } else {
                prevTree = mergedTree;
            }
        }
        trees.push_back(mergedTree == NULL ? prevTree : mergedTree);
        return trees;
    }


    Program::~Program() {
        for (auto const &f : functions) {
            delete f;
        }
    }

    vector <string> Program::toL2() {
        vector <string> l2;
        stringstream ss;
        int index = 0;
        l2.push_back("(" + name);
        for (auto const &f : functions) {
            l2.push_back("    (" + f->name);
            ss << f->arguments.size() << " 0";
            l2.push_back("        " + ss.str());
            ss.str(string());
            for (int i = 0; i < f->arguments.size() && i < argReg.size(); i++) {
                ss << "        (" << f->arguments[i] << " <- " << argReg[i] << ")";
                l2.push_back(ss.str());
                ss.str(string());
            }
            for (int i = 6, sp = 8 * (f->arguments.size() - 7); i < f->arguments.size(); i++, sp -= 8) {
                ss << "(" << f->arguments[i] << " <- (stack-arg " << sp << "))";
                l2.push_back(ss.str());
                ss.str(string());
            }
            for (auto const &inst : f->instructions) {
                string idx = to_string(index++);
                vector <string> instL2 = inst->toL2(idx);
                for (auto const &s : instL2) {
                    l2.push_back("        " + s);
                }
            }
            l2.push_back("    )");
        }
        l2.push_back(")");
        return l2;
    }
}