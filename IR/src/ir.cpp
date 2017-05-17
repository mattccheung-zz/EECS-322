#include "ir.h"

using namespace std;


namespace IR {

    Type::Type(const string &s) {
        type = parseType(s);
        if (type == INT64) {
            dim = (s.length() - 5) / 2;
        } else {
            dim = -1;
        }
    }

    string Type::toString() {
        string str = typeToString(type);
        for (int i = 0; i < dim; i++) {
            str += "[]";
        }
        return str;
    }

    ostream &operator<<(ostream &os, Instruction &inst) {
        inst.print(os);
        return os;
    }

    LabelInst::LabelInst(const string &s) {
        lb = s;
    }

    void LabelInst::print(ostream &os) {
        os << lb;
    }

    BranchInst::BranchInst(const string &s) {
        lb = s;
    }

    BranchInst::BranchInst(const string &t, const string &lb, const string &rb) {
        this->t = t;
        this->lb = lb;
        this->rb = rb;
    }

    void BranchInst::print(ostream &os) {
        if (t.empty()) {
            os << "br " << lb;
        } else {
            os << "br " << t << " " << lb << " " << rb;
        }
    }

    ReturnInst::ReturnInst(const string &s) {
        t = s;
    }

    void ReturnInst::print(ostream &os) {
        if (t.empty()) {
            os << "return";
        } else {
            os << "return " << t;
        }
    }

    TypeInst::TypeInst(const string &t, const string &v) {
        type = Type(t);
        var = v;
    }

    void TypeInst::print(ostream &os) {
        os << type.toString() << " " << var;
    }

    AssignInst::AssignInst(const string &v, const string &s) {
        var = v;
        this->s = s;
    }

    AssignInst::AssignInst(const string &v, const vector <string> &ts, const string &s) {
        var = v;
        this->s = s;
        varIndex = ts;
    }

    AssignInst::AssignInst(const string &lv, const string &rv, const vector <string> &ts) {
        var = lv;
        s = rv;
        sIndex = ts;
    }

    void AssignInst::print(ostream &os) {
        os << var << " <- " << s << " not complete";
    }

    AssignOpInst::AssignOpInst(const string &var, const string &lt, const string &rt, const string &op) {
        this->var = var;
        this->lt = lt;
        this->rt = rt;
        this->op = parseOp(op);
    }

    void AssignOpInst::print(ostream &os) {
        os << var << " <- " << lt << " " << opToString(op) << " " << rt;
    }

    AssignLengthInst::AssignLengthInst(const string &lv, const string &rv, const string &t) {
        this->lv = lv;
        this->rv = rv;
        this->t = t;
    }

    void AssignLengthInst::print(ostream &os) {
        os << lv << " <- length " << rv << " " << t;
    }

    AssignCallInst::AssignCallInst(const string &c, const vector <string> &as) {
        callee = c;
        args = as;
    }

    AssignCallInst::AssignCallInst(const string &v, const string &c, const vector <string> &as) {
        var = v;
        callee = c;
        args = as;
    }

    void AssignCallInst::print(ostream &os) {
        if (!var.empty()) {
            os << var << " <- ";
        }
        os << "call " << callee << " args not complete";
    }

    NewArrayInst::NewArrayInst(const string &v, const vector <string> as) {
        var = v;
        args = as;
    }

    void NewArrayInst::print(ostream &os) {
        os << var << "<- new Array(args) not complete";
    }

    NewTupleInst::NewTupleInst(const string &v, const string &t) {
        var = v;
        this->t = t;
    }

    void NewTupleInst::print(ostream &os) {
        os << var << " <- new Tuple(" << t << ")";
    }

    BasicBlock::BasicBlock(vector <Instruction *> insts) {
        instructions = insts;
    }

    BasicBlock::~BasicBlock() {
        for (auto const &inst : instructions) {
            delete inst;
        }
    }

    Function::~Function() {
        for (auto const &arg : arguments) {
            delete arg;
        }
        for (auto const &b : basicBlocks) {
            delete b;
        }
    }

    Program::~Program() {
        for (auto const &f : functions) {
            delete f;
        }
    }
}