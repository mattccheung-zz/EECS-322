#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

#include "parser.h"

using namespace std;


string get_operand(string operand) {
    if (operand[0] == ':') {
        return "$_" + operand.substr(1);
    } else if (operand[0] == 'r') {
        return "%" + operand;
    } else {
        return "$" + operand;
    }
}

string get_mem_operand(string x, string m) {
    return m + "(%" + x + ")";
}

string get_call_operand(string operand) {
    if (operand[0] == 'r') {
        return "*%" + operand;
    } else {
        return "_" + operand.substr(1);
    }
}

string get_call_shift(string num_of_args) {
    int64_t n = stoll(num_of_args);
    n = n <= 6 ? 8 : (n - 5) * 8;
    return "$" + to_string(n);
}

string get_shift_operand(string operand) {
    return operand[0] == 'r' ? "%cl" : "$" + operand;
}

bool eval_inst(string operand1, string operand2, L1::Operator_Type op) {
    int64_t a = stoll(operand1), b = stoll(operand2);
    switch (op) {
        case L1::Operator_Type::LQ:
            return a < b;
        case L1::Operator_Type::LEQ:
            return a <= b;
        case L1::Operator_Type::EQ:
            return a == b;
        default:
            cout << "error in eval_inst" << endl;
            return false;
    }
}

