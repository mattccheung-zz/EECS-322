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
        int matched_num = 0;
        Instruction *nInst = new Instruction;
        for (auto const &op : inst->operands) {
            if (op == sp) {
                matched_num++;
            }
        }
        if (matched_num > 0) {
            if (matched_num == 1 && inst->operators.size() == 1 && (inst->operators[0] == Operator_Type::MOVQ ||
                    inst->operators[0] == Operator_Type::ADDQ|| inst->operators[0] == Operator_Type::SUBQ)) {
                if (inst->operands[0] == sp) {
                    nInst->operators = {Operator_Type::MEM, inst->operators[0]};
                    nInst->operands = {"rsp", "0", inst->operands[1]};
                } else {
                    nInst->operators = {inst->operators[0], Operator_Type::MEM};
                    nInst->operands = {inst->operands[0], "rsp", "0"};
                }
                func->instructions.push_back(nInst);
            } else {
                string nv = sp + "_nv_" + to_string(++index);
                if (!(matched_num == 1 && inst->operators[0] == Operator_Type::MOVQ && inst->operands[0] == sp)) {
                    Instruction *preInst = new Instruction;
                    preInst->operators = {Operator_Type::MOVQ, Operator_Type::MEM};
                    preInst->operands = {nv, "rsp", "0"};
                    func->instructions.push_back(preInst);
                }
                Instruction *midInst = new Instruction;
                midInst->operators = inst->operators;
                midInst->operands = inst->operands;
                for (int i = 0; i < midInst->operands.size(); i++) {
                    if (midInst->operands[i] == sp) {
                        midInst->operands[i] = nv;
                    }
                }
                func->instructions.push_back(midInst);
                if (inst->operators[0] != Operator_Type::CJUMP && (inst->operands[0] == sp && inst->operators[0] != Operator_Type::MEM)) {
                    Instruction *postInst = new Instruction;
                    postInst->operators = {Operator_Type::MEM, Operator_Type::MOVQ};
                    postInst->operands = {"rsp", "0", nv};
                    func->instructions.push_back(postInst);
                }
            }
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
                if (stoll(inst->operands[1]) >= 0) {
                    inst->operands[1] = to_string(stoll(inst->operands[1]) + 8);
                }
            } else if (inst->operators.size() == 2 && inst->operators[1] == Operator_Type::MEM && inst->operands[1] == "rsp") {
                if (stoll(inst->operands[2]) >= 0) {
                    inst->operands[2] = to_string(stoll(inst->operands[2]) + 8);
                }
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