#include <map>
#include <set>
#include <string>
#include <vector>

#include "L2.h"
#include "liveness.h"

using namespace std;

namespace L2 {
    set<string> registers = {"rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10",
                             "r11", "r12", "r13", "r14", "r15", "rbp", "rbx"};

    inline bool isNumber(string operand) {
        return operand[0] == '+' || operand[0] == '-' || (operand[0] >= '0' && operand[0] <= '9');
    }

    void add_into_graph(map<string, set<string>> &graph, const string &key, const string &value) {
        if (key != value) {
            if (graph.find(key) == graph.end()) {
                set<string> s;
                graph[key] = s;
            }
            if (graph.find(value) == graph.end()) {
                set<string> s;
                graph[value] = s;
            }
            graph[key].insert(value);
            graph[value].insert(key);
        }
    }

    map<string, set<string>> compute_interference_graph(Function *f) {
        vector<set<string>> gen, kill, in, out;
        live_analysis(f, gen, kill, in, out);

        map<string, set<string>> graph;
        for (set<string>::iterator itr = registers.begin(); itr != registers.end(); itr++) {
            set<string> s = registers;
            s.erase(*itr);
            graph[*itr] = s;
        }

        for (int i = 0; i < in.size(); i++) {
            vector<string> v(in[i].begin(), in[i].end());
            for (int j = 0; j < v.size() - 1; j++) {
                for (int k = j + 1; k < v.size(); k++) {
                    add_into_graph(graph, v[j], v[k]);
                }
            }
        }

        for (int i = 0; i < kill.size(); i++) {
            Instruction *instruction = f->instructions[i];
            for (set<string>::iterator itk = kill[i].begin(); itk != kill[i].end(); itk++) {
                for (set<string>::iterator ito = out[i].begin(); ito != out[i].end(); ito++) {
                    if (instruction->operators.size() == 1 && instruction->operators.front() == Operator_Type::MOVQ &&
                            *itk == instruction->operands[0] && *ito == instruction->operands[1]) {
                        continue;
                    } else {
                        add_into_graph(graph, *itk, *ito);
                    }
                }
            }
        }

        for (int i = 0; i < kill.size(); i++) {
            Instruction *instruction = f->instructions[i];
            if ((instruction->operators[0] == Operator_Type::SALQ || instruction->operators[0] == Operator_Type::SARQ) &&
                    !isNumber(instruction->operands[1])) {
                set<string> s = registers;
                s.erase("rcx");
                for (set<string>::iterator it = s.begin(); it != s.end(); it++) {
                    add_into_graph(graph, instruction->operands[1], *it);
                }
            }
        }

        return graph;

    }
}