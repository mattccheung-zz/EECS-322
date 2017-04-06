#pragma once

#include <vector>

namespace L1 {

    struct L1_item {
        std::string labelName;
    };

    struct Function {
        std::string name;
    };

    struct Program {
        std::string entryPointLabel;
        std::vector<L1::Function *> functions;
    };

} // L1
