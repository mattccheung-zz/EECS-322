#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include "L2.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/contrib/raw_string.hh>

using namespace pegtl;
using namespace std;

namespace L2 {

    struct variable :
        pegtl::seq<
            pegtl::plus<pegtl::sor<pegtl::alpha, pegtl::one<'_'>>>,
            pegtl::star<pegtl::sor<pegtl::alpha, pegtl::one<'_'>, pegtl::digit>>
        > {};

    struct label : pegtl::seq<pegtl::one<':'>, variable> {};

    struct entry_point_label : label {};

    struct function_name : label {};

    struct number :
        pegtl::seq<
            pegtl::opt<pegtl::sor<pegtl::one<'-'>, pegtl::one<'+'>>>,
            pegtl::plus<pegtl::digit>
        >{};

    struct argument_number : number {};

    struct local_number : number {};

    struct comment : pegtl::disable<pegtl::one<';'>, pegtl::until<pegtl::eolf>> {};

    struct seps : pegtl::star<pegtl::sor<pegtl::ascii::space, comment>> {};

    struct operand_sx : pegtl::sor<pegtl::string<'r', 'c', 'x'>, variable> {};

    struct operand_a :
        pegtl::sor<
            pegtl::string<'r', 'd', 'i'>,
            pegtl::string<'r', 's', 'i'>,
            pegtl::string<'r', 'd', 'x'>,
            operand_sx,
            pegtl::string<'r', '8'>,
            pegtl::string<'r', '9'>
        > {};

    struct operand_w :
        pegtl::sor<
            operand_a,
            pegtl::string<'r', 'a', 'x'>,
            pegtl::string<'r', 'b', 'x'>,
            pegtl::string<'r', 'b', 'p'>,
            pegtl::string<'r', '1', '0'>,
            pegtl::string<'r', '1', '1'>,
            pegtl::string<'r', '1', '2'>,
            pegtl::string<'r', '1', '3'>,
            pegtl::string<'r', '1', '4'>,
            pegtl::string<'r', '1', '5'>
        > {};

    struct operand_x : pegtl::sor<operand_w, pegtl::string<'r', 's', 'p'>> {};

    struct operand_u : pegtl::sor<operand_w, label> {};

    struct operand_t : pegtl::sor<operand_x, number> {};

    struct operand_s : pegtl::sor<operand_t, label> {};

    struct operand_E : pegtl::sor<pegtl::one<'0'>, pegtl::one<'2'>, pegtl::one<'4'>, pegtl::one<'8'>> {};

    struct operand_M : number {};

    struct w : operand_w {};

    struct s : operand_s {};

    struct t : operand_t {};

    struct u : operand_u {};

    struct x : operand_x {};

    struct E : operand_E {};

    struct M : operand_M {};

    struct operator_movq : pegtl::string<'<', '-'> {};

    struct operator_inc : pegtl::string<'+', '+'> {};

    struct operator_dec : pegtl::string<'-', '-'> {};

    struct operator_at : pegtl::one<'@'> {};

    struct operator_addq : pegtl::string<'+', '='> {};

    struct operator_subq : pegtl::string<'-', '='> {};

    struct operator_imulq : pegtl::string<'*', '='> {};

    struct operator_andq : pegtl::string<'&', '='> {};

//    struct operator_aop : pegtl::sor<operator_addq, operator_subq, operator_imulq, operator_andq> {};

    struct operator_salq : pegtl::string<'<', '<', '='> {};

    struct operator_sarq : pegtl::string<'>', '>', '='> {};

    struct operator_sop : pegtl::sor<operator_salq, operator_sarq> {};

    struct operator_lq : pegtl::one<'<'> {};

    struct operator_eq : pegtl::one<'='> {};

    struct operator_leq : pegtl::string<'<', '='> {};

    struct operator_cmp : pegtl::sor<operator_leq, operator_lq, operator_eq> {};

    struct operand_sop : pegtl::sor<number, operand_sx> {};

    struct mem : pegtl::string<'m', 'e', 'm'> {};

