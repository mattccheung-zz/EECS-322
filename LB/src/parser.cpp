#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include "lb.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/contrib/raw_string.hh>

namespace LB {
    using namespace pegtl;
    using pegtl::plus;
    using pegtl::string;

#ifdef DEBUG
    int n = 0;
#endif

    struct kwIf : string<'i', 'f'> {};
    struct kwReturn : string<'r', 'e', 't', 'u', 'r', 'n'> {};
    struct kwWhile : string<'w', 'h', 'i', 'l', 'e'> {};
    struct kwContinue : string<'c', 'o', 'n', 't', 'i', 'n', 'u', 'e'> {};
    struct kwBreak : string<'b', 'r', 'e', 'a', 'k'> {};
    struct kwLength : string<'l', 'e', 'n', 'g', 't', 'h'> {};
    struct kwCall : string<'c', 'a', 'l', 'l'> {};
    struct kwNew : string<'n', 'e', 'w'> {};
    struct kwCapArray : string<'A', 'r', 'r', 'a', 'y'> {};
    struct kwCapTuple : string<'T', 'u', 'p', 'l', 'e'> {};
    struct kwVoid : string<'v', 'o', 'i', 'd'> {};
    struct kwInt64 : string<'i', 'n', 't', '6', '4'> {};
    struct kwTuple : string<'t', 'u', 'p', 'l', 'e'> {};
    struct kwCode : string<'c', 'o', 'd', 'e'> {};

    struct opAssign : string<'<', '-'> {};
    struct opBrackets : string<'[', ']'> {};

    struct cmt : disable<one<';'>, until<eolf>> {};
    struct wSeps : star<sor<ascii::space, cmt>> {};
    struct sSeps : plus<sor<ascii::space, cmt>> {};

    struct op : sor<one<'+'>, one<'-'>, one<'*'>, one<'&'>, string<'<', '<'>, string<'>', '>'>,
        one<'<'>, string<'<', '='>, one<'='>, string<'>', '='>, one<'>'>> {};
    struct v : seq<plus<sor<alpha, one<'_'>>>, star<sor<alpha, one<'_'>, digit>>> {};
    struct name : v {};
    struct lb : seq<one<':'>, v> {};
    struct label : lb {};
    struct vv : seq<one<'%'>, v> {};
    struct var : vv {};
    struct N : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};
    struct tt : sor<vv, N> {};
    struct t : tt {};
    struct s : sor<tt, lb> {};
    struct callee : sor<vv, v> {};
    struct vars : seq<var, star<seq<wSeps, one<','>, wSeps, var>>> {};
    struct args : seq<t, star<seq<wSeps, one<','>, wSeps, t>>> {};
    struct tty : sor<kwTuple, kwCode, seq<kwInt64, star<opBrackets>>> {};
    struct type : tty {};
    struct T : sor<tty, kwVoid> {};
    struct cond : seq<t, sSeps, op, sSeps, t> {};
    struct varAccess : seq<var, plus<seq<wSeps, one<'['>, wSeps, t, wSeps, one<']'>>>> {};
    struct funcArgs : seq<type, sSeps, var, star<seq<wSeps, one<','>, wSeps, type, sSeps, var>>> {};

    struct typeInst : seq<type, sSeps, vars> {};
    struct assignInst : seq<var, wSeps, opAssign, wSeps, s> {};
    struct assignCondInst : seq<var, wSeps, opAssign, wSeps, t, wSeps, op, wSeps, t> {};
    struct labelInst : label {};
    struct ifInst : seq<kwIf, wSeps, one<'('>, wSeps, cond, wSeps, one<')'>, wSeps, label, sSeps, label> {};
    struct returnInst : seq<kwReturn, opt<sSeps, t>> {};
    struct whileInst : seq<kwWhile, wSeps, one<'('>, wSeps, cond, wSeps, one<')'>, wSeps, label, sSeps, label> {};
    struct continueInst : kwContinue {};
    struct breakInst : kwBreak {};
    struct arrayToVarInst : seq<var, wSeps, opAssign, wSeps, varAccess> {};
    struct varToArrayInst : seq<varAccess, wSeps, opAssign, wSeps, s> {};
    struct assignLengthInst : seq<var, wSeps, opAssign, wSeps, kwLength, sSeps, var, sSeps, t> {};
    struct callInstTmp : seq<kwCall, sSeps, callee, wSeps, one<'('>, wSeps, opt<args>, wSeps, one<')'>> {};
    struct callInst : callInstTmp {};
    struct assignCallInst : seq<var, wSeps, opAssign, wSeps, callInstTmp> {};
    struct newArrayInst : seq<var, wSeps, opAssign, wSeps, kwNew, sSeps, kwCapArray, wSeps, one<'('>, wSeps, args, wSeps, one<')'>> {};
    struct newTupleInst : seq<var, wSeps, opAssign, wSeps, kwNew, sSeps, kwCapTuple, wSeps, one<'('>, wSeps, t, wSeps, one<')'>> {};

    struct scope;
    struct inst : sor<
        scope,
        labelInst,
        returnInst,
        ifInst,
        typeInst,
        whileInst,
        continueInst,
        breakInst,
        callInst,
        newTupleInst,
        newArrayInst,
        assignCallInst,
        assignLengthInst,
        arrayToVarInst,
        varToArrayInst,
        assignCondInst,
        assignInst> {};
    struct scope : seq<one<'{'>, wSeps, inst, star<sSeps, inst>, wSeps, one<'}'>> {};
    struct fHead : seq<T, sSeps, name, wSeps, one<'('>, wSeps, opt<funcArgs>, wSeps, one<')'>> {};
    struct f : seq<fHead, wSeps, scope> {};
    struct p : seq<wSeps, f, star<seq<sSeps, f>>, wSeps> {};


    vector<std::string> operands;

    template<typename Rule>
    struct action : nothing<Rule> {};

    template<>
    struct action<T> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<type> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

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
    struct action<op> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
        }
    };

    template<>
    struct action<name> {
        static void apply(const input &in, Program &p) {
            operands.push_back(in.string());
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
//            assert(operands.size() % 2 == 0 && operands.size() >= 2);
//            if (p.name.empty()) {
//                p.name = operands[1];
//            }
//            Function *f = new Function;
//            f->name = operands[1];
//            f->returnType = Type(operands[0]);
//            for (int i = 2; i < operands.size(); i += 2) {
//                f->arguments.push_back(new TypeInst(operands[i], operands[i + 1]));
//            }
//            p.functions.push_back(f);
//            operands.clear();
#ifdef DEBUG
            n = 0;
            //cout << "function " << f->name << endl;
#endif
        }
    };

    template<>
    struct action<typeInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.size() == 2);
