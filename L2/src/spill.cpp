#include <map>
#include <set>
#include <string>
#include <iostream>

#include "L2.h"

using namespace std;

namespace L2 {
    void replace(const Function *f, const map<string, string> &reg_map) {
        for (auto const &inst : f->instructions) {
            for (int i = 0; i < inst->operands.size(); i++) {
                if (reg_map.find(inst->operands[i]) != reg_map.end()) {
                    inst->operands[i] = reg_map.at(inst->operands[i]);
                }
            }
        }
    }

    void transform_instruction(Function *func, const Instruction *inst, const string &sp, int64_t &index) {
        bool flag = false;
        Instruction *nInst = new Instruction;
        for (auto const &op : inst->operands) {
            if (op == sp) {
                flag = true;
                break;
            }
        }
        if (flag) {
            string nv = sp + to_string(++index);
            Instruction *preInst = new Instruction, *postInst = new Instruction, *midInst = new Instruction;
            preInst->operators = {Operator_Type::MOVQ, Operator_Type::MEM};
            preInst->operands = {nv, "rsp", "0"};
            postInst->operators = {Operator_Type::MEM, Operator_Type::MOVQ};
            postInst->operands = {"rsp", "0", nv};
            midInst->operators = inst->operators;
            midInst->operands = inst->operands;
            for (int i = 0; i < midInst->operands.size(); i++) {
                if (midInst->operands[i] == sp) {
                    midInst->operands[i] = nv;
                }
            }
            func->instructions.push_back(preInst);
            func->instructions.push_back(midInst);
            func->instructions.push_back(postInst);
        } else {
            nInst->operands = inst->operands;
            nInst->operators = inst->operators;
            func->instructions.push_back(nInst);
        }
    }

    Function *spill(const Function *f, const string &sp) {
        int64_t index = 0;
        Function *func = new Function;
        func->name = f->name;
        func->arguments = f->arguments;
        func->locals = f->locals + 1;
        for (auto const &inst : f->instructions) {
            if (inst->operators[0] == Operator_Type::MEM && inst->operands[0] == "rsp") {
                inst->operands[1] = to_string(stoll(inst->operands[1]) + 8);
            } else if (inst->operators.size() == 2 && inst->operators[1] == Operator_Type::MEM && inst->operands[1] == "rsp") {
                inst->operands[2] = to_string(stoll(inst->operands[2]) + 8);
            }
        }
        for (auto const &inst : f->instructions) {
            transform_instruction(func, inst, sp, index);
        }
        return func;
    }

    Function *replace_and_spill(Function *f, const map<string, string> &reg_map, const set<string> &spill_set) {
        replace(f, reg_map);
        for (auto const &sp : spill_set) {
            f = spill(f, sp);
        }
        return f;
    }

}