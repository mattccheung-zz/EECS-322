#pragma once

#include <vector>
#include <string>

namespace L3 {

    struct Instruction {
    };

    struct Function {
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<Instruction> instructions;
    };

    struct Program {
        std::string entryPoint;
        std::vector<Function> functions;
    };

}
