#include <iostream>
#include <string>
#include <vector>

#include "l3.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/contrib/raw_string.hh>

namespace L3 {
    using namespace pegtl;
    using pegtl::plus;

#ifdef DEBUG
    int n = 0;
#endif

    struct kwDefine : string<'d', 'e', 'f', 'i', 'n', 'e'> {};
    struct kwLoad : string<'l', 'o', 'a', 'd'> {};
    struct kwStore : string<'s', 't', 'o', 'r', 'e'> {};
    struct kwBr : string <'b', 'r'> {};
    struct kwReturn : string<'r', 'e', 't', 'u', 'r', 'n'> {};
    struct kwCall : string<'c', 'a', 'l', 'l'> {};
    struct kwPrint : string<'p', 'r', 'i', 'n', 't'> {};
    struct kwAllocate : string<'a', 'l', 'l', 'o', 'c', 'a', 't', 'e'> {};
    struct kwArrayError : string<'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r'> {};

    struct opAssign : string<'<', '-'> {};

    struct cmt : disable<one<';'>, until<eolf>> {};
    struct seps : star<sor<ascii::space, cmt>> {};

    struct op : sor<one<'+'>, one<'-'>, one<'*'>, one<'&'>, string<'<', '<'>, string<'>', '>'>> {};
    struct cmp : sor<one<'<'>, string<'<', '='>, one<'='>, string<'>', '='>, one<'>'>> {};
    struct v : seq<plus<sor<alpha, one<'_'>>>, star<sor<alpha, one<'_'>, digit>>> {};
    struct lb : seq<one<':'>, v> {};
    struct label : lb {};
    struct var : v {};
    struct N : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};
    struct tt : sor<v, N> {};
    struct t : tt {};
    struct uu : sor<v, lb> {};
    struct u : uu {};
    struct s : sor<tt, lb> {};
    struct callee : sor<kwPrint, kwAllocate, kwArrayError, uu> {};
    struct vars : seq<var, star<seq<seps, one<','>, seps, var>>> {};
    struct args : seq<t, star<seq<seps, one<','>, seps, t>>> {};

    struct assignInst : seq<var, seps, opAssign, seps, s> {};
    struct assignOpInst : seq<var, seps, opAssign, seps, t, seps, op, seps, t> {};
    struct assignCmpInst : seq<var, seps, opAssign, seps, t, seps, cmp, seps, t> {};
    struct loadInst : seq<var, seps, opAssign, seps, kwLoad, seps, var> {};
    struct storeInst : seq<kwStore, seps, var, seps, opAssign, seps, s> {};
    struct branchLabelInst : seq<kwBr, seps, label> {};
    struct labelInst : label {};
    struct branchVarInst : seq<kwBr, seps, var, seps, label, seps, label> {};
    struct returnInst : kwReturn {};
    struct returnVarInst : seq<kwReturn, seps, t> {};
    struct callInstTmp : seq<kwCall, seps, callee, seps, one<'('>, seps, args, seps, one<')'>> {};
    struct callInst : callInstTmp {};
    struct assignCallInst : seq<var, seps, opAssign, seps, callInstTmp> {};

    struct i : sor<
            labelInst,
            returnVarInst,
            returnInst,
            branchVarInst,
            branchLabelInst,
            storeInst,
            callInst,
            assignCallInst,
            loadInst,
            assignOpInst,
            assignCmpInst,
            assignInst
        > {};
    struct fHead : seq<kwDefine, seps, label, seps, one<'('>, seps, opt<vars>, seps, one<')'>> {};
    struct fBody : seq<one<'{'>, plus<seq<seps, i>>, seps, one<'}'>> {};
    struct f : seq<fHead, seps, fBody> {};
    struct p : seq<plus<seq<seps, f>>, seps> {};

    vector<std::string> operators;
    vector<std::string> operands;
    inline void clearCache() {
        operators.clear();
        operands.clear();
    }

    template<typename Rule>
    struct action : nothing<Rule> {};

    template<>
    struct action<callee> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<s> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<t> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<u> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<op> {
        static void apply(const input &in, Program &p) {
            operators.push_back(in.string());
        }
    };

    template<>
    struct action<cmp> {
        static void apply(const input &in, Program &p) {
            operators.push_back(in.string());
        }
    };

    template<>
    struct action<label> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<var> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<fHead> {
        static void apply(const input &in, Program &p) {
#ifdef DEBUG
            n = 0;
#endif
            if (p.name.length() == 0) {
                p.name = operands.front();
            }
            Function *f = new Function;
            f->name = operands.front();
            operands.erase(operands.begin());
            f->arguments = operands;
            p.functions.push_back(f);
            clearCache();
        }
    };

    template<>
    struct action<assignInst> {
        static void apply(const input &in, Program &p) {
            AssignInst *inst = new AssignInst;
            inst->var = operands[operands.size() - 2];
            inst->s = operands[operands.size() - 1];
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<assignOpInst> {
        static void apply(const input &in, Program &p) {
            AssignOpInst *inst = new AssignOpInst;
            inst->var = operands[operands.size() - 3];
            inst->lt = operands[operands.size() - 2];
            inst->rt = operands[operands.size() - 1];
            inst->op = parseOp(operators[operators.size() - 1]);
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<assignCmpInst> {
        static void apply(const input &in, Program &p) {
            AssignCmpInst *inst = new AssignCmpInst;
            inst->var = operands[operands.size() - 3];
            inst->lt = operands[operands.size() - 2];
            inst->rt = operands[operands.size() - 1];
            inst->cmp = parseCmp(operators[operators.size() - 1]);
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<loadInst> {
        static void apply(const input &in, Program &p) {
            LoadInst *inst = new LoadInst;
            inst->lvar = operands[operands.size() - 2];
            inst->rvar = operands[operands.size() - 1];
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<storeInst> {
        static void apply(const input &in, Program &p) {
            StoreInst *inst = new StoreInst;
            inst->var = operands[operands.size() - 2];
            inst->s = operands[operands.size() - 1];
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<branchLabelInst> {
        static void apply(const input &in, Program &p) {
            BranchInst *inst = new BranchInst;
            inst->llabel = operands[operands.size() - 1];
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<labelInst> {
        static void apply(const input &in, Program &p) {
            LabelInst *inst = new LabelInst;
            inst->label = in.string();
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<branchVarInst> {
        static void apply(const input &in, Program &p) {
            BranchInst *inst = new BranchInst;
            inst->var = operands[operands.size() - 3];
            inst->llabel = operands[operands.size() - 2];
            inst->rlabel = operands[operands.size() - 1];
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<returnInst> {
        static void apply(const input &in, Program &p) {
            ReturnInst *inst = new ReturnInst;
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<returnVarInst> {
        static void apply(const input &in, Program &p) {
            ReturnInst *inst = new ReturnInst;
            inst->var = operands[operands.size() - 1];
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<callInst> {
        static void apply(const input &in, Program &p) {
            CallInst *inst = new CallInst;
            inst->callee = operands.front();
            operands.erase(operands.begin());
            inst->args = operands;
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

    template<>
    struct action<assignCallInst> {
        static void apply(const input &in, Program &p) {
            AssignCallInst *inst = new AssignCallInst;
            inst->var = operands[0];
            inst->callee = operands[1];
            operands.erase(operands.begin(), operands.begin() + 2);
            inst->args = operands;
            p.functions.back()->instructions.push_back(inst);
            clearCache();
        }
    };

#ifdef DEBUG
    template<>
    struct action<i> {
        static void apply(const input &in, Program &p) {
            cout << ++n << "\t" << *p.functions.back()->instructions.back() << endl;
        }
    };
#endif

    Program L3ParseFile(char *file) {
        struct grammar : must<p> {};
        analyze<grammar>();
        Program p;
        file_parser(file).parse<grammar, action>(p);
        return p;
    }

}