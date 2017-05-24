#include <sstream>
#include <cstdlib>
#include <set>
#include <cassert>

#include "la.h"

using namespace std;


namespace LA {

    inline bool isRunTime(const string &s) {
        return s == "print" || s == "array-error";
    }

    inline bool isNum(const string &s) {
        return s[0] == '+' || s[0] == '-' || (s[0] >= '0' && s[0] <= '9');
    }

    inline string encode(const string &s) {
        return to_string(stoll(s) * 2 + 1);
    }

    inline string encodeIfNum(const string &s) {
        return isNum(s) ? encode(s) : s;
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
        os << lb;
    }

    vector <string> LabelInst::toIR(set <string> &nVarSet) {
        return {lb};
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

    vector <string> BranchInst::toIR(set <string> &nVarSet) {
        vector <string> ir;
        if (t.empty()) {
            ir.push_back("br " + lb);
        } else {
            if (!isNum(t)) {
                ir.push_back(t + " <- " + t + " >> 1");
            }
            ir.push_back("br " + t + " " + lb + " " + rb);
        }
        return ir;
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

    vector <string> ReturnInst::toIR(set <string> &nVarSet) {
        return {t.empty() ? "return" : "return " + t};
    }

    TypeInst::TypeInst(const string &t, const string &v) {
        type = Type(t);
        var = v;
    }

    void TypeInst::print(ostream &os) {
        os << type.toString() << " " << var;
    }

    vector <string> TypeInst::toIR(set <string> &nVarSet) {
        vector <string> ir = {type.toString() + " " + var};
        if (type.type == TUPLE || type.dim > 0) {
            ir.push_back(var + " <- 0");
        }
        return ir;
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

    vector <string> AssignInst::toIR(set <string> &nVarSet) {
        vector <string> ir;
        vector <string> &indices = varIndex;
        string v;
        if (!varIndex.empty()) {
            indices = varIndex;
            v = var;
        } else if (!sIndex.empty()) {
            indices = sIndex;
            v = s;
        }
        if (!v.empty()) {
            //TODO: array access check
            vector <string> nIndices;
            for (auto const &idx : indices) {
                string nIdx = idx;
                if (!isNum(idx)) {
                    nVarSet.insert(nIdx += "_decoded_");
                    ir.push_back(nIdx + " <- " + idx + " >> 1");
                }
                nIndices.push_back(nIdx);
            }
            stringstream ss;
            ss << v;
            for (auto const &nIdx : nIndices) {
                ss << "[" << nIdx << "]";
            }
            if (!varIndex.empty()) {
                ir.push_back(ss.str() + " <- " + encodeIfNum(s));
            } else {
                ir.push_back(var + " <- " + ss.str());
            }
        } else {
            ir.push_back(var + " <- " + encodeIfNum(s));
        }
        return ir;
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

    vector <string> AssignOpInst::toIR(set <string> &nVarSet) {
        vector <string> ir;
        string nlt = lt, nrt = rt;
        if (!isNum(lt)) {
            nVarSet.insert(nlt += "_decoded_");
            ir.push_back(nlt + " <- " + lt + " >> 1");
        }
        if (!isNum(rt)) {
            nVarSet.insert(nrt += "_decoded_");
            ir.push_back(nrt + " <- " + rt + " >> 1");
        }
        ir.push_back(var + " <- " + nlt + " " + opToString(op) + " " + nrt);
        ir.push_back(var + " <- " + var + " << 1");
        ir.push_back(var + " <- " + var + " + 1");
        return ir;
    }

    AssignLengthInst::AssignLengthInst(const string &lv, const string &rv, const string &t) {
        this->lv = lv;
        this->rv = rv;
        this->t = t;
    }

    void AssignLengthInst::print(ostream &os) {
        os << lv << " <- length " << rv << " " << t;
    }

    vector <string> AssignLengthInst::toIR(set <string> &nVarSet) {
        vector <string> ir;
        string nt = t;
        if (!isNum(t)) {
            nVarSet.insert(nt += "_decoded_");
            ir.push_back(nt + " <- " + t + " >> 1");
        }
        ir.push_back(lv + " <- length " + rv + " " + nt);
        return ir;
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
        os << "call " << callee << "(";
        for (int i = 0; i < args.size(); i++) {
            if (i > 0) {
                os << ", ";
            }
            os << args[i];
        }
        os << ")";
    }

    vector <string> AssignCallInst::toIR(set <string> &nVarSet) {
        stringstream ss;
        if (!var.empty()) {
            ss << var << " <- ";
        }
        ss << "call " << (isRunTime(callee) ? callee : ":" + callee) << "(";
        for (int i = 0; i < args.size(); i++) {
            if (i > 0) {
                ss << ", ";
            }
            ss << encodeIfNum(args[i]);
        }
        ss << ")";
        return {ss.str()};
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

    vector <string> NewArrayInst::toIR(set <string> &nVarSet) {
        stringstream ss;
        ss << var << " <- new Array(";
        for (int i = 0; i < args.size(); i++) {
            if (i > 0) {
                ss << ", ";
            }
            ss << encodeIfNum(args[i]);
        }
        ss << ")";
        return {ss.str()};
    }

    NewTupleInst::NewTupleInst(const string &v, const string &t) {
        var = v;
        this->t = t;
    }

    void NewTupleInst::print(ostream &os) {
        os << var << " <- new Tuple(" << t << ")";
    }

    vector <string> NewTupleInst::toIR(set <string> &nVarSet) {
        return {var + " <- new Tuple(" + encodeIfNum(t) + ")"};
    }

    Function::~Function() {
        for (auto const &arg : arguments) {
            delete arg;
        }
        for (auto const &inst : instructions) {
            delete inst;
        }
    }

    string Function::toIR() {
        set<string> nVarSet;
        stringstream ss;
        ss << "define " << returnType.toString() << " :" << name << "(";
        for (int i = 0; i < arguments.size(); i++) {
            if (i > 0) {
                ss << ", ";
            }
            ss << arguments[i]->type.toString() << " " << arguments[i]->var;
        }
        ss << ") {" << endl;
        vector <string> irs;
        bool startBasicBlock = true;
        for (auto const &inst : instructions) {
            if (startBasicBlock) {
                if (dynamic_cast<LabelInst *>(inst) == NULL) {
                    irs.push_back(":_entry_" + genRandStr(4) + "_");
                }
                startBasicBlock = false;
            } else if (LabelInst *labelInst = dynamic_cast<LabelInst *>(inst)) {
                irs.push_back("br " + labelInst->lb);
            }
            vector<string> tmp = inst->toIR(nVarSet);
            irs.insert(irs.end(), tmp.begin(), tmp.end());
            if (dynamic_cast<BranchInst *>(inst) || dynamic_cast<ReturnInst *>(inst)) {
                startBasicBlock = true;
            }
        }
        for (auto const &nVar : nVarSet) {
            irs.insert(irs.begin() + 1, "int64 " + nVar);
        }
        for (auto const &s : irs) {
            ss << "    " << s << endl;
        }
        ss << "}" << endl;
        return ss.str();
    }

    Program::~Program() {
        for (auto const &f : functions) {
            delete f;
        }
    }

    string Program::toIR() {
        stringstream ss;
        for (auto const &f : functions) {
            ss << f->toIR() << endl;
        }
        return ss.str();
    }
}