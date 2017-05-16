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

    Program IRParseFile(char *file) {
//        struct grammar : must<p> {};
//        analyze<grammar>();
        Program p;
//        file_parser(file).parse<grammar, action>(p);
        return p;
    }
}