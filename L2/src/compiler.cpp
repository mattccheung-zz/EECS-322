#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <map>

#include "parser.h"
#include "interference.h"
#include "coloring.h"
#include "spill.h"

using namespace std;
using namespace L2;


ostream &operator<<(ostream &os, const Instruction &inst) {
    string op;
    switch (inst.operators.front()) {
        case Operator_Type::MOVQ:
            if (inst.operators.size() == 1) {
                os << '(' << inst.operands[0] << " <- " << inst.operands[1] << ')';
            } else if (inst.operators[1] == Operator_Type::MEM) {
                os << '(' << inst.operands[0] << " <- (mem " << inst.operands[1] << ' ' << inst.operands[2] << "))";
            } else if (inst.operators[1] == Operator_Type::STACK_ARG) {
                os << '(' << inst.operands[0] << " <- (stack-arg " << inst.operands[1] << "))";
            } else {
                op = inst.operators[1] == Operator_Type::LQ ? " < " :
                     (inst.operators[1] == Operator_Type::LEQ ? " <= " : " = ");
                os << '(' << inst.operands[0] << " <- " << inst.operands[1] << op << inst.operands[2] << ")";
            }
            break;
        case Operator_Type::ADDQ:
        case Operator_Type::SUBQ:
            op = inst.operators[0] == Operator_Type::ADDQ ? " += " : " -= ";
            if (inst.operators.size() == 1) {
                os << '(' << inst.operands[0] << op << inst.operands[1] << ')';
            } else {
                os << '(' << inst.operands[0] << op << "(mem " << inst.operands[1] << ' ' << inst.operands[2] << "))";
            }
            break;
        case Operator_Type::IMULQ:
        case Operator_Type::ANDQ:
        case Operator_Type::SALQ:
        case Operator_Type::SARQ:
            op = inst.operators[0] == Operator_Type::IMULQ ? " *= " :
                 (inst.operators[0] == Operator_Type::ANDQ ? " &= " :
                  (inst.operators[0] == Operator_Type::SALQ ? " <<= " : " >>= "));
            os << '(' << inst.operands[0] << op << inst.operands[1] << ')';
            break;
        case Operator_Type::CJUMP:
            op = inst.operators[1] == Operator_Type::LQ ? " < " :
                 (inst.operators[1] == Operator_Type::LEQ ? " <= " : " = ");
            os << "(cjump " << inst.operands[0] << op << inst.operands[1] << ' ' << inst.operands[2] << ' ' << inst.operands[3] << ')';
            break;
        case Operator_Type::LABEL:
            os << inst.operands[0];
            break;
        case Operator_Type::GOTO:
            os << "(goto " << inst.operands[0] << ')';
            break;
        case Operator_Type::RETURN:
            os << "(return)";
            break;
        case Operator_Type::CALL:
            os << "(call " << inst.operands[0] << ' ' << inst.operands[1] << ')';
            break;
        case Operator_Type::CISC:
            os << '(' << inst.operands[0] << " @ " << inst.operands[1] << ' ' << inst.operands[2] << ' ' << inst.operands[3] << ')';
            break;
        case Operator_Type::MEM:
            op = inst.operators[1] == Operator_Type::MOVQ ? " <- " :
                 (inst.operators[1] == Operator_Type::ADDQ ? " += " : " -= ");
            os << "((mem " << inst.operands[0] << ' ' << inst.operands[1] << ')' << op << inst.operands[2] << ')';
            break;
        case Operator_Type::INC:
        case Operator_Type::DEC:
            op = inst.operators[0] == Operator_Type::INC ? "++" : "--";
            os << "(" << inst.operands[0] << op << ')';
            break;
        default:
            cerr << "\tERROR L2" << endl;
            break;
    }
    return os;
}

ostream &operator<<(ostream &os, const Program &p) {
    os << '(' << p.entryPointLabel << endl;
    for (auto const &f : p.functions) {
        os << "    (" << f->name << endl
           << "    " << f->arguments << ' ' << f->locals << endl;
        for (auto const &inst : f->instructions) {
            os << "        " << *inst << endl;
        }
        os << "    )" << endl;
    }
    os << ')' << endl;
    return os;
}



void remove_stack_arg(Program &p) {
    int64_t locals;
    for (auto const &f : p.functions) {
        locals = f->locals;
        for (auto const &inst : f->instructions) {
            if (inst->operators.size() == 2 && inst->operators[2] == Operator_Type::STACK_ARG) {
                inst->operators[1] = Operator_Type::MEM;
                inst->operands[1] = to_string(stoll(inst->operands[1]) + locals * 8);
            }
        }
    }
}



int main(int argc, char **argv) {
    bool verbose;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " SOURCE [-v]" << std::endl;
        return 1;
    }
    int32_t opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << "[-v] SOURCE" << std::endl;
                return 1;
        }
    }

    ofstream output;
    output.open("prog.L1");
    Program p = L2_parse_file(argv[optind]);

    for (int i = 0; i < p.functions.size(); i++) {
        map<string, string> reg_map;
        set<string> spill_set;
        do {
            reg_map.clear();
            spill_set.clear();
            map<string, set<string>> graph = compute_interference_graph(p.functions[i]);
            graph_coloring(graph, reg_map, spill_set);
            p.functions[i] = replace_and_spill(p.functions[i], reg_map, spill_set);
        } while (!spill_set.empty());
    }

    output << p << endl;

    output.close();
    return 0;
}
