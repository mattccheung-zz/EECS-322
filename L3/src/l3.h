#pragma once

#include <vector>
#include <string>
#include <iostream>

using namespace std;


namespace L3 {

    enum OP {
        NOP, ADDQ, SUBQ, IMULQ, ANDQ, SALQ, SARQ
    };

    inline OP parseOp(string &s) {
        return s == "+" ? ADDQ :
               (s == "-" ? SUBQ :
                (s == "*" ? IMULQ :
                 (s == "&" ? ANDQ :
                  (s == "<<" ? SALQ :
                   (s == ">>" ? SARQ : NOP)))));
    }

    inline string opToString(OP op) {
        return op == ADDQ ? "+" :
               (op == SUBQ ? "-" :
                (op == IMULQ ? "*" :
                 (op == ANDQ ? "&" :
                  (op == SALQ ? "<<" :
                   (op == SARQ ? ">>" : "NOP")))));
    }

    enum CMP {
        NCMP, LT, LE, EQ, GE, GT
    };

    inline CMP parseCmp(string &s) {
        return s == "<" ? LT :
               (s == "<=" ? LE :
                (s == "=" ? EQ :
                 (s == ">=" ? GE :
                  (s == ">" ? GT : NCMP))));
    }

    inline string cmpToString(CMP cmp) {
        return cmp == LT ? "<" :
               (cmp == LE ? "<=" :
                (cmp == EQ ? "=" :
                 (cmp == GE ? ">=" :
                  (cmp == GT ? ">" : "NCMP"))));
    }

    struct Instruction {
        virtual void print(ostream &os) = 0;

        virtual ~Instruction() {};
    };

    inline ostream &operator<<(ostream &os, Instruction &inst) {
        inst.print(os);
        return os;
    }

    struct AssignInst : public Instruction {
        string var, s;

        ~AssignInst() {};

        void print(ostream &os) {
            os << var << " <- " << s;
        }
    };

    struct AssignOpInst : public Instruction {
        string var, lt, rt;
        OP op;

        ~AssignOpInst() {};

        void print(ostream &os) {
            os << var << " <- " << lt << " " << opToString(op) << " " << rt;
        }
    };

    struct AssignCmpInst : public Instruction {
        string var, lt, rt;
        CMP cmp;

        ~AssignCmpInst() {};

        void print(ostream &os) {
            os << var << " <- " << lt << " " << cmpToString(cmp) << " " << rt;
        }
    };

    struct LoadInst : public Instruction {
        string lvar, rvar;

        ~LoadInst() {};

        void print(ostream &os) {
            os << lvar << " <- load " << rvar;
        }
    };

    struct StoreInst : public Instruction {
        string var, s;

        ~StoreInst() {};

        void print(ostream &os) {
            os << "store " << var << " <- " << s;
        }
    };

    struct BranchInst : public Instruction {
        string var, llabel, rlabel;

        ~BranchInst() {};

        void print(ostream &os) {
            if (var.length() == 0) {
                os << "br " << llabel;
            } else {
                os << "br " << var << " " << llabel << " " << rlabel;
            }
        }
    };

    struct LabelInst : public Instruction {
        string label;

        ~LabelInst() {};

        void print(ostream &os) {
            os << label;
        }
    };

    struct ReturnInst : public Instruction {
        string var;

        ~ReturnInst() {};

        void print(ostream &os) {
            if (var.length() == 0) {
                os << "return";
            } else {
                os << "return " << var;
            }
        }
    };

    struct CallInst : public Instruction {
        string callee;
        vector <string> args;

        ~CallInst() {};

        void print(ostream &os) {
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
    };

    struct AssignCallInst : public Instruction {
        string var, callee;
        vector <string> args;

        ~AssignCallInst() {};

        void print(ostream &os) {
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
    };

    struct Function {
        string name;
        vector <string> arguments;
        vector<Instruction *> instructions;

        ~Function() {
            for (auto const &i : instructions) {
                delete i;
            }
        }
    };

    struct Program {
        string name;
        vector <Function*> functions;

        ~Program() {
            for (auto const &f : functions) {
                delete f;
            }
        }
    };

}
