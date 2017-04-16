#pragma once

#include <vector>

namespace L2 {

    enum Operator_Type {
        EMPTY, MOVQ, ADDQ, SUBQ, IMULQ, ANDQ, SALQ, SARQ,
        CJUMP, LABEL, GOTO, RETURN, CALL, PRINT, ALLOCATE,
        ARRAY_ERROR, CISC, MEM, INC, DEC, LQ, EQ, LEQ
    };

    struct L2_item {
        std::string labelName;
    };

    struct Instruction {
        std::vector<L2::Operator_Type> operators;
        std::vector<std::string> operands;
    };

    struct Function {
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<L2::Instruction *> instructions;
    };

    struct Program {
        std::string entryPointLabel;
        std::vector<L2::Function *> functions;
    };

} // L2
