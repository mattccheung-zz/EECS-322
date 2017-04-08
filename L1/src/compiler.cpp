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

    const char* MAPS[] {
            "EMPTY", "MOVQ", "ADDQ", "SUBQ", "IMULQ", "ANDQ", "SALQ", "SARQ",
            "CJUMP", "LABEL", "JMP", "RETURN", "CALL", "PRINT", "ALLOCATE",
            "ARRAY_ERROR", "CISC", "MEM", "INC", "DEC", "LQ", "EQ", "LEQ"
    };

    cout << p.entryPointLabel << endl;

    string label, operand, operand2, operand3;

    for (auto f : p.functions) {
        cout << "\t" << f->name << endl << "\t" << f->arguments << "\t" << f->locals << endl;
        for (auto inst : f->instructions) {
            switch (inst->operators.front()) {
                case L1::Operator_Type::MOVQ:
                    if (inst->operators.size() == 1) {
                        cout << "\t\tmovq " << get_operand(inst->operands[1]) << " %" << inst->operands[0];
                    } else {
                    }
                    break;
                case L1::Operator_Type::ADDQ:
                    cout << "\t\tADDQ";
                    break;
                case L1::Operator_Type::SUBQ:
                    cout << "\t\tSUBQ";
                    break;
                case L1::Operator_Type::IMULQ:
                    cout << "\t\tIMULQ";
                    break;
                case L1::Operator_Type::ANDQ:
                    cout << "\t\tANDQ";
                    break;
                case L1::Operator_Type::SALQ:
                    cout << "\t\tSALQ";
                    break;
                case L1::Operator_Type::SARQ:
                    cout << "\t\tSARQ";
                    break;
                case L1::Operator_Type::CJUMP:
                    cout << "\t\tcmpq " << get_operand(inst->operands[1]) << " " << get_operand(inst->operands[0]) << endl;
                    if (inst->operators[0] == L1::Operator_Type::LEQ) {
                        cout << "\t\tjge ";
                    } else if (inst->operators[0] == L1::Operator_Type::LQ) {
                        cout << "\t\tjg ";
                    } else {
                        cout << "\t\tje ";
                    }
                    cout << get_operand(inst->operands[2]) << endl;
                    cout << "\t\tjmp " << get_operand(inst->operands[3]);
                    break;
                case L1::Operator_Type::LABEL:
                    label = inst->operands.front();
                    label[0] = '_';
                    cout << "\t" << label;
                    break;
                case L1::Operator_Type::JMP:
                    cout << "\t\tJMP";
                    break;
                case L1::Operator_Type::RETURN:
                    cout << "\t\treq";
                    break;
                case L1::Operator_Type::CALL:
                    cout << "\t\tCALL";
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
                    cout << "\t\tCISC";
                    break;
                case L1::Operator_Type::MEM:
                    cout << "\t\tMEM";
                    break;
                case L1::Operator_Type::INC:
                    cout << "\t\tINC";
                    break;
                case L1::Operator_Type::DEC:
                    cout << "\t\tDEC";
                    break;
                case L1::Operator_Type::LQ:
                    cout << "\t\tLQ";
                    break;
                case L1::Operator_Type::EQ:
                    cout << "\t\tEQ";
                    break;
                case L1::Operator_Type::LEQ:
                    cout << "\t\tLEQ";
                    break;
                default:
                    cout << "\t\tERROR";
                    break;
            }
            cout << endl;
        }
    }

    return 0;
}
