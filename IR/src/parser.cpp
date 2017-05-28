#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include "ir.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/contrib/raw_string.hh>

namespace IR {
    using namespace pegtl;
    using pegtl::plus;
    using pegtl::string;

#ifdef DEBUG
    int n = 0;
#endif

    struct kwDefine : string<'d', 'e', 'f', 'i', 'n', 'e'> {};
    struct kwBr : string<'b', 'r'> {};
    struct kwLength : string<'l', 'e', 'n', 'g', 't', 'h'> {};
    struct kwReturn : string<'r', 'e', 't', 'u', 'r', 'n'> {};
    struct kwCall : string<'c', 'a', 'l', 'l'> {};
    struct kwNewArray : string<'n', 'e', 'w', ' ', 'A', 'r', 'r', 'a', 'y'> {};
    struct kwNewTuple : string<'n', 'e', 'w', ' ', 'T', 'u', 'p', 'l', 'e'> {};
    struct kwVoid : string<'v', 'o', 'i', 'd'> {};
    struct kwInt64 : string<'i', 'n', 't', '6', '4'> {};
    struct kwTuple : string<'t', 'u', 'p', 'l', 'e'> {};
    struct kwCode : string<'c', 'o', 'd', 'e'> {};
    struct kwPrint : string<'p', 'r', 'i', 'n', 't'> {};
    struct kwArrayError : string<'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r'> {};

    struct opAssign : string<'<', '-'> {};
    struct opBrackets : string<'[', ']'> {};

    struct cmt : disable<one<';'>, until<eolf>> {};
    struct seps : star<sor<ascii::space, cmt>> {};

    struct op : sor<one<'+'>, one<'-'>, one<'*'>, one<'&'>, string<'<', '<'>, string<'>', '>'>,
                string<'<', '='>, one<'<'>, one<'='>, string<'>', '='>, one<'>'>> {};
    struct v : seq<plus<sor<alpha, one<'_'>>>, star<sor<alpha, one<'_'>, digit>>> {};
    struct lb : seq<one<':'>, v> {};
    struct label : lb {};
    struct vv : seq<one<'%'>, v> {};
    struct var : vv {};
    struct N : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};
    struct uu : sor<vv, lb> {};
    struct u : uu {};
    struct tt : sor<vv, N> {};
    struct t : tt {};
    struct s : sor<tt, lb> {};
    struct callee : sor<kwPrint, kwArrayError, uu> {};
    struct vars : seq<var, star<seq<seps, one<','>, seps, var>>> {};
    struct args : seq<t, star<seq<seps, one<','>, seps, t>>> {};
    struct tty : sor<kwTuple, kwCode, seq<kwInt64, star<opBrackets>>> {};
    struct type : tty {};
    struct T : sor<tty, kwVoid> {};
    struct varAccess : seq<var, plus<seq<one<'['>, t, one<']'>>>> {};
    struct funcArgs : seq<type, seps, var, star<seq<seps, one<','>, seps, type, seps, var>>> {};

    struct typeInst : seq<type, seps, var> {};
    struct assignInst : seq<var, seps, opAssign, seps, s> {};
    struct assignOpInst : seq<var, seps, opAssign, seps, t, seps, op, seps, t> {};
    struct assignArrayInst : seq<var, seps, opAssign, seps, varAccess> {};
    struct arrayAssignInst : seq<varAccess, seps, opAssign, seps, s> {};
    struct assignLengthInst : seq<var, seps, opAssign, seps, kwLength, seps, var, seps, t> {};
    struct callInstTmp : seq<kwCall, seps, callee, seps, one<'('>, seps, opt<args>, seps, one<')'>> {};
    struct callInst : callInstTmp {};
    struct assignCallInst : seq<var, seps, opAssign, seps, callInstTmp> {};
    struct newArrayInst : seq<var, seps, opAssign, seps, kwNewArray, seps, one<'('>, seps, args, seps, one<')'>> {};
    struct newTupleInst : seq<var, seps, opAssign, seps, kwNewTuple, seps, one<'('>, seps, t, seps, one<')'>> {};
    struct labelInst : label {};
    struct brLabelInst : seq<kwBr, seps, label> {};
    struct brVarLabelInst : seq<kwBr, seps, t, seps, label, seps, label> {};
    struct returnInst : kwReturn {};
    struct returnVarInst : seq<kwReturn, seps, t> {};
    struct teInst : sor<brLabelInst, brVarLabelInst, returnVarInst, returnInst> {};

    struct inst : sor<
            typeInst,
            callInst,
            arrayAssignInst,
            newArrayInst,
            newTupleInst,
            assignCallInst,
            assignLengthInst,
            assignArrayInst,
            assignOpInst,
            assignInst> {};
    struct bBlock : seq<labelInst, star<seq<seps, inst>>, seps, teInst> {};
    struct fHead : seq<kwDefine, seps, T, seps, label, seps, one<'('>, seps, opt<funcArgs>, seps, one<')'>> {};
    struct fBody : seq<one<'{'>, plus<seq<seps, bBlock>>, seps, one<'}'>> {};
    struct f : seq<fHead, seps, fBody> {};
    struct p : seq<plus<seq<seps, f>>, seps> {};


    vector<std::string> operators, operands;
    vector<Instruction *> instructions;
    inline void clearCache() {
        operators.clear();
        operands.clear();
    }

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

            assert(operands.size() % 2 == 0 && operands.size() >= 2);
            assert(operators.empty());
            if (p.name.empty()) {
                p.name = operands[1];
            }
            Function *f = new Function;
            f->name = operands[1];
            f->returnType = Type(operands[0]);
            for (int i = 2; i < operands.size(); i += 2) {
                f->arguments.push_back(new TypeInst(operands[i], operands[i + 1]));
            }
            p.functions.push_back(f);
            clearCache();