    struct inst_mem : pegtl::seq<pegtl::one<'('>, seps, mem, seps, x, seps, M, seps, pegtl::one<')'>> {};

    struct stack_arg : pegtl::string<'s', 't', 'a', 'c', 'k', '-', 'a', 'r', 'g'> {};

    struct inst_stack_arg : pegtl::seq<pegtl::one<'('>, seps, stack_arg, seps, M, seps, pegtl::one<')'>> {};

    struct inst_cjump : pegtl::string<'c', 'j', 'u', 'm', 'p'> {};

    struct inst_return : pegtl::string<'r', 'e', 't', 'u', 'r', 'n'> {};

    struct inst_goto : pegtl::string<'g', 'o', 't', 'o'> {};

    struct inst_call : pegtl::string<'c', 'a', 'l', 'l'> {};

    struct inst_print : pegtl::string<'p', 'r', 'i', 'n', 't'> {};

    struct inst_allocate : pegtl::string<'a', 'l', 'l', 'o', 'c', 'a', 't', 'e'> {};

    struct inst_array_error : pegtl::string<'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r'> {};

    struct inst_cjump_label : label{};

    struct inst_call_number : number {};

    struct inst_label : label {};

    struct inst_start : pegtl::one<'('> {};

    struct goto_label : label {};

    struct instruction :
        pegtl::sor<
            inst_label,
            pegtl::seq<
                inst_start,
                seps,
                pegtl::sor<
                    pegtl::seq<
                        w, seps,
                        pegtl::sor<
                            operator_inc, operator_dec,
                            pegtl::seq<
                                operator_movq, seps,
                                pegtl::sor<pegtl::seq<t, seps, operator_cmp, seps, t>, s, inst_mem, inst_stack_arg>
                            >,
                            pegtl::seq<operator_at, seps, w, seps, w, seps, E>,
                            pegtl::seq<operator_sop, seps, operand_sop>,
                            pegtl::seq<pegtl::sor<operator_addq, operator_subq>, seps, pegtl::sor<t, inst_mem>>,
                            pegtl::seq<pegtl::sor<operator_imulq, operator_andq>, seps, t>
                        >
                    >,
                    pegtl::seq<
                        inst_mem, seps,
                        pegtl::sor<
                            pegtl::seq<operator_movq, seps, s>,
                            pegtl::seq<pegtl::sor<operator_addq, operator_subq>, seps, t>
                        >
                    >,
                    pegtl::seq<inst_cjump, seps, t, seps, operator_cmp, seps, t, seps, inst_cjump_label, seps, inst_cjump_label>,
                    pegtl::seq<inst_goto, seps, goto_label>,
                    inst_return,
                    pegtl::seq<
                        inst_call, seps,
                        pegtl::sor<
                            pegtl::seq<u, seps, inst_call_number>,
                            pegtl::seq<inst_print, seps, pegtl::one<'1'>>,
                            pegtl::seq<inst_allocate, seps, pegtl::one<'2'>>,
                            pegtl::seq<inst_array_error, seps, pegtl::one<'2'>>
                        >
                    >
                >,
                seps,
                pegtl::one<')'>
            >
        > {};

    struct L2_instruction_rule : instruction {};

    struct L2_function_rule :
        pegtl::seq<
            pegtl::one<'('>,
            seps,
            function_name,
            seps,
            argument_number,
            seps,
            local_number,
            seps,
            pegtl::star<
                pegtl::seq<
                    L2_instruction_rule,
                    seps
                >
            >,
            pegtl::one<')'>
        > {};

    struct entry_point_rule :
        pegtl::seq<
            seps,
            pegtl::one<'('>,
            seps,
            entry_point_label,
            seps,
            pegtl::plus<
                pegtl::seq<
                    L2_function_rule,
                    seps
                >
            >,
            pegtl::one<')'>,
            seps
        > {};





    template<typename Rule>
    struct action : pegtl::nothing<Rule> {};

