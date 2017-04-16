#include <string>
#include <vector>
//#include <utility>
#include <algorithm>
#include <set>
//#include <iterator>
//#include <iostream>
//#include <cstring>
//#include <cctype>
//#include <cstdlib>
//#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <map>

#include "parser.h"

using namespace std;
using namespace L2;


ostream &operator<<(ostream &os, const set <string> &s) {
    if (s.empty()) {
        os << "()";
    } else {
        vector <string> v(s.begin(), s.end());
        sort(v.begin(), v.end());
        os << '(';
        for (int i = 0; i < v.size() - 1; i++) {
            os << v[i] << ' ';
        }
        os << v[v.size() - 1] << ')';
    }
    return os;
}

void print_result(set <string> in[], set <string> out[], int n) {
    cout << '(' << endl << "(in" << endl;
    for (int i = 0; i < n; i++) {
        cout << in[i] << endl;
    }
    cout << ')' << endl << endl << "(out" << endl;
    for (int i = 0; i < n; i++) {
        cout << out[i] << endl;
    }
    cout << ')' << endl;
}

inline void insert_var(set<string> &ss, string &s, bool flag = true) {
    if (flag || (s[0] != ':' && s[0] != '+' && s[0] != '-' && (s[0] < '0' || s[0] > '9'))) {
        ss.insert(s);
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

    Program p = L2_parse_function(argv[optind]);

    vector<Instruction *> instructions = p.functions.front()->instructions;
    int n = instructions.size();

    set <string> gen[n], kill[n], in[n], out[n];
    map <string, int> m;

    for (int i = 0; i < instructions.size(); i++) {
        Instruction *inst = instructions[i];
        switch (inst->operators.front()) {
            case Operator_Type::MOVQ:
                insert_var(kill[i], inst->operands[0]);
                if (inst->operators.size() == 1) {
                    insert_var(gen[i], inst->operands[1], false);
                } else if (inst->operators[1] == Operator_Type::MEM) {
                    insert_var(gen[i], inst->operands[1]);
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
            case Operator_Type::PRINT:
            case Operator_Type::ALLOCATE:
            case Operator_Type::ARRAY_ERROR:
            case Operator_Type::CALL:
                kill[i] = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi"};
                switch (stoll(inst->operands[1])) {
                    case 0:
                        break;
                    case 1:
                        gen[i] = {"rdi"};
                    case 2:
                        gen[i] = {"rdi", "rsi"};
                    case 3:
                        gen[i] = {"rdi", "rsi", "rdx"};
                    case 4:
                        gen[i] = {"rdi", "rsi", "rdx", "rcx"};
                    case 5:
                        gen[i] = {"rdi", "rsi", "rdx", "rcx", "r8"};
                    default:
                        gen[i] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                        break;
                }
                if (inst->operators.front() == Operator_Type::CALL) {
                    insert_var(gen[i], inst->operands[0], false);
                }
                break;
            case Operator_Type::CISC:
                insert_var(kill[i], inst->operands[0]);
                insert_var(gen[i], inst->operands[1]);
                insert_var(gen[i], inst->operands[2]);
                break;
            case Operator_Type::MEM:
                insert_var(gen[i], inst->operands[0]);
                insert_var(gen[i], inst->operands[2], false);
                break;
            case Operator_Type::INC:
            case Operator_Type::DEC:
                insert_var(kill[i], inst->operands[0]);
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
                    out_tmp = in[i + 1];
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

    print_result(in, out, n);

    return 0;
}