#ifdef DEBUG
            n = 0;
            cout << "function " << f->name << endl;
#endif
        }
    };

    template<>
    struct action<bBlock> {
        static void apply(const input &in, Program &p) {
            assert(operands.empty());
            assert(operators.empty());
            assert(!instructions.empty());
            p.functions.back()->basicBlocks.push_back(new BasicBlock(instructions));
            instructions.clear();
        }
    };

    template<>
    struct action<labelInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.empty());
            assert(operators.empty());
            instructions.push_back(new LabelInst(in.string()));
            clearCache();
#ifdef DEBUG
            cout << ++n << "\t" << *instructions.back() << endl;
#endif
        }
    };

    template<>
    struct action<brLabelInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() == 1);
            assert(operators.empty());
            instructions.push_back(new BranchInst(operands[0]));
            clearCache();
        }
    };

    template<>
    struct action<brVarLabelInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() == 3);
            assert(operators.empty());
            instructions.push_back(new BranchInst(operands[0], operands[1], operands[2]));
            clearCache();
        }
    };

    template<>
    struct action<returnInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.empty());
            assert(operators.empty());
            instructions.push_back(new ReturnInst());
        }
    };

    template<>
    struct action<returnVarInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() == 1);
            assert(operators.empty());
            instructions.push_back(new ReturnInst(operands[0]));
            clearCache();
        }
    };

    template<>
    struct action<typeInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() == 2);
            assert(operators.empty());
            instructions.push_back(new TypeInst(operands[0], operands[1]));
            clearCache();
        }
    };

    template<>
    struct action<assignInst> {
        static void apply(const input &in, Program &p) {
            instructions.push_back(new AssignInst(operands[operands.size() - 2], operands.back()));
            clearCache();
        }
    };

    template<>
    struct action<assignOpInst> {
        static void apply(const input &in, Program &p) {
            instructions.push_back(new AssignOpInst(operands[operands.size() - 3], operands[operands.size() - 2], operands.back(), operators.front()));
            clearCache();
        }
    };

    template<>
    struct action<assignArrayInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() >= 8);
            assert(operators.empty());
            std::string lv = operands[5], rv = operands[6];
            operands.erase(operands.begin(), operands.begin() + 7);
            instructions.push_back(new AssignInst(lv, rv, operands));
            clearCache();
        }
    };

    template<>
    struct action<arrayAssignInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() >= 3);
            assert(operators.empty());
            std::string var = operands.front(), s = operands.back();
            operands.erase(operands.begin());
            operands.pop_back();
            instructions.push_back(new AssignInst(var, operands, s));
            clearCache();
        }
    };

    template<>
    struct action<assignLengthInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() == 7);
            assert(operators.empty());
            instructions.push_back(new AssignLengthInst(operands[operands.size() - 3], operands[operands.size() - 2], operands.back()));
            clearCache();
        }
    };

    template<>
    struct action<callInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() >= 1);
            assert(operators.empty());
            std::string callee = operands[0];
            operands.erase(operands.begin());
            instructions.push_back(new AssignCallInst(callee, operands));
            clearCache();
        }
    };

    template<>
    struct action<assignCallInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() >= 5);
            assert(operators.empty());
            std::string var = operands[3], callee = operands[4];
            operands.erase(operands.begin(), operands.begin() + 5);
            instructions.push_back(new AssignCallInst(var, callee, operands));
            clearCache();
        }
    };

    template<>
    struct action<newArrayInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() >= 3);
            assert(operators.empty());
            std::string var = operands[1];
            operands.erase(operands.begin(), operands.begin() + 2);
            instructions.push_back(new NewArrayInst(var, operands));
            clearCache();
        }
    };

    template<>
    struct action<newTupleInst> {
        static void apply(const input &in, Program &p) {
            assert(operands.size() == 4);
            assert(operators.empty());
            instructions.push_back(new NewTupleInst(operands[operands.size() - 2], operands.back()));
            clearCache();
        }
    };

#ifdef DEBUG
    template<>
    struct action<inst> {
        static void apply(const input &in, Program &p) {
            cout << ++n << "\t" << *instructions.back() << endl;
        }
    };

    template<>
    struct action<teInst> {
        static void apply(const input &in, Program &p) {
            cout << ++n << "\t" << *instructions.back() << endl;
        }
    };
#endif


    Program IRParseFile(char *file) {
        struct grammar : must<p> {};
        analyze<grammar>();
        Program p;
        file_parser(file).parse<grammar, action>(p);
        return p;
    }
}