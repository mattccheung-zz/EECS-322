#include <sstream>
#include <cstdlib>
#include <map>
#include <cassert>

#include "ir.h"

using namespace std;


namespace IR {

    inline string strip(const string &s) {
        return s[0] == '%' ? s.substr(1) : s;
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

    vector <string> LabelInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        l3.push_back(lb == ":entry" ? lb + genRandStr(2) : lb);
        return l3;
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

    vector <string> BranchInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        if (t.empty()) {
            l3.push_back("br " + lb);
        } else {
            l3.push_back("br " + strip(t) + " " + lb + " " + rb);
        }
        return l3;
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

    vector <string> ReturnInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        if (t.empty()) {
            l3.push_back("return");
        } else {
            l3.push_back("return " + strip(t));
        }
        return l3;
    }

    TypeInst::TypeInst(const string &t, const string &v) {
        type = Type(t);
        var = v;
    }

    void TypeInst::print(ostream &os) {
        //os << ";" << type.toString() << " " << var;
    }

    vector <string> TypeInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        return l3;
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

    vector <string> AssignInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        string addr = "_addr_asn_inst_", dim = "_dim_asn_inst_", ofst = "_ofst_asn_inst_", fct = "_fct_asn_inst_";
        if (!varIndex.empty() || !sIndex.empty()) {
            vector<string> indexes = varIndex.empty() ? sIndex : varIndex;
            string array = varIndex.empty() ? s : var;
            assert(varMap.count(array) > 0);
            if (varMap.at(array).type == TUPLE) {
                assert(indexes.size() == 1);
                if (isVar(indexes[0])) {
                    l3.push_back(addr + " <- " + strip(indexes[0]));
                    l3.push_back(addr + " <- " + addr + " + 1");
                } else {
                    l3.push_back(addr + " <- " + to_string(stoll(indexes[0]) + 1));
                }
            } else {
                l3.push_back(ofst + " <- " + to_string(indexes.size() + 1));
                l3.push_back(ofst + " <- " + ofst + " * 8");
                l3.push_back(ofst + " <- " + ofst + " + " + strip(array));
                l3.push_back(fct + " <- 1");
                l3.push_back(addr + " <- " + to_string(indexes.size() + 2) + " + " + strip(indexes.back()));
                for (int i = indexes.size() - 2; i >= 0; i--) {
                    l3.push_back(dim + " <- load " + ofst);
                    l3.push_back(dim + " <- " + dim + " >> 1");
                    l3.push_back(fct + " <- " + fct + " * " + dim);
                    l3.push_back(dim + " <- " + fct + " * " + indexes[i]);
                    l3.push_back(addr + " <- " + addr + " + " + dim);
                    l3.push_back(ofst + " <- " + ofst + " - 8");
                }
            }
            l3.push_back(addr + " <- " + addr + " * 8");
            if (varIndex.empty()) {
                l3.push_back(addr + " <- " + addr + " + " + strip(s));
                l3.push_back(strip(var) + " <- load " + addr);
            } else {
                l3.push_back(addr + " <- " + addr + " + " + strip(var));
                l3.push_back("store " + addr + " <- " + strip(s));
            }
        } else {
            l3.push_back(strip(var) + " <- " + strip(s));
        }
        return l3;
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

    vector <string> AssignOpInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        l3.push_back(strip(var) + " <- " + strip(lt) + " " + opToString(op) + " " + strip(rt));
        return l3;
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

    vector <string> AssignLengthInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        string addr = "_addr_len_inst_";
        if (isVar(t)) {
            l3.push_back(addr + " <- " + strip(t) + " * 8");
            l3.push_back(addr + " <- " + addr + " + 16");
        } else {
            l3.push_back(addr + " <- " + to_string(stoll(t) * 8 + 16));
        }
        l3.push_back(addr + " <- " + addr + " + " + strip(rv));
        l3.push_back(strip(lv) + " <- load " + addr);
        return l3;
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

    vector <string> AssignCallInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        stringstream ss;
        if (!var.empty()) {
            ss << strip(var) << " <- ";
        }
        ss << "call " << strip(callee) << "(";
        for (int i = 0; i < args.size(); i++) {
            if (i == 0) {
                ss << strip(args[i]);
            } else {
                ss << ", " << strip(args[i]);
            }
        }
        ss << ")";
        l3.push_back(ss.str());
        return l3;
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
               << "    " << "store " << addr << " <- " << strip(arg) << endl;
        }
    }

    vector <string> NewArrayInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        string cnt = "_cnt_nry_inst_", dim = "_dim_nry_inst_", addr = "_addr_nry_inst_";
        l3.push_back(cnt + " <- 1");
        for (auto const &arg : args) {
            l3.push_back(dim + " <- " + strip(arg) + " >> 1");
            l3.push_back(cnt + " <- " + cnt + " * " + dim);
        }
        l3.push_back(cnt + " <- " + cnt + " + " + to_string(args.size() + 1));
        l3.push_back(cnt + " <- " + cnt + " << 1");
        l3.push_back(cnt + " <- " + cnt + " + 1");
        l3.push_back(strip(var) + " <- call allocate(" + cnt + ", 1)");
        l3.push_back(addr + " <- " + strip(var) + " + 8");
        l3.push_back("store " + addr + " <- " + to_string(args.size() * 2 + 1));
        for (auto const &arg : args) {
            l3.push_back(addr + " <- " + addr + " + 8");
            l3.push_back("store " + addr + " <- " + strip(arg));
        }
        return l3;
    }

    NewTupleInst::NewTupleInst(const string &v, const string &t) {
        var = v;
        this->t = t;
    }

    void NewTupleInst::print(ostream &os) {
        os << strip(var) << " <- call allocate(" << strip(t) << ", 1)" << endl;
    }

    vector <string> NewTupleInst::toL3(const map <string, Type> &varMap) {
        vector <string> l3;
        l3.push_back(strip(var) + " <- call allocate(" + strip(t) + ", 1)");
        return l3;
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
        map <string, Type> varMap;
        stringstream ss;
        ss << "define " << name << " (";
        for (int i = 0; i < arguments.size(); i++) {
            if (i == 0) {
                ss << arguments[i]->var.substr(1);
            } else {
                ss << ", " << arguments[i]->var.substr(1);
            }
            varMap[arguments[i]->var] = arguments[i]->type;
        }
        ss << ") {" << endl;
        for (auto const &bb : basicBlocks) {
            for (auto const &inst : bb->instructions) {
                if (TypeInst *typeInst = dynamic_cast<TypeInst *>(inst)) {
                    varMap[typeInst->var] = typeInst->type;
                } else {
                    for (auto const &l3 : inst->toL3(varMap)) {
                        ss << "    " << l3 << endl;
                    }
                }
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