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

#include <L1.h>
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/contrib/raw_string.hh>

using namespace pegtl;
using namespace std;

namespace L1 {

    /*
     * Grammar rules from now on.
     */
    struct label :
        pegtl::seq<
            pegtl::one<':'>,
            pegtl::plus<
                pegtl::sor<
                    pegtl::alpha,
                    pegtl::one<'_'>
                >
            >,
            pegtl::star<
                pegtl::sor<
                    pegtl::alpha,
                    pegtl::one<'_'>,
                    pegtl::digit
                >
            >
        > {};

    struct function_name : label {};

    struct number :
        pegtl::seq<
            pegtl::opt<
                pegtl::sor<
                    pegtl::one<'-'>,
                    pegtl::one<'+'>
                >
            >,
            pegtl::plus<
                pegtl::digit
            >
        >{};

    struct argument_number : number {};

    struct local_number : number {};

    struct comment :
        pegtl::disable<
            pegtl::one<';'>,
            pegtl::until<pegtl::eolf>
        > {};

    struct seps :
        pegtl::star<
            pegtl::sor<
                pegtl::ascii::space,
                comment
            >
        > {};

    struct operand_a :
        pegtl::sor<
            pegtl::string<'r', 'd', 'i'>,
            pegtl::string<'r', 's', 'i'>,
            pegtl::string<'r', 'd', 'x'>,
            pegtl::string<'r', 'x', 'i'>,
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

    struct operand_E :
        pegtl::sor<
            pegtl::one<'0'>,
            pegtl::one<'2'>,
            pegtl::one<'4'>,
            pegtl::one<'8'>
        > {};

    struct operand_M : number {};

    struct w : operand_w {};

    struct s : operand_s {};

    struct t : operand_t {};

    struct u : operand_u {};

    struct x : operand_x {};

    struct E : operand_E {};

    struct M : operand_M {};

    struct operator_moveq : pegtl::string<'<', '-'> {};

    struct operator_inc : pegtl::string<'+', '+'> {};

    struct operator_dec : pegtl::string<'-', '-'> {};

    struct operator_at : pegtl::one<'@'> {};

    struct operator_addq : pegtl::string<'+', '='> {};

    struct operator_subq : pegtl::string<'-', '='> {};

    struct operator_imulq : pegtl::string<'*', '='> {};

    struct operator_andq : pegtl::string<'&', '='> {};

    struct operator_aop :
        pegtl::sor<
            operator_addq,
            operator_subq,
            operator_imulq,
            operator_andq
        > {};

    struct operator_salq : pegtl::string<'<', '<', '='> {};

    struct operator_sarq : pegtl::string<'>', '>', '='> {};

    struct operator_sop : pegtl::sor<operator_salq, operator_sarq> {};

    struct operator_lq : pegtl::one<'<'> {};

    struct operator_eq : pegtl::one<'='> {};

    struct operator_leq : pegtl::string<'<', '='> {};

    struct operator_cmp : pegtl::sor<operator_lq, operator_eq, operator_leq> {};

    struct operand_sop : pegtl::sor<number, pegtl::string<'r', 'c', 'x'>> {};

    struct mem : pegtl::string<'m', 'e', 'm'> {};

    struct inst_mem : pegtl::seq<pegtl::one<'('>, seps, mem, seps, x, seps, M, seps, pegtl::one<')'>> {};

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
                                operator_moveq, seps,
                                pegtl::sor<
                                    s, inst_mem, pegtl::seq<t, seps, operator_cmp, t>
                                >
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
                            pegtl::seq<operator_moveq, seps, s>,
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

    struct L1_instruction_rule : instruction {};

//    struct L1_label_rule : label {};

    struct L1_function_rule :
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
                    L1_instruction_rule,
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
            label,
            seps,
            pegtl::plus<
                pegtl::seq<
                    L1_function_rule,
                    seps
                >
            >,
            pegtl::one<')'>,
            seps
        > {};

    struct grammar : pegtl::must<entry_point_rule> {};

    /*
     * Data structures required to parse
     */
    std::vector <L1_item> parsed_registers;