string get_low_register(string reg) {
    if (reg[1] == '1' || reg[1] == '8' || reg[1] == '9') {
        return "%" + reg + "b";
    } else if (reg[2] == 'x') {
        return "%" + string(1, reg[1]) + "l";
    } else {
        return "%" + reg.substr(1) + "l";
    }
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

    cout << p.entryPointLabel << endl;

    string label, operand, operand2, operand3;

    for (auto f : p.functions) {
        cout << "\t" << f->name << endl << "\t" << f->arguments << "\t" << f->locals << endl;
        for (auto inst : f->instructions) {
            switch (inst->operators.front()) {
                case L1::Operator_Type::MOVQ:
                    operand = get_operand(inst->operands[0]);
                    if (inst->operators.size() == 1) {
                        cout << "\t\tmovq " << get_operand(inst->operands[1]) << ", " << operand;
                    } else if (inst->operators[1] == L1::Operator_Type::MEM) {
                        cout << "\t\tmovq " << get_mem_operand(inst->operands[1], inst->operands[2]) << ", " << operand;
                    } else {
                        operand2 = inst->operands[1];
                        operand3 = inst->operands[2];
                        if (operand2[0] != 'r' && operand3[0] != 'r') {
                            if (eval_inst(operand2, operand3, inst->operators[1])) {
                                cout << "\t\tmovq $1, " << operand;
                            } else {
                                cout << "\t\tmovq $0, " << operand;
                            }
                        } else if (operand2[0] != 'r') {
                            cout << "\t\tcmpq " << get_operand(operand2) << ", " <<  get_operand(operand3) << endl;
                            label = get_low_register(operand3);
                            if (inst->operators[1] == L1::Operator_Type::LQ) {
                                cout << "\t\tsetge " << label << endl;
                            } else if (inst->operators[1] == L1::Operator_Type::LEQ) {
                                cout << "\t\tsetg " << label << endl;
                            } else {
                                cout << "\t\tsete " << label << endl;
                            }
                            cout << "\t\tmovzbq " << label << ", " << operand;
                        } else {
                            cout << "\t\tcmpq " << get_operand(operand3) << ", " <<  get_operand(operand2) << endl;
                            label = get_low_register(operand2);
                            if (inst->operators[1] == L1::Operator_Type::LQ) {
                                cout << "\t\tsetl " << label << endl;
                            } else if (inst->operators[1] == L1::Operator_Type::LEQ) {
                                cout << "\t\tsetle " << label << endl;
                            } else {
                                cout << "\t\tsete " << label << endl;
                            }
                            cout << "\t\tmovzbq " << label << ", " << operand;
                        }
                    }
                    break;
                case L1::Operator_Type::ADDQ:
                    cout << "\t\taddq ";
                    if (inst->operators.size() == 1) {
                        cout << get_operand(inst->operands[1]);
                    } else {
                        cout << get_mem_operand(inst->operands[1], inst->operands[2]);
                    }
                    cout << ", " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::SUBQ:
                    cout << "\t\tsubq ";
                    if (inst->operators.size() == 1) {
                        cout << get_operand(inst->operands[1]);
                    } else {
                        cout << get_mem_operand(inst->operands[1], inst->operands[2]);
                    }
                    cout << ", " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::IMULQ:
                    cout << "\t\timulq " << get_operand(inst->operands[1]) << ", " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::ANDQ:
                    cout << "\t\tandq " << get_operand(inst->operands[1]) << ", " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::SALQ:
                    cout << "\t\tsalq " << get_shift_operand(inst->operands[1]) << ", " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::SARQ:
                    cout << "\t\tsarq " << get_shift_operand(inst->operands[1]) << ", " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::CJUMP:
                    label = get_operand(inst->operands[2]);
                    operand = get_operand(inst->operands[3]);
                    operand2 = inst->operands[0];
                    operand3 = inst->operands[1];
                    if (operand2[0] != 'r' && operand3[0] != 'r') {
                        if (eval_inst(operand2, operand3, inst->operators[1])) {
                            cout << "\t\tjmp " << label;
                        } else {
                            cout << "\t\tjmp " << operand;
                        }
                    } else if (operand2[0] != 'r') {
                        cout << "\t\tcmpq " << operand2 << ", " << operand3 << endl;
                        if (inst->operators[0] == L1::Operator_Type::LEQ) {
                            cout << "\t\tjg ";
                        } else if (inst->operators[0] == L1::Operator_Type::LQ) {
                            cout << "\t\tjge ";
                        } else {
                            cout << "\t\tje ";
                        }
                        cout << label << endl;
                        cout << "\t\tjmp " << operand;
                    } else {
                        cout << "\t\tcmpq " << operand3 << ", " << operand2 << endl;
                        if (inst->operators[0] == L1::Operator_Type::LEQ) {
                            cout << "\t\tjle ";
                        } else if (inst->operators[0] == L1::Operator_Type::LQ) {
                            cout << "\t\tjl ";
                        } else {
                            cout << "\t\tje ";
                        }
                        cout << label << endl;
                        cout << "\t\tjmp " << operand;
                    }
                    break;
                case L1::Operator_Type::LABEL:
                    label = inst->operands.front();
                    label[0] = '_';
                    cout << "\t" << label;
                    break;
                case L1::Operator_Type::GOTO:
                    cout << "\t\tjmp " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::RETURN:
                    cout << "\t\treq";
                    break;
                case L1::Operator_Type::CALL:
                    cout << "\t\tsubq " + get_call_shift(inst->operands[1]) << ", %rsp" << endl;
                    cout << "\t\tjmp " + get_call_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::PRINT:
                    cout << "\t\tcall print";
                    break;
                case L1::Operator_Type::ALLOCATE:
                    cout << "\t\tcall allocate";
                    break;
                case L1::Operator_Type::ARRAY_ERROR:
                    cout << "\t\tcall array-error";
                    break;
                case L1::Operator_Type::CISC:
                    cout << "\t\tlea (" + get_operand(inst->operands[1]) << ", " << get_operand(inst->operands[2])
                            << ", " << inst->operands[3] << "), " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::MEM:
                    operand = get_operand(inst->operands[2]);
                    operand2 = get_mem_operand(inst->operands[0], inst->operands[1]);
                    if (inst->operators[1] == L1::Operator_Type::ADDQ) {
                        cout << "\t\taddq ";
                    } else if (inst->operators[1] == L1::Operator_Type::SUBQ) {
                        cout << "\t\tsubq ";
                    } else {
                        cout << "\t\tmovq ";
                    }
                    cout << operand << ", " << operand2;
                    break;
                case L1::Operator_Type::INC:
                    cout << "\t\tinc " << get_operand(inst->operands[0]);
                    break;
                case L1::Operator_Type::DEC:
                    cout << "\t\tdec " << get_operand(inst->operands[0]);
                    break;
                default:
                    cout << "\t\tERROR ASSEMBLY";
                    break;
            }
            cout << endl;
        }
    }

    return 0;
}
