#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include "l3.h"

using namespace std;

namespace L3 {

    vector <string> argReg = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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


    AssignCmpInst::~AssignCmpInst() {}

    void AssignCmpInst::print(ostream &os) {
        os << var << " <- " << lt << " " << cmpToString(cmp) << " " << rt;
    }

    vector <string> AssignCmpInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        ss << "(" << var << " <- " << lt << " " << cmpToString(cmp) << " " << rt << ")";
        l2.push_back(ss.str());
        return l2;
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


    LabelInst::~LabelInst() {}

    void LabelInst::print(ostream &os) {
        os << label;
    }

    vector <string> LabelInst::toL2(string &suffix) {
        vector <string> l2;
        stringstream ss;
        ss << label;
        l2.push_back(ss.str());
        return l2;
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
        return l2;
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


    Function::~Function() {
        for (auto const &i : instructions) {
            delete i;
        }
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