    /*
     * Actions attached to grammar rules.
     */
    template<typename Rule>
    struct action : pegtl::nothing<Rule> {};

    template<>
    struct action<label> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            if (p.entryPointLabel.empty()) {
                p.entryPointLabel = in.string();
            }
        }
    };

    template<>
    struct action<function_name> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Function *newF = new L1::Function();
            newF->name = in.string();
            p.functions.push_back(newF);
        }
    };

//    template<>
//    struct action<L1_label_rule> {
//        static void apply(const pegtl::input &in, L1::Program &p) {
//            L1_item i;
//            i.labelName = in.string();
//            parsed_registers.push_back(i);
//        }
//    };

    template<>
    struct action<argument_number> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Function *currentF = p.functions.back();
            currentF->arguments = std::stoll(in.string());
        }
    };

    template<>
    struct action<local_number> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Function *currentF = p.functions.back();
            currentF->locals = std::stoll(in.string());
        }
    };

    template<>
    struct action<w> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<s> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<t> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<u> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<x> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<E> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<M> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<inst_label> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Function *currentF = p.functions.back();
            L1::Instruction *newI = new L1::Instruction();
            currentF->instructions.push_back(newI);
            newI->operators.push_back(L1::Operator_Type::LABEL);
            newI->operands.push_back(in.string());
        }
    };

    template<>
    struct action<inst_start> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Function *currentF = p.functions.back();
            L1::Instruction *newI = new L1::Instruction();
            currentF->instructions.push_back(newI);
        }
    };

    template<>
    struct action<operator_moveq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::MOVEQ);
        }
    };

    template<>
    struct action<mem> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::MEM);
        }
    };

    template<>
    struct action<operator_addq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::ADDQ);
        }
    };

    template<>
    struct action<operator_subq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::SUBQ);
        }
    };

    template<>
    struct action<operator_imulq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::IMULQ);
        }
    };

    template<>
    struct action<operator_andq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::ANDQ);
        }
    };

    template<>
    struct action<operator_salq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::SALQ);
        }
    };

    template<>
    struct action<operator_lq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::LQ);
        }
    };

    template<>
    struct action<operator_leq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::LEQ);
        }
    };

    template<>
    struct action<operator_eq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::EQ);
        }
    };

    template<>
    struct action<operator_inc> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::INC);
        }
    };

    template<>
    struct action<operator_dec> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::DEC);
        }
    };

    template<>
    struct action<operator_sarq> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::SARQ);
        }
    };

    template<>
    struct action<inst_goto> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::JMP);
        }
    };

    template<>
    struct action<goto_label> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<inst_return> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::RETURN);
        }
    };

    template<>
    struct action<inst_call> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operators.push_back(L1::Operator_Type::JMP);
        }
    };

    template<>
    struct action<inst_call_number> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            p.functions.back()->instructions.back()->operands.push_back(in.string());
        }
    };

    template<>
    struct action<inst_print> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Instruction *currentI = p.functions.back()->instructions.back();
            if (!currentI->operators.empty()) {
                currentI->operators.clear();
            }
            currentI->operators.push_back(L1::Operator_Type::PRINT);
        }
    };

    template<>
    struct action<inst_allocate> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Instruction *currentI = p.functions.back()->instructions.back();
            if (!currentI->operators.empty()) {
                currentI->operators.clear();
            }
            currentI->operators.push_back(L1::Operator_Type::ALLOCATE);
        }
    };

    template<>
    struct action<inst_array_error> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1::Instruction *currentI = p.functions.back()->instructions.back();
            if (!currentI->operators.empty()) {
                currentI->operators.clear();
            }
            currentI->operators.push_back(L1::Operator_Type::ARRAY_ERROR);
        }
    };





    Program L1_parse_file(char *fileName) {

        /*
         * Check the grammar for some possible issues.
         */
        pegtl::analyze<L1::grammar>();

        /*
         * Parse.
         */
        L1::Program p;
        pegtl::file_parser(fileName).parse<L1::grammar, L1::action>(p);

        return p;
    }

} // L1
