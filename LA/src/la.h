#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <map>


using namespace std;


namespace LA {
    enum OP {
        NOP, ADDQ, SUBQ, IMULQ, ANDQ, SALQ, SARQ, LT, LE, EQ, GE, GT
    };

    inline OP parseOp(const string &s) {
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

    inline string opToString(const OP &op) {
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
        NO_TYPE, VOID, INT64, TUPLE, CODE
    };

    inline TYPE parseType(const string &s) {
        return s == "void" ? VOID :
               (s == "tuple" ? TUPLE :
                (s == "code" ? CODE :
                 (s.substr(0, 5) == "int64" ? INT64 : NO_TYPE)));
    }

    inline string typeToString(const TYPE &type) {
        return type == VOID ? "void" :
               (type == INT64 ? "int64" :
                (type == TUPLE ? "tuple" :
                 (type == CODE ? "code" : "NO_TYPE")));
    }

    struct Type {
        TYPE type;
        int dim;

        Type() {};

        Type(const string &s);

        string toString();
    };

    struct Instruction {
        virtual ~Instruction() {};

        virtual void print(ostream &os) = 0;

        virtual vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap) = 0;
    };

    ostream &operator<<(ostream &os, Instruction &inst);

    struct LabelInst : public Instruction {
        string lb;

        LabelInst(const string &s);

        ~LabelInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct BranchInst : public Instruction {
        string t, lb, rb;

        BranchInst(const string &s);

        BranchInst(const string &t, const string &lb, const string &rb);

        ~BranchInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct ReturnInst : public Instruction {
        string t;

        ReturnInst() {};

        ReturnInst(const string &s);

        ~ReturnInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct TypeInst : public Instruction {
        Type type;
        string var;

        TypeInst(const string &t, const string &s);

        ~TypeInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct AssignInst : public Instruction {
        string var, s;
        vector <string> varIndex, sIndex;

        AssignInst(const string &v, const string &s);

        AssignInst(const string &lv, const string &rv, const vector <string> &ts);

        AssignInst(const string &v, const vector <string> &ts, const string &s);

        ~AssignInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct AssignOpInst : public Instruction {
        string var, lt, rt;
        OP op;

        AssignOpInst(const string &var, const string &lt, const string &rt, const string &op);

        ~AssignOpInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct AssignLengthInst : public Instruction {
        string lv, rv, t;

        AssignLengthInst(const string &lv, const string &rv, const string &t);

        ~AssignLengthInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct AssignCallInst : public Instruction {
        string var, callee;
        vector <string> args;

        AssignCallInst(const string &c, const vector <string> &as);

        AssignCallInst(const string &v, const string &c, const vector <string> &as);

        ~AssignCallInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct NewArrayInst : public Instruction {
        string var;
        vector <string> args;

        NewArrayInst(const string &v, const vector <string> as);

        ~NewArrayInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct NewTupleInst : public Instruction {
        string var, t;

        NewTupleInst(const string &v, const string &t);

        ~NewTupleInst() {};

        void print(ostream &os);

        vector <string> toIR(set <string> &nVarSet, map<string, Type> &varMap);
    };

    struct Function {
        string name;
        Type returnType;
        vector<TypeInst *> arguments;
        vector<Instruction *> instructions;

        ~Function();

        string toIR();
    };

    struct Program {
        string name;
        vector<Function *> functions;

        ~Program();

        string toIR();
    };
}

