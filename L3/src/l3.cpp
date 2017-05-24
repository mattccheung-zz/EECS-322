#include <vector>
#include <string>
#include <iostream>
#include <stack>
#include <set>
#include <map>

#include "l3.h"

using namespace std;

namespace L3 {
    int n = 0;

    inline string tsfLabel(const string &label, const map<string, string> &labelMap) {
        return labelMap.count(label) > 0 ? labelMap.at(label) : label;
    }

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

    vector <string> AssignInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        l2.push_back("(" + var + " <- " + tsfLabel(s, labelMap) + ")");
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

    vector <string> AssignOpInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        string tmp = "_tmp_aop_inst_";
        if (var == lt) {
            if (op == ADDQ && rt == "1") {
                l2.push_back("(" + var + "++)");
            } else if (op == SUBQ && rt == "1") {
                l2.push_back("(" + var + "--)");
            } else {
                l2.push_back("(" + var + " " + opToString(op) + "= " + rt + ")");
            }
        } else if (var == rt && (op == ADDQ || op == IMULQ)) {
            if (op == ADDQ || op == IMULQ) {
                if (op == ADDQ && lt == "1") {
                    l2.push_back("(" + var + "++)");
                } else {
                    l2.push_back("(" + var + " " + opToString(op) + "= " + lt + ")");
                }
            } else {
                l2.push_back("(" + tmp + " <- " + lt + ")");
                l2.push_back("(" + tmp + " " + opToString(op) + "= " + rt + ")");
                l2.push_back("(" + var + " <- " + tmp + ")");
            }
        } else {
            l2.push_back("(" + var + " <- " + lt + ")");
            l2.push_back("(" + var + " " + opToString(op) + "= " + rt + ")");
        }
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

    vector <string> AssignCmpInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        if (cmp == LT || cmp == LE || cmp == EQ) {
            l2.push_back("(" + var + " <- " + lt + " " + cmpToString(cmp) + " " + rt + ")");
        } else if (cmp == GT) {
            l2.push_back("(" + var + " <- " + rt + " < " + lt + ")");
        } else {
            l2.push_back("(" + var + " <- " + rt + " <= " + lt + ")");
        }
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

    vector <string> LoadInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        l2.push_back("(" + lvar + " <- (mem " + rvar + " 0))");
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

    vector <string> StoreInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        l2.push_back("((mem " + var + " 0) <- " + tsfLabel(s, labelMap) + ")");
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

    vector <string> BranchInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        if (var.length() == 0) {
            l2.push_back("(goto " + tsfLabel(llabel, labelMap) + ")");
        } else {
            l2.push_back("(cjump " + var + " = 0 " + tsfLabel(rlabel, labelMap) + " " + tsfLabel(llabel, labelMap) + ")");
        }
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

    vector <string> LabelInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        l2.push_back(tsfLabel(label, labelMap));
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

    vector <string> ReturnInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        if (var.length() != 0) {
            l2.push_back("(rax <- " + var + ")");
        }
        l2.push_back("(return)");
        return l2;
    }

    TreeNode *ReturnInst::getInstTree() {
        TreeNode *retNode = new TreeNode("return");
        if (var.length() != 0) {
            retNode->firstChild = new TreeNode(var);
        }
        return retNode;
    }

    AssignCallInst::~AssignCallInst() {};

    void AssignCallInst::print(ostream &os) {
        if (!var.empty()) {
            os << var << " <- ";
        }
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

    vector <string> AssignCallInst::toL2(const map<string, string> &labelMap) {
        vector <string> l2;
        string calleeRetLabel = (callee[0] == ':' ? callee : ":" + callee) + "_ret" + to_string(++n);
        for (int i = 0; i < args.size() && i < argReg.size(); i++) {
            l2.push_back("(" + argReg[i] + " <- " + args[i] + ")");
        }
        bool isRunTime = callee == "print" || callee == "allocate" || callee == "array-error";
        if (!isRunTime) {
            l2.push_back("((mem rsp -8) <- " + calleeRetLabel + ")");
            for (int i = 6, sp = -16; i < args.size(); i++, sp -= 8) {
                l2.push_back("(mem rsp " + to_string(sp) + ") <- " + args[i] + ")");
            }
        }
        l2.push_back("(call " + callee + " " + to_string(args.size()) + ")");
        if (!isRunTime) {
            l2.push_back(calleeRetLabel);
        }
        if (!var.empty()) {
            l2.push_back("(" + var + " <- rax)");
        }
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
        return var.empty() ? callNode : varNode;
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
        set<string> programLabelSet;
        l2.push_back("(" + name);
        for (auto const &f : functions) {
            set<string> functionLabelSet;
            map<string, string> labelMap;
            l2.push_back("    (" + f->name);
            l2.push_back("        " + to_string(f->arguments.size()) + " 0");
            for (int i = 0; i < f->arguments.size() && i < argReg.size(); i++) {
                l2.push_back("        (" + f->arguments[i] + " <- " + argReg[i] + ")");
            }
            for (int i = 6, sp = 8 * (f->arguments.size() - 7); i < f->arguments.size(); i++, sp -= 8) {
                l2.push_back("        (" + f->arguments[i] + " <- (stack-arg " + to_string(sp) + "))");
            }
            for (auto const &inst : f->instructions) {
                if (LabelInst *lbInst = dynamic_cast<LabelInst *>(inst)) {
                    if (programLabelSet.count(lbInst->label) > 0) {
                        string newLabel = lbInst->label + "_" + f->name.substr(1) + "_";
                        labelMap[lbInst->label] = newLabel;
                        functionLabelSet.insert(newLabel);
                    } else {
                        functionLabelSet.insert(lbInst->label);
                    }
                }
            }
            for (auto const &inst : f->instructions) {
                for (auto const &s : inst->toL2(labelMap)) {
                    l2.push_back("        " + s);
                }
            }
            l2.push_back("    )");
            programLabelSet.insert(functionLabelSet.begin(), functionLabelSet.end());
        }
        l2.push_back(")");
        return l2;
    }
}