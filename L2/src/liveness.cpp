#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <unistd.h>
#include <map>
#include <iostream>

#include "L2.h"

using namespace std;

namespace L2 {
    inline void insert_var(set<string> &ss, string &s, bool flag = true) {
        if (flag || (s != "rsp" && s[0] != ':' && s[0] != '+' && s[0] != '-' && (s[0] < '0' || s[0] > '9'))) {
            ss.insert(s);
        }
    }

    void live_analysis(Function *f, std::vector<set<string>> &gen, vector<set<string>> &kill, vector<set<string>> &in, vector<set<string>> &out) {
        vector<Instruction *> instructions = f->instructions;
        int n = instructions.size();
        for (int idx = 0; idx < n; idx++) {
            set<string> g, k, i, o;
            gen.push_back(g);
            kill.push_back(k);
            in.push_back(i);
            out.push_back(o);
        }
        map <string, int> m;
        for (int i = 0; i < instructions.size(); i++) {
            Instruction *inst = instructions[i];
            switch (inst->operators.front()) {
                case Operator_Type::MOVQ:
                    insert_var(kill[i], inst->operands[0]);
                    if (inst->operators.size() == 1) {
                        insert_var(gen[i], inst->operands[1], false);
                    } else if (inst->operators[1] == Operator_Type::MEM) {
                        insert_var(gen[i], inst->operands[1], false);
                    } else if (inst->operators[1] != Operator_Type::STACK_ARG) {
                        insert_var(gen[i], inst->operands[1], false);
                        insert_var(gen[i], inst->operands[2], false);
                    }
                    break;
                case Operator_Type::ADDQ:
                case Operator_Type::SUBQ:
                case Operator_Type::IMULQ:
                case Operator_Type::ANDQ:
                case Operator_Type::SALQ:
                case Operator_Type::SARQ:
                    insert_var(kill[i], inst->operands[0]);
                    insert_var(gen[i], inst->operands[0]);
                    insert_var(gen[i], inst->operands[1], inst->operators.size() > 1);
                    break;
                case Operator_Type::CJUMP:
                    insert_var(gen[i], inst->operands[0], false);
                    insert_var(gen[i], inst->operands[1], false);
                    break;
                case Operator_Type::LABEL:
                    m[inst->operands[0]] = i;
                case Operator_Type::GOTO:
                    break;
                case Operator_Type::RETURN:
                    gen[i] = {"rax", "r12", "r13", "r14", "r15", "rbp", "rbx"};
                    break;
                case Operator_Type::CALL:
                    kill[i] = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi"};
                    switch ((int)stoll(inst->operands[1])) {
                        case 0:
                            break;
                        case 1:
                            gen[i] = {"rdi"};
                            break;
                        case 2:
                            gen[i] = {"rdi", "rsi"};
                            break;
                        case 3:
                            gen[i] = {"rdi", "rsi", "rdx"};
                            break;
                        case 4:
                            gen[i] = {"rdi", "rsi", "rdx", "rcx"};
                            break;
                        case 5:
                            gen[i] = {"rdi", "rsi", "rdx", "rcx", "r8"};
                            break;
                        default:
                            gen[i] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                            break;
                    }
                    if (inst->operands[0] != "print" && inst->operands[0] != "allocate" && inst->operands[0] != "array-error") {
                        insert_var(gen[i], inst->operands[0], false);
                    }
                    break;
                case Operator_Type::CISC:
                    insert_var(kill[i], inst->operands[0]);
                    insert_var(gen[i], inst->operands[1]);
                    insert_var(gen[i], inst->operands[2]);
                    break;
                case Operator_Type::MEM:
                    insert_var(gen[i], inst->operands[0], false);
                    insert_var(gen[i], inst->operands[2], false);
                    break;
                case Operator_Type::INC:
                case Operator_Type::DEC:
                    insert_var(gen[i], inst->operands[0]);
                    insert_var(kill[i], inst->operands[0]);
                    break;
                default:
                    cerr << "\tERROR ASSEMBLY";
                    break;
            }
        }

        bool flag = true;
        int idx1, idx2;
        while (flag) {
            flag = false;
            for (int i = n - 1; i >= 0; i--) {
                Instruction *inst = instructions[i];
                set<string> out_tmp, in_tmp;
                switch (inst->operators.front()) {
                    case Operator_Type::CJUMP:
                        idx1 = m[inst->operands[2]];
                        idx2 = m[inst->operands[3]];
                        out_tmp = in[idx1];
                        out_tmp.insert(in[idx2].begin(), in[idx2].end());
                        break;
                    case Operator_Type::GOTO:
                        out_tmp = in[m[inst->operands[0]]];
                        break;
                    case Operator_Type::RETURN:
                        break;
                    default:
                        if (i < n - 1) {
                            out_tmp = in[i + 1];
                        }
                        break;
                }
                if (out_tmp != out[i]) {
                    out[i] = out_tmp;
                    flag = true;
                }
                for (auto x : out[i]) {
                    if (kill[i].count(x) == 0) {
                        in_tmp.insert(x);
                    }
                }
                in_tmp.insert(gen[i].begin(), gen[i].end());
                if (in_tmp != in[i]) {
                    in[i] = in_tmp;
                    flag = true;
                }
            }
        }
    }
}