#include <vector>

#include "ir.h"

using namespace std;


namespace IR {

    BasicBlock::~BasicBlock() {
        for (auto const &inst : instructions) {
            delete inst;
        }
    }

    Function::~Function() {
        for (auto const &arg : arguments) {
            delete arg;
        }
        for (auto const &b : basicBlocks) {
            delete b;
        }
    }

    Program::~Program() {
        for (auto const &f : functions) {
            delete f;
        }
    }
}