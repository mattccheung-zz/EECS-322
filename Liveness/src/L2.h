#pragma once

#include <vector>

namespace L2 {

    enum Operator_Type {
        EMPTY, MOVQ, ADDQ, SUBQ, IMULQ, ANDQ, SALQ, SARQ,
        CJUMP, LABEL, GOTO, RETURN, CALL, CISC, MEM, INC,
        DEC, LQ, EQ, LEQ, STACK_ARG
    };

    struct Instruction {
        std::vector<Operator_Type> operators;
        std::vector<std::string> operands;
    };

    struct Function {
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<Instruction *> instructions;
    };

    struct Program {
        std::string entryPointLabel;
        std::vector<Function *> functions;
    };

} // L2
