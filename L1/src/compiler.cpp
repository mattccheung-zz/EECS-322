#include <string>
#include <vector>
//#include <utility>
//#include <algorithm>
//#include <set>
//#include <iterator>
//#include <iostream>
//#include <cstring>
//#include <cctype>
//#include <cstdlib>
//#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "parser.h"

using namespace std;


string get_label(string &operand) {
    return "_" + operand.substr(1) + ":";
}

string get_opd(string &operand) {
    return operand[0] == ':' ? "$_" + operand.substr(1) :
           (operand[0] == 'r' ? "%" + operand :
            (operand[0] == '+' ? "$" + operand.substr(1) : "$" + operand));
}

string get_mem_opd(string &x, string &m) {
    return m + "(%" + x + ")";
}

string get_call_opd(string &operand) {
    return operand[0] == 'r' ? "*%" + operand : "_" + operand.substr(1);
}

string get_call_shift(string &num_of_args) {
    int64_t n = stoll(num_of_args);
    n = n <= 6 ? 8 : (n - 5) * 8;
    return "$" + to_string(n);
}

string get_shift_opd(string &operand) {
    return operand[0] == 'r' ? "%cl" : "$" + operand;
}

bool eval_inst(string &operand1, string &operand2, L1::Operator_Type &op) {
    int64_t a = stoll(operand1), b = stoll(operand2);
    return op == L1::Operator_Type::LQ ? a < b : (op == L1::Operator_Type::LEQ ? a <= b : a == b);
}

string get_low_reg(string &reg) {
    return reg[1] == '1' || reg[1] == '8' || reg[1] == '9' ? "%" + reg + "b" :
           (reg[2] == 'x' ? "%" + string(1, reg[1]) + "l" : "%" + reg.substr(1) + "l");
}


