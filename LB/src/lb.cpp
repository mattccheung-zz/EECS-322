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

    string getNewVar(Scope* scope, const string &v, map<Scope *, map<string, string>> &varMap) {
        if (!isVar(v)) {
            return v;
        }
        while (scope != NULL && varMap.at(scope).count(v) <= 0) {
            scope = scope->parent;
        }
        if (scope == NULL) {
            throw runtime_error("undefined variable: " + v);
        }
        return varMap.at(scope).at(v);
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

    void removeScope(Scope *scope, const string &id, map<Scope *, map<string, string>> &varMap, vector<Instruction *> &insts) {
        map<string, string> m;
        varMap[scope] = m;
        int index = 0, idx = 0;
        for (auto const &inst : scope->instructions) {
            if (Scope *s = dynamic_cast<Scope *>(inst)) {
                removeScope(s, id + "d" + to_string(index++), varMap, insts);
            } else if (TypeInst *t = dynamic_cast<TypeInst *>(inst)) {
                for (auto const &v : t->vars) {
                    varMap[scope][v] = "var" + id + "i" + to_string(++idx);
                }
                insts.push_back(t->getInstWithNewVar(scope, varMap));
            } else {
                insts.push_back(inst->getInstWithNewVar(scope, varMap));
            }
        }
    }

    vector<Instruction *> removeScope(Scope *scope) {
        vector<Instruction *> insts;
        map<Scope *, map<string, string>> varMap;
        removeScope(scope, "", varMap, insts);
        return insts;
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

    Instruction *TypeInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        vector<string> nVars;
        for (auto const &v : vars) {
            nVars.push_back(getNewVar(scope, v, varMap));
        }
        return new TypeInst(type.toString(), nVars);
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

    Instruction *AssignInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        string nv = getNewVar(scope, var, varMap), ns = getNewVar(scope, s, varMap);
        if (!varIndex.empty()) {
            vector<string> nvIndex;
            for (auto const &v : varIndex) {
                nvIndex.push_back(getNewVar(scope, v, varMap));
            }
            return new AssignInst(nv, nvIndex, ns);
        } else if (!sIndex.empty()) {
            vector<string> nsIndex;
            for (auto const &v : sIndex) {
                nsIndex.push_back(getNewVar(scope, v, varMap));
            }
            return new AssignInst(nv, ns, nsIndex);
        } else {
            return new AssignInst(nv, ns);
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

    Instruction *AssignCondInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new AssignCondInst(getNewVar(scope, var, varMap), getNewVar(scope, lt, varMap), opToString(op),
            getNewVar(scope, rt, varMap));
    }

    LabelInst::LabelInst(const string &s) {
        lb = s;
    }

    void LabelInst::print(ostream &os) {
        os << lb;
    }

    Instruction *LabelInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new LabelInst(lb);
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

    Instruction *IfInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new IfInst(getNewVar(scope, lt, varMap), opToString(op), getNewVar(scope, rt, varMap), lb, rb);
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

    Instruction *ReturnInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new ReturnInst(getNewVar(scope, t, varMap));
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

    Instruction *WhileInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new WhileInst(getNewVar(scope, lt, varMap), opToString(op), getNewVar(scope, rt, varMap), lb, rb);
    }

    void ContinueInst::print(ostream &os) {
        os << "continue";
    }

    Instruction *ContinueInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new ContinueInst();
    }

    void BreakInst::print(ostream &os) {
        os << "break";
    }

    Instruction *BreakInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new BreakInst();
    }

    AssignLengthInst::AssignLengthInst(const string &lv, const string &rv, const string &t) {
        this->lv = lv;
        this->rv = rv;
        this->t = t;
    }

    void AssignLengthInst::print(ostream &os) {
        os << lv << " <- length " << rv << " " << t;
    }

    Instruction *AssignLengthInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new AssignLengthInst(getNewVar(scope, lv, varMap), getNewVar(scope, rv, varMap), getNewVar(scope, t, varMap));
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

    Instruction *AssignCallInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        vector<string> nArgs;
        for (auto const &v : args) {
            nArgs.push_back(getNewVar(scope, v, varMap));
        }
        string nCallee;
        try {
            nCallee = getNewVar(scope, callee, varMap);
        } catch (exception e) {}
        return new AssignCallInst(getNewVar(scope, var, varMap), nCallee.empty() ? callee : nCallee, nArgs);
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

    Instruction *NewArrayInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        vector<string> nArgs;
        for (auto const &v : args) {
            nArgs.push_back(getNewVar(scope, v, varMap));
        }
        return new NewArrayInst(getNewVar(scope, var, varMap), nArgs);
    }

    NewTupleInst::NewTupleInst(const string &v, const string &t) {
        var = v;
        this->t = t;
    }

    void NewTupleInst::print(ostream &os) {
        os << var << " <- new Tuple(" << t << ")";
    }

    Instruction *NewTupleInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new NewTupleInst(getNewVar(scope, var, varMap), getNewVar(scope, t, varMap));
    }

    Scope::~Scope() {
        for (auto const &inst : instructions) {
            delete inst;
        }
    }

    void Scope::print(ostream &os) {
        throw runtime_error("scope print is not implemented");
    }

    Instruction *Scope::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        throw runtime_error("getInstWithNewVar is not implemented in Scope");
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