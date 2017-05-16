#include <iostream>
#include <string>
#include <vector>

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
                one<'<'>, string<'<', '='>, one<'='>, string<'>', '='>, one<'>'>> {};
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

    struct typeInst : seq<type, seps, var> {};
    struct assignInst : seq<var, seps, opAssign, seps, s> {};
    struct assignOpInst : seq<var, seps, opAssign, seps, t, seps, op, seps, t> {};
    struct assignArrayInst : seq<> {};


    Program IRParseFile(char *file) {
//        struct grammar : must<p> {};
//        analyze<grammar>();
        Program p;
//        file_parser(file).parse<grammar, action>(p);
        return p;
    }
}