    template<>
    struct action<entry_point_label> {
        static void apply(const pegtl::input &in, Program &p) {
            p.entryPointLabel = in.string();
        }
    };

    template<>
    struct action<function_name> {
        static void apply(const pegtl::input &in, Program &p) {
            Function *newF = new Function();
            newF->name = in.string();
            p.functions.push_back(newF);
        }
    };

    template<>
    struct action<argument_number> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->arguments = std::stoll(in.string());
        }
    };

    template<>
    struct action<local_number> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->locals = std::stoll(in.string());
        }
    };

    template<>
    struct action<w> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<s> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<t> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<u> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<x> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<E> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<M> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<operand_sop> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<inst_label> {
        static void apply(const pegtl::input &in, Program &p) {
            Instruction *newI = new Instruction();
            newI->operators.push_back(Operator_Type::LABEL);
            newI->operands.push_back(in.string());
            p.functions.back()->instructions.push_back(newI);
        }
    };

    template<>
    struct action<inst_start> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.push_back(new Instruction());
        }
    };

    template<>
    struct action<operator_movq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::MOVQ);
        }
    };

    template<>
    struct action<mem> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::MEM);
        }
    };

    template<>
    struct action<stack_arg> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::STACK_ARG);
        }
    };

    template<>
    struct action<operator_addq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::ADDQ);
        }
    };

    template<>
    struct action<operator_subq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::SUBQ);
        }
    };

    template<>
    struct action<operator_imulq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::IMULQ);
        }
    };

    template<>
    struct action<operator_andq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::ANDQ);
        }
    };

    template<>
    struct action<operator_salq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::SALQ);
        }
    };

    template<>
    struct action<operator_lq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::LQ);
        }
    };

    template<>
    struct action<operator_leq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::LEQ);
        }
    };

    template<>
    struct action<operator_eq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::EQ);
        }
    };

    template<>
    struct action<operator_inc> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::INC);
        }
    };

    template<>
    struct action<operator_dec> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::DEC);
        }
    };

    template<>
    struct action<operator_sarq> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::SARQ);
        }
    };

    template<>
    struct action<operator_at> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::CISC);
        }
    };

    template<>
    struct action<inst_goto> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::GOTO);
        }
    };

    template<>
    struct action<goto_label> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<inst_return> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::RETURN);
        }
    };

    template<>
    struct action<inst_call> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::CALL);
        }
    };

    template<>
    struct action<inst_call_number> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<inst_print> {
        static void apply(const pegtl::input &in, Program &p) {
            vector<Operator_Type> operators = p.functions.back()->instructions.back()->operators;
            operators.clear();
            operators.push_back(Operator_Type::PRINT);
        }
    };

    template<>
    struct action<inst_allocate> {
        static void apply(const pegtl::input &in, Program &p) {
            vector<Operator_Type> operators = p.functions.back()->instructions.back()->operators;
            operators.clear();
            operators.push_back(Operator_Type::ALLOCATE);
        }
    };

    template<>
    struct action<inst_array_error> {
        static void apply(const pegtl::input &in, Program &p) {
            vector<Operator_Type> operators = p.functions.back()->instructions.back()->operators;
            operators.clear();
            operators.push_back(Operator_Type::ARRAY_ERROR);
        }
    };

    template<>
    struct action<inst_cjump> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(Operator_Type::CJUMP);
        }
    };

    template<>
    struct action<inst_cjump_label> {
        static void apply(const pegtl::input &in, Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };


    Program L2_parse_file(char *fileName) {
        struct grammar : pegtl::must<entry_point_rule> {};
        pegtl::analyze<grammar>();
        Program p;
        pegtl::file_parser(fileName).parse<grammar, action>(p);
        return p;
    }


    struct function_entry_point_rule : pegtl::seq<seps, L2_function_rule, seps> {};

    Program L2_parse_function(char *file) {
        struct function_grammar : pegtl::must<function_entry_point_rule> {};
        pegtl::analyze<function_grammar>();
        Program p;
        pegtl::file_parser(file).parse<function_grammar, action>(p);
        return p;
    }

} // L2
