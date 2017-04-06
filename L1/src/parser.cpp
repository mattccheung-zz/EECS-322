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
                    pegtl::one <'_'>,
                    pegtl::digit
                >
            >
        > {};

    struct function_name : label {};

    struct seps : pegtl::star<pegtl::ascii::space> {};

    struct L1_label_rule : label {};

    struct L1_function_rule :
        pegtl::seq<
            pegtl::one <'('>,
            function_name,
            seps,
            pegtl::one<')'>,
            seps
        > {};

    struct L1_functions_rule : pegtl::plus<L1_function_rule> {};

    struct entry_point_rule :
            pegtl::seq<
                pegtl::one <'('>,
                label,
                seps,
                L1_functions_rule,
                seps,
                pegtl::one<')'>
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

    template<>
    struct action<L1_label_rule> {
        static void apply(const pegtl::input &in, L1::Program &p) {
            L1_item i;
            i.labelName = in.string();
            parsed_registers.push_back(i);
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