//            p.functions.back()->instructions.push_back(new TypeInst(operands[0], operands[1]));
//            operands.clear();
        }
    };

    template<>
    struct action<assignInst> {
        static void apply(const input &in, Program &p) {
//            p.functions.back()->instructions.push_back(new AssignInst(operands[operands.size() - 2], operands.back()));
//            operands.clear();
        }
    };

    template<>
    struct action<assignCondInst> {
        static void apply(const input &in, Program &p) {
//            p.functions.back()->instructions.push_back(new AssignOpInst(operands[operands.size() - 4], operands[operands.size() - 3],
//                                                                        operands.back(), operands[operands.size() - 2]));
//            operands.clear();
        }
    };

    template<>
    struct action<labelInst> {
        static void apply(const input &in, Program &p) {
//            p.functions.back()->instructions.push_back(new LabelInst(in.string()));
//            operands.clear();
        }
    };

    template<>
    struct action<ifInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.size() == 1);
//            p.functions.back()->instructions.push_back(new BranchInst(operands[0]));
//            operands.clear();
        }
    };

    template<>
    struct action<returnInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.empty());
//            p.functions.back()->instructions.push_back(new ReturnInst());
        }
    };

    template<>
    struct action<whileInst> {
        static void apply(const input &in, Program &p) {
        }
    };

    template<>
    struct action<continueInst> {
        static void apply(const input &in, Program &p) {
        }
    };

    template<>
    struct action<breakInst> {
        static void apply(const input &in, Program &p) {
        }
    };

    template<>
    struct action<arrayToVarInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.size() >= 7);
//            std::string lv = operands[4], rv = operands[5];
//            operands.erase(operands.begin(), operands.begin() + 6);
//            p.functions.back()->instructions.push_back(new AssignInst(lv, rv, operands));
//            operands.clear();
        }
    };

    template<>
    struct action<varToArrayInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.size() >= 8);
//            std::string var = operands[5], s = operands.back();
//            operands.erase(operands.begin(), operands.begin() + 6);
//            operands.pop_back();
//            p.functions.back()->instructions.push_back(new AssignInst(var, operands, s));
//            operands.clear();
        }
    };

    template<>
    struct action<assignLengthInst> {
        static void apply(const input &in, Program &p) {
//            p.functions.back()->instructions.push_back(new AssignLengthInst(operands[operands.size() - 3], operands[operands.size() - 2],
//                                                        operands.back()));
//            operands.clear();
        }
    };

    template<>
    struct action<callInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.size() >= 1);
//            std::string callee = operands[0];
//            operands.erase(operands.begin());
//            p.functions.back()->instructions.push_back(new AssignCallInst(callee, operands));
//            operands.clear();
        }
    };

    template<>
    struct action<assignCallInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.size() >= 4);
//            std::string var = operands[2], callee = operands[3];
//            operands.erase(operands.begin(), operands.begin() + 4);
//            p.functions.back()->instructions.push_back(new AssignCallInst(var, callee, operands));
//            operands.clear();
        }
    };

    template<>
    struct action<newArrayInst> {
        static void apply(const input &in, Program &p) {
//            assert(operands.size() >= 3);
//            std::string var = operands[1];
//            operands.erase(operands.begin(), operands.begin() + 2);
//            p.functions.back()->instructions.push_back(new NewArrayInst(var, operands));
//            operands.clear();
        }
    };

    template<>
    struct action<newTupleInst> {
        static void apply(const input &in, Program &p) {
//            p.functions.back()->instructions.push_back(new NewTupleInst(operands[operands.size() - 2], operands.back()));
//            operands.clear();
        }
    };

#ifdef DEBUG
    template<>
    struct action<inst> {
        static void apply(const input &in, Program &p) {
//            cout << ++n << "\t" << *p.functions.back()->instructions.back() << endl;
        }
    };
#endif


    Program ParseFile(char *file) {
        struct grammar : must<p> {};
        analyze<grammar>();
        Program p;
        file_parser(file).parse<grammar, action>(p);
        return p;
    }
}