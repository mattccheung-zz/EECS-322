#include <sstream>
#include <cstdlib>

#include "ir.h"

using namespace std;


namespace IR {

    string strip(const string &s) {
        return s[0] == '%' ? s.substr(1) : s;
    }

    string genRandStr(int len) {
        static const char alphahum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        static bool flag = false;
        if (!flag) {
            flag = true;
            srand(time(NULL));
        }
        stringstream ss;
        for (int i = 0; i < len; i++) {
            ss << alphahum[rand() % (sizeof(alphahum) - 1)];
        }
        return ss.str();
    }

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
        if (lb == ":entry") {
            os << lb << genRandStr(2);
        } else {
            os << lb;
        }
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
            os << "br " << strip(t) << " " << lb << " " << rb;
        }
    }

    ReturnInst::ReturnInst(const string &s) {
        t = s;
    }

    void ReturnInst::print(ostream &os) {
        if (t.empty()) {
            os << "return";
        } else {
            os << "return " << strip(t);
        }
    }

    TypeInst::TypeInst(const string &t, const string &v) {
        type = Type(t);
        var = v;
    }

    void TypeInst::print(ostream &os) {
        //os << ";" << type.toString() << " " << var;
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
        if (!varIndex.empty()) {
//            os << "assign instruction var index not empty";
        } else if (!sIndex.empty()) {
//            os << "assign instruction s index not empty";
        } else {
            os << strip(var) << " <- " << strip(s);
        }
    }

    AssignOpInst::AssignOpInst(const string &var, const string &lt, const string &rt, const string &op) {
        this->var = var;
        this->lt = lt;
        this->rt = rt;
        this->op = parseOp(op);
    }

    void AssignOpInst::print(ostream &os) {
        os << strip(var) << " <- " << strip(lt) << " " << opToString(op) << " " << strip(rt);
    }

    AssignLengthInst::AssignLengthInst(const string &lv, const string &rv, const string &t) {
        this->lv = lv;
        this->rv = rv;
        this->t = t;
    }

    void AssignLengthInst::print(ostream &os) {
        string offset = "addr_" + genRandStr(2);
        os << offset << " <- " << strip(t) << " * 8" << endl
           << "    " << offset << " <- " << offset << " + 16" << endl
           << "    " << offset << " <- " << offset << " + " << strip(rv) << endl
           << "    " << strip(lv) << " <- load " << offset;
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
            os << strip(var) << " <- ";
        }
        os << "call " << strip(callee) << "(";
        for (int i = 0; i < args.size(); i++) {
            if (i == 0) {
                os << strip(args[i]);
            } else {
                os << ", " << strip(args[i]);
            }
        }
        os << ")";
    }

    NewArrayInst::NewArrayInst(const string &v, const vector <string> as) {
        var = v;
        args = as;
    }

    void NewArrayInst::print(ostream &os) {
        string count = "cnt_" + genRandStr(2), d = "dim_" + genRandStr(2), addr = "addr_" + genRandStr(2);
        os << count << " <- 1" << endl;
        for (auto const &arg : args) {
            os << "    " << d << " <- " << strip(arg) << " >> 1" << endl
               << "    " << count << " <- " << count << " * " << d << endl;
        }
        os << "    " << count << " <- " << count << " + " << args.size() << endl
           << "    " << count << " <- " << count << " << 1" << endl
           << "    " << count << " <- " << count << " + 3" << endl
           << "    " << strip(var) << " <- call allocate(" << count << ", 1)" << endl
           << "    " << addr << " <- " << strip(var) << " + 8" << endl
           << "    " << "store " << addr << " <- " << args.size() * 2 + 1 << endl;
        for (auto const &arg : args) {
            os << "    " << addr << " <- " << addr << " + 8" << endl
               << "    " << "store " << addr << " <- " << arg << endl;
        }
    }

    NewTupleInst::NewTupleInst(const string &v, const string &t) {
        var = v;
        this->t = t;
    }

    void NewTupleInst::print(ostream &os) {
        os << strip(var) << " <- call allocate(" << strip(t) << ", 1)" << endl;
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

    string Function::toL3() {
        stringstream ss;
        ss << "define " << name << " (";
        for (int i = 0; i < arguments.size(); i++) {
            if (i == 0) {
                ss << arguments[i]->var.substr(1);
            } else {
                ss << ", " << arguments[i]->var.substr(1);
            }
        }
        ss << ") {" << endl;
        for (auto const &bb : basicBlocks) {
            for (auto const &inst : bb->instructions) {
                ss << "    " << *inst << endl;
            }
        }
        ss << "}" << endl;
        return ss.str();
    }

    Program::~Program() {
        for (auto const &f : functions) {
            delete f;
        }
    }

    string Program::toL3() {
        stringstream ss;
        for (auto const &f : functions) {
            ss << f->toL3() << endl;
        }
        return ss.str();
    }
}