int main(int argc, char **argv) {
    bool verbose;

    /* Check the input.
     */
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

    /* Parse the L1 program.
     */
    L1::Program p = L1::L1_parse_file(argv[optind]);

    /* Generate x86_64 code
     */

    ofstream output;
    output.open("prog.S");

    output << ".text" << endl
           << "\t.globl go" << endl
           << "go:" << endl
           << "\tpushq %rbx" << endl
           << "\tpushq %rbp" << endl
           << "\tpushq %r12" << endl
           << "\tpushq %r13" << endl
           << "\tpushq %r14" << endl
           << "\tpushq %r15" << endl
           << "\tcall " << get_call_opd(p.entryPointLabel) << endl
           << "\tpopq %r15" << endl
           << "\tpopq %r14" << endl
           << "\tpopq %r13" << endl
           << "\tpopq %r12" << endl
           << "\tpopq %rbp" << endl
           << "\tpopq %rbx" << endl
           << "\tretq" << endl;

    string label, operand, operand2, operand3;

    for (auto f : p.functions) {
        output << get_label(f->name) << endl;
        if (f->locals > 0) {
            output << "\tsubq $" << f->locals * 8 << ", %rsp" << endl;
        }
        for (auto inst : f->instructions) {
            switch (inst->operators.front()) {
                case L1::Operator_Type::MOVQ:
                    operand = get_opd(inst->operands[0]);
                    if (inst->operators.size() == 1) {
                        output << "\tmovq " << get_opd(inst->operands[1]) << ", " << operand;
                    } else if (inst->operators[1] == L1::Operator_Type::MEM) {
                        output << "\tmovq " << get_mem_opd(inst->operands[1], inst->operands[2]) << ", " << operand;
                    } else {
                        operand2 = inst->operands[1];
                        operand3 = inst->operands[2];
                        if (operand2[0] != 'r' && operand3[0] != 'r') {
                            output << (eval_inst(operand2, operand3, inst->operators[1]) ? "\tmovq $1, "
                                                                                         : "\tmovq $0, ")
                                   << operand;
                        } else if (operand2[0] != 'r') {
                            output << "\tcmpq " << get_opd(operand2) << ", " << get_opd(operand3) << endl;
                            label = get_low_reg(inst->operands[0]);
                            if (inst->operators[1] == L1::Operator_Type::LQ) {
                                output << "\tsetg ";
                            } else if (inst->operators[1] == L1::Operator_Type::LEQ) {
                                output << "\tsetge ";
                            } else {
                                output << "\tsete ";
                            }
                            output << label << endl << "\tmovzbq " << label << ", " << operand;
                        } else {
                            output << "\tcmpq " << get_opd(operand3) << ", " << get_opd(operand2) << endl;
                            label = get_low_reg(inst->operands[0]);
                            if (inst->operators[1] == L1::Operator_Type::LQ) {
                                output << "\tsetl ";
                            } else if (inst->operators[1] == L1::Operator_Type::LEQ) {
                                output << "\tsetle ";
                            } else {
                                output << "\tsete ";
                            }
                            output << label << endl << "\tmovzbq " << label << ", " << operand;
                        }
                    }
                    break;
                case L1::Operator_Type::ADDQ:
                    output << "\taddq "
                           << (inst->operators.size() == 1 ? get_opd(inst->operands[1]) : get_mem_opd(inst->operands[1],
                                                                                                      inst->operands[2]))
                           << ", " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::SUBQ:
                    output << "\tsubq "
                           << (inst->operators.size() == 1 ? get_opd(inst->operands[1]) : get_mem_opd(inst->operands[1],
                                                                                                      inst->operands[2]))
                           << ", " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::IMULQ:
                    output << "\timulq " << get_opd(inst->operands[1]) << ", " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::ANDQ:
                    output << "\tandq " << get_opd(inst->operands[1]) << ", " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::SALQ:
                    output << "\tsalq " << get_shift_opd(inst->operands[1]) << ", " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::SARQ:
                    output << "\tsarq " << get_shift_opd(inst->operands[1]) << ", " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::CJUMP:
                    label = get_call_opd(inst->operands[2]);
                    operand = get_call_opd(inst->operands[3]);
                    operand2 = inst->operands[0];
                    operand3 = inst->operands[1];
                    if (operand2[0] != 'r' && operand3[0] != 'r') {
                        if (eval_inst(operand2, operand3, inst->operators[1])) {
                            output << "\tjmp " << label;
                        } else {
                            output << "\tjmp " << operand;
                        }
                    } else if (operand2[0] != 'r') {
                        output << "\tcmpq " << get_opd(operand2) << ", " << get_opd(operand3) << endl;
                        if (inst->operators[1] == L1::Operator_Type::LEQ) {
                            output << "\tjg ";
                        } else if (inst->operators[1] == L1::Operator_Type::LQ) {
                            output << "\tjge ";
                        } else {
                            output << "\tje ";
                        }
                        output << label << endl << "\tjmp " << operand;
                    } else {
                        output << "\tcmpq " << get_opd(operand3) << ", " << get_opd(operand2) << endl;
                        if (inst->operators[1] == L1::Operator_Type::LEQ) {
                            output << "\tjle ";
                        } else if (inst->operators[1] == L1::Operator_Type::LQ) {
                            output << "\tjl ";
                        } else {
                            output << "\tje ";
                        }
                        output << label << endl << "\tjmp " << operand;
                    }
                    break;
                case L1::Operator_Type::LABEL:
                    output << get_label(inst->operands[0]);
                    break;
                case L1::Operator_Type::GOTO:
                    output << "\tjmp " << get_call_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::RETURN:
                    if (f->locals > 0 || f->arguments > 6) {
                        output << "\taddq $" << (f->arguments > 6 ? f->arguments - 6 + f->locals : f->locals) * 8
                               << ", %rsp" << endl;
                    }
                    output << "\tretq";
                    break;
                case L1::Operator_Type::CALL:
                    output << "\tsubq " + get_call_shift(inst->operands[1]) << ", %rsp" << endl;
                    output << "\tjmp " + get_call_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::PRINT:
                    output << "\tcall print";
                    break;
                case L1::Operator_Type::ALLOCATE:
                    output << "\tcall allocate";
                    break;
                case L1::Operator_Type::ARRAY_ERROR:
                    output << "\tcall array_error";
                    break;
                case L1::Operator_Type::CISC:
                    output << "\tlea (" + get_opd(inst->operands[1]) << ", " << get_opd(inst->operands[2])
                           << ", " << inst->operands[3] << "), " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::MEM:
                    operand = get_opd(inst->operands[2]);
                    operand2 = get_mem_opd(inst->operands[0], inst->operands[1]);
                    if (inst->operators[1] == L1::Operator_Type::ADDQ) {
                        output << "\taddq ";
                    } else if (inst->operators[1] == L1::Operator_Type::SUBQ) {
                        output << "\tsubq ";
                    } else {
                        output << "\tmovq ";
                    }
                    output << operand << ", " << operand2;
                    break;
                case L1::Operator_Type::INC:
                    output << "\tinc " << get_opd(inst->operands[0]);
                    break;
                case L1::Operator_Type::DEC:
                    output << "\tdec " << get_opd(inst->operands[0]);
                    break;
                default:
                    output << "\tERROR ASSEMBLY";
                    break;
            }
            output << endl;
        }
    }

    output << endl;

    output.close();

    return 0;
}
