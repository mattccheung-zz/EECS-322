#include <sstream>
#include <cstdlib>
#include <set>
#include <cassert>
#include <map>
#include <stack>

#include "lb.h"

using namespace std;


namespace LB {

    inline bool isRunTime(const string &s) {
        return s == "print" || s == "array-error";
    }

    inline bool isVar(const string &s) {
        return s[0] == '%';
    }

    string getAnotherFlagVar() {
        static int idx = 0;
        return "%NewFlagVar" + to_string(idx++);
    }

    string getAnotherLabel() {
        static int idx = 0;
        return ":NewLabel" + to_string(idx++);
    }

    string getNewVar(Scope* scope, const string &v, map<Scope *, map<string, string>> &varMap) {
        if (!isVar(v)) {
            return v;
        }
        while (scope != NULL && varMap.at(scope).count(v) <= 0) {
            scope = scope->parent;
        }
        if (varMap.at(scope).count(v) <= 0) {
            throw runtime_error("undefined variable: " + v);
        }
        return varMap.at(scope).at(v);
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
                    varMap[scope][v] = "%Var" + id + "I" + to_string(idx++);
                }
                insts.push_back(t->getInstWithNewVar(scope, varMap));
            } else {
                insts.push_back(inst->getInstWithNewVar(scope, varMap));
            }
        }
    }

    Type::Type(const string &s) {
        type = parseType(s);
        if (type == INT64) {
            dim = (s.length() - 5) / 2;
        } else {
            dim = -1;
        }
    }

    string Type::toString() const {
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

    vector<LA::Instruction *> TypeInst::getLA() {
        vector<LA::Instruction *> la;
        string t = type.toString();
        for (auto const &v : vars) {
            la.push_back(new LA::TypeInst(t, v));
        }
        return la;
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

    vector<LA::Instruction *> AssignInst::getLA() {
        if (!varIndex.empty()) {
            return {new LA::AssignInst(var, varIndex, s)};
        } else if (!sIndex.empty()) {
            return {new LA::AssignInst(var, s, sIndex)};
        } else {
            return {new LA::AssignInst(var, s)};
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

    vector<LA::Instruction *> AssignCondInst::getLA() {
        return {new LA::AssignOpInst(var, lt, rt, opToString(op))};
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

    vector<LA::Instruction *> LabelInst::getLA() {
        return {new LA::LabelInst(lb)};
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

    vector<LA::Instruction *> IfInst::getLA() {
        throw runtime_error("IfInst::getLA() not implemented");
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

    vector<LA::Instruction *> ReturnInst::getLA() {
        return {new LA::ReturnInst(t)};
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

    vector<LA::Instruction *> WhileInst::getLA() {
        throw runtime_error("WhileInst::getLA() not implemented");
    }

    void ContinueInst::print(ostream &os) {
        os << "continue";
    }

    Instruction *ContinueInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new ContinueInst();
    }

    vector<LA::Instruction *> ContinueInst::getLA() {
        throw runtime_error("ContinueInst::getLA() not implemented");
    }

    void BreakInst::print(ostream &os) {
        os << "break";
    }

    Instruction *BreakInst::getInstWithNewVar(Scope *scope, map<Scope *, map<string, string>> &varMap) {
        return new BreakInst();
    }

    vector<LA::Instruction *> BreakInst::getLA() {
        throw runtime_error("BreakInst::getLA() not implemented");
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

    vector<LA::Instruction *> AssignLengthInst::getLA() {
        return {new LA::AssignLengthInst(lv, rv, t)};
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
        return new AssignCallInst(getNewVar(scope, var, varMap), getNewVar(scope, callee, varMap), nArgs);
    }

    vector<LA::Instruction *> AssignCallInst::getLA() {
        return {new LA::AssignCallInst(var, callee, args)};
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

    vector<LA::Instruction *> NewArrayInst::getLA() {
        return {new LA::NewArrayInst(var, args)};
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

    vector<LA::Instruction *> NewTupleInst::getLA() {
        return {new LA::NewTupleInst(var, t)};
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

    vector<LA::Instruction *> Scope::getLA() {
        throw runtime_error("Scope::getLA() not implemented");
    }

    Function::~Function() {
        delete scope;
    }

    LA::Function *Function::getLA() {
        map<Instruction *, string> whileToBegin, whileToEnd, WhileToCond;
        map<string, Instruction *> beginToWhile, EndToWhile;
        LA::Function *f = new LA::Function();
        f->name = name;
        f->returnType = LA::Type(returnType.toString());

        vector<Instruction *> insts;
        map<Scope *, map<string, string>> varMap;
        map<string, string> m;
        varMap[NULL] = m;
        for (auto const &arg : arguments) {
            varMap[NULL][arg.second] = arg.second;
            f->arguments.push_back(new LA::TypeInst(arg.first.toString(), arg.second));
        }
        removeScope(scope, "", varMap, insts);

        for (int i = 0; i < insts.size(); i++) {
            if (WhileInst *whileInst = dynamic_cast<WhileInst *>(insts[i])) {
                string condLabel = getAnotherLabel();
                whileToBegin[whileInst] = whileInst->lb;
                whileToEnd[whileInst] = whileInst->rb;
                WhileToCond[whileInst] = condLabel;
                beginToWhile[whileInst->lb] = whileInst;
                EndToWhile[whileInst->rb] = whileInst;
                insts.insert(insts.begin() + i, new LabelInst(condLabel));
                i++;
            }
        }

        stack<Instruction *> loopStack;
        for (auto const &inst : insts) {
            vector<LA::Instruction *> las;
            if (IfInst *ifInst = dynamic_cast<IfInst *>(inst)) {
                string flag = getAnotherFlagVar();
                las.push_back(new LA::TypeInst("int64", flag));
                las.push_back(new LA::AssignOpInst(flag, ifInst->lt, ifInst->rt, opToString(ifInst->op)));
                las.push_back(new LA::BranchInst(flag, ifInst->lb, ifInst->rb));
            } else if (LabelInst *labelInst = dynamic_cast<LabelInst *>(inst)) {
                if (beginToWhile.count(labelInst->lb) > 0 && loopStack.top() != beginToWhile.at(labelInst->lb)) {
                    loopStack.push(beginToWhile.at(labelInst->lb));
                } else if (EndToWhile.count(labelInst->lb) > 0) {
                    loopStack.pop();
                }
                las = labelInst->getLA();
            } else if (WhileInst *whileInst = dynamic_cast<WhileInst *>(inst)) {
                if (loopStack.empty() || loopStack.top() != whileInst) {
                    loopStack.push(whileInst);
                }
                string flag = getAnotherFlagVar();
                las.push_back(new LA::TypeInst("int64", flag));
                las.push_back(new LA::AssignOpInst(flag, whileInst->lt, whileInst->rt, opToString(whileInst->op)));
                las.push_back(new LA::BranchInst(flag, whileInst->lb, whileInst->rb));
            } else if (ContinueInst *continueInst = dynamic_cast<ContinueInst *>(inst)) {
                las.push_back(new LA::BranchInst(WhileToCond[loopStack.top()]));
            } else if (BreakInst *breakInst = dynamic_cast<BreakInst *>(inst)) {
                las.push_back(new LA::BranchInst(whileToEnd[loopStack.top()]));
            } else if (Scope *scope = dynamic_cast<Scope *>(inst)) {
                throw runtime_error("got scope even after removing scope");
            } else {
                las = inst->getLA();
            }
            f->instructions.insert(f->instructions.end(), las.begin(), las.end());
        }

        for (auto const &inst : insts) {
            delete inst;
        }
        return f;
    }

    Program::~Program() {
        for (auto const &f : functions) {
            delete f;
        }
    }

    LA::Program Program::getLA() {
        LA::Program p;
        p.name = name;
        for (auto const &f : functions) {
            p.functions.push_back(f->getLA());
        }
        return p;
    }
}