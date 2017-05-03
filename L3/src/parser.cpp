


#include "l3.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/contrib/raw_string.hh>

using namespace pegtl;

namespace L2 {
    struct p;
    struct f;
    struct i;
    struct callee;
    struct vars;
    struct args;
    struct s;
    struct t;
    struct u;
    struct op;
    struct cmp;
    struct label;
    struct var;
    struct N;

    struct comment : disable<one<';'>, until<eolf>> {};
    struct seps : star<sor<ascii::space, comment>> {};


    struct p : plus<f> {};
    struct f : seq<
            string<'d', 'e', 'f', 'i', 'n', 'e'>, seps, label, seps, one<'('>, seps, vars, seps, one<')'>, seps,
            one<'{'>, seps, plus<i>, seps, one<'}'>> {};
    struct i : sor<
            seq<var, seps, string<'<', '-'>, seps, s>,
            seq<var, seps, string<'<', '-'>, seps, t, seps, op, seps, t>,
            seq<var, seps, string<'<', '-'>, seps, t, seps, cmp, seps, t>,
            seq<var, seps, string<'l', 'o', 'a', 'd'>, seps, var>,
            seq<string<'s', 't', 'o', 'r', 'e'>, seps, var, string<'<', '-'>, seps, s>,
            seq<string<'b', 'r'>, seps, label>,
            label,
            seq<string<'b', 'r'>, seps, var, seps, label, seps, label>,
            string<'r', 'e', 't', 'u', 'r', 'n'>,
            seq<string<'r', 'e', 't', 'u', 'r', 'n'>, seps, var>,
            seq<string<'c', 'a', 'l', 'l'>, seps, callee, seps, one<'('>, seps, args, seps, one<')'>>,
            seq<var, seps, string<'<', '-'>, seps, callee, seps, one<'('>, seps, args, seps, one<')'>>> {};
    struct callee : sor<
            string<'p', 'r', 'i', 'n', 't'>,
            string<'a', 'l', 'l', 'o', 'c', 'a', 't', 'e'>,
            string<'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r'>,
            u> {};
    struct vars : seq<var, star<seq<seps, one<','>, seps, var>>> {};
    struct args : seq<t, star<seq<seps, one<','>, seps, t>>> {};
    struct s : sor<t, label> {};
    struct t : sor<var, N> {};
    struct u : sor<var, label> {};
    struct op : sor<one<'+'>, one<'-'>, one<'*'>, one<'&'>, seq<string<'<', '<'>>, seq<string<'>', '>'>>> {};
    struct cmp : sor<one<'<'>, seq<string<'<', '='>>, one<'='>, seq<string<'>', '='>>, one<'>'>> {};
    struct label : seq<one<':'>, var> {};
    struct var : seq<plus<sor<alpha, one<'_'>>>, star<sor<alpha, one<'_'>, digit>>> {};
    struct N : seq<opt<sor<pegtl::one<'-'>, pegtl::one<'+'>>>, plus<pegtl::digit>> {};
}