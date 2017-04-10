#pragma once

#include <vector>

namespace L1 {

    enum Operator_Type {
        EMPTY, MOVQ, ADDQ, SUBQ, IMULQ, ANDQ, SALQ, SARQ,
        CJUMP, LABEL, GOTO, RETURN, CALL, PRINT, ALLOCATE,
        ARRAY_ERROR, CISC, MEM, INC, DEC, LQ, EQ, LEQ
    };

    struct L1_item {
        std::string labelName;
    };

    struct Instruction {
        std::vector<L1::Operator_Type> operators;
        std::vector<std::string> operands;
    };

    struct Function {
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<L1::Instruction *> instructions;
    };

    struct Program {
        std::string entryPointLabel;
        std::vector<L1::Function *> functions;
    };

} // L1
