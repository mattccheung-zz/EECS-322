#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include <utility>


using namespace std;


namespace LB {
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
    };

    ostream &operator<<(ostream &os, Instruction &inst);

    struct TypeInst : Instruction {
        Type type;
        vector<string> vars;

        TypeInst(const string &t, const vector<string> &vars);

        ~TypeInst() {};

        void print(ostream &os);
    };

    struct AssignInst : Instruction {
        string var, s;
        vector <string> varIndex, sIndex;

        AssignInst(const string &v, const string &s);

        AssignInst(const string &lv, const string &rv, const vector <string> &ts);

        AssignInst(const string &v, const vector <string> &ts, const string &s);

        ~AssignInst() {};

        void print(ostream &os);
    };

    struct AssignCondInst : Instruction {
        string var, lt, rt;
        OP op;

        AssignCondInst(const string &var, const string &lt, const string &op, const string &rt);

        ~AssignCondInst() {};

        void print(ostream &os);
    };

    struct LabelInst : Instruction {
        string lb;

        LabelInst(const string &s);

        ~LabelInst() {};

        void print(ostream &os);
    };

    struct IfInst : Instruction {
        string lt, rt, lb, rb;
        OP op;

        IfInst(const string &lt, const string &op, const string &rt, const string &lb, const string &rb);

        ~IfInst() {};

        void print(ostream &os);
    };

    struct ReturnInst : Instruction {
        string t;

        ReturnInst(const string &s);

        ~ReturnInst() {};

        void print(ostream &os);
    };

    struct WhileInst : Instruction {
        string lt, rt, lb, rb;
        OP op;

        WhileInst(const string &lt, const string &op, const string &rt, const string &lb, const string &rb);

        ~WhileInst() {};

        void print(ostream &os);
    };

    struct ContinueInst : Instruction {
        ~ContinueInst() {};

        void print(ostream &os);
    };

    struct BreakInst : Instruction {
        ~BreakInst() {};

        void print(ostream &os);
    };

    struct AssignLengthInst : Instruction {
        string lv, rv, t;

        AssignLengthInst(const string &lv, const string &rv, const string &t);

        ~AssignLengthInst() {};

        void print(ostream &os);
    };

    struct AssignCallInst : Instruction {
        string var, callee;
        vector <string> args;

        AssignCallInst(const string &v, const string &c, const vector <string> &as);

        ~AssignCallInst() {};

        void print(ostream &os);
    };

    struct NewArrayInst : Instruction {
        string var;
        vector <string> args;

        NewArrayInst(const string &v, const vector <string> as);

        ~NewArrayInst() {};

        void print(ostream &os);
    };

    struct NewTupleInst : Instruction {
        string var, t;

        NewTupleInst(const string &v, const string &t);

        ~NewTupleInst() {};

        void print(ostream &os);
    };

    struct Scope;
    struct Scope : Instruction {
        vector<Instruction *> instructions;
        Scope *parent;

        ~Scope();

        void print(ostream &os);
    };

    struct Function {
        string name;
        Type returnType;
        vector<pair<Type, string>> arguments;
        Scope *scope;

        ~Function();
    };

    struct Program {
        string name;
        vector<Function *> functions;

        ~Program();
    };
}

