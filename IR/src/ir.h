#pragma once

#include <vector>
#include <string>


using namespace std;


namespace IR {
    enum OP {
        NOP, ADDQ, SUBQ, IMULQ, ANDQ, SALQ, SARQ, LT, LE, EQ, GE, GT
    };

    inline OP parseOp(string &s) {
        return s == "+" ? ADDQ :
               (s == "-" ? SUBQ :
                (s == "*" ? IMULQ :
                 (s == "&" ? ANDQ :
                  (s == "<<" ? SALQ :
                   (s == ">>" ? SARQ :
                    (s == "<" ? LT :
                     (s == "<=" ? LE :
                      (s == "=" ? EQ :
                       (s == ">=" ? GE :
                        (s == ">" ? GT : NOP))))))))));
    }

    inline string opToString(OP op) {
        return op == ADDQ ? "+" :
               (op == SUBQ ? "-" :
                (op == IMULQ ? "*" :
                 (op == ANDQ ? "&" :
                  (op == SALQ ? "<<" :
                   (op == SARQ ? ">>" :
                    (op == LT ? "<" :
                     (op == LE ? "<=" :
                      (op == EQ ? "=" :
                       (op == GE ? ">=" :
                        (op == GT ? ">" : "NOP"))))))))));
    }


    enum TYPE {
        VOID, INT64, TUPLE, CODE
    };

    struct Type {
        TYPE type;
        int dim;
    };

    struct Instruction {
        virtual ~Instruction() {};
    };

    struct LabelInst : public Instruction {
        string lb;

        ~LabelInst() {};
    };

    struct BranchInst : public Instruction {
        string t, lb, rb;

        ~BranchInst() {};
    };

    struct ReturnInst : public Instruction {
        string t;

        ~ReturnInst() {};
    };

    struct TypeInst : public Instruction {
        Type type;
        string var;

        ~TypeInst() {};
    };

    struct AssignInst : public Instruction {
        string var, s;
        vector<string> varIndex, sIndex;

        ~AssignInst() {};
    };

    struct AssignOpInst : public Instruction {
        string var, lt, rt;
        OP op;

        ~AssignOpInst() {};
    };

    struct AssignLengthInst : public Instruction {
        string lv, rv, t;

        ~AssignLengthInst() {};
    };

    struct AssignCallInst : public Instruction {
        string var, callee;
        vector<string> args;

        ~AssignCallInst() {};
    };

    struct AssignArrayInst : public Instruction {
        string var;
        vector<string> args;

        ~AssignArrayInst() {};
    };

    struct AssignTupleInst : public Instruction {
        string var, t;

        ~AssignTupleInst() {};
    };

    struct BasicBlock {
        vector<Instruction *> instructions;

        ~BasicBlock();
    };

    struct Function {
        string name;
        Type returnType;
        vector <TypeInst *> arguments;
        vector<BasicBlock *> basicBlocks;

        ~Function();
    };

    struct Program {
        string name;
        vector<Function *> functions;

        ~Program();
    };
}

