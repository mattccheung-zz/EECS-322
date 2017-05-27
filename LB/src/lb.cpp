#include <sstream>
#include <cstdlib>
#include <set>
#include <cassert>
#include <map>

#include "lb.h"

using namespace std;


namespace LB {

    inline bool isRunTime(const string &s) {
        return s == "print" || s == "array-error";
    }

    inline bool isNum(const string &s) {
        return s[0] == '+' || s[0] == '-' || (s[0] >= '0' && s[0] <= '9');
    }

    inline bool isVar(const string &s) {
        return s[0] == '%';
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

    TypeInst::TypeInst(const string &t, const vector<string> &vars) {
        type = Type(t);
        this->vars = vars;
    }

    void TypeInst::print(ostream &os) {
        os << type.toString() << " ";
        bool flag = false;
        for (auto const &var : vars) {
            if (flag) {
                os << ", ";
            } else {
                flag = true;
            }
            os << var;
        }
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
            os << var;
            for (auto const &idx : varIndex) {
                os << "[" << idx << "]";
            }
            os << " <- " << s;
        } else if (!sIndex.empty()) {
            os << var << " <- " << s;
            for (auto const &idx : sIndex) {
                os << "[" << idx << "]";
            }
        } else {
            os << var << " <- " << s;
        }
    }

    AssignCondInst::AssignCondInst(const string &var, const string &lt, const string &op, const string &rt) {
        this->var = var;
        this->lt = lt;
        this->rt = rt;
        this->op = parseOp(op);
    }

    void AssignCondInst::print(ostream &os) {
        os << var << " <- " << lt << " " << opToString(op) << " " << rt;
    }

    LabelInst::LabelInst(const string &s) {
        lb = s;
    }

    void LabelInst::print(ostream &os) {
        os << lb;
    }

    IfInst::IfInst(const string &lt, const string &op, const string &rt, const string &lb, const string &rb) {
        this->lt = lt;
        this->rt = rt;
        this->lb = lb;
        this->rb = rb;
        this->op = parseOp(op);
    }

    void IfInst::print(ostream &os) {
        os << "if (" << lt << " " << opToString(op) << " " << rt << ") " << lb << " " << rb;
    }

    ReturnInst::ReturnInst(const string &s) {
        t = s;
    }

    void ReturnInst::print(ostream &os) {
        os << "return";
        if (!t.empty()) {
            os << " " << t;
        }
    }

    WhileInst::WhileInst(const string &lt, const string &op, const string &rt, const string &lb, const string &rb) {
        this->lt = lt;
        this->rt = rt;
        this->lb = lb;
        this->rb = rb;
        this->op = parseOp(op);
    }

    void WhileInst::print(ostream &os) {
        os << "while (" << lt << " " << opToString(op) << " " << rt << ") " << lb << " " << rb;
    }

    void ContinueInst::print(ostream &os) {
        os << "continue";
    }

    void BreakInst::print(ostream &os) {
        os << "break";
    }

    AssignLengthInst::AssignLengthInst(const string &lv, const string &rv, const string &t) {
        this->lv = lv;
        this->rv = rv;
        this->t = t;
    }

    void AssignLengthInst::print(ostream &os) {
        os << lv << " <- length " << rv << " " << t;
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
        os << "call " << callee << "(";
        for (int i = 0; i < args.size(); i++) {
            if (i > 0) {
                os << ", ";
            }
            os << args[i];
        }
        os << ")";
    }

    NewArrayInst::NewArrayInst(const string &v, const vector <string> as) {
        var = v;
        args = as;
    }

    void NewArrayInst::print(ostream &os) {
        os << var << " <- new Array(";
        for (int i = 0; i < args.size(); i++) {
            if (i > 0) {
                os << ", ";
            }
            os << args[i];
        }
        os << ")";
    }

    NewTupleInst::NewTupleInst(const string &v, const string &t) {
        var = v;
        this->t = t;
    }

    void NewTupleInst::print(ostream &os) {
        os << var << " <- new Tuple(" << t << ")";
    }

    Scope::~Scope() {
        for (auto const &inst : instructions) {
            delete inst;
        }
    }

    void Scope::print(ostream &os) {
        throw runtime_error("scope print is not implemented");
    }

    Function::~Function() {
        delete scope;
    }

    Program::~Program() {
        for (auto const &f : functions) {
            delete f;
        }
    }
}