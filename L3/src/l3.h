#pragma once

#include <vector>
#include <string>

using namespace std;


namespace L3 {

    enum OP {
        ADDQ, SUBQ, IMULQ, ANDQ, SALQ, SARQ
    };

    enum CMP {
        LT, LE, EQ, GE, GT
    };

    struct Instruction {
    };

    struct AssignInst : public Instruction {
        string var, s;
    };

    struct AssignOpInst : public Instruction {
        string var, lt, rt;
        OP op;
    };

    struct AssignCmpInst : public Instruction {
        string var, lt, rt;
        CMP cmp;
    };

    struct LoadInst : public Instruction {
        string lvar, rvar;
    };

    struct StoreInst : public Instruction {
        string var, s;
    };

    struct BranchInst : public Instruction {
        string var, llabel, rlabel;
    };

    struct LabelInst : public Instruction {
        string label;
    };

    struct ReturnInst : public Instruction {
        string var;
    };

    struct CallInst : public Instruction {
        string callee;
        vector <string> args;
    };

    struct AssignCallInst : public Instruction {
        string var, callee;
        vector <string> args;
    };

    struct Function {
        string name;
        vector <string> arguments;
        vector <Instruction> instructions;
    };

    struct Program {
        string name;
        vector <Function> functions;
    };

}
