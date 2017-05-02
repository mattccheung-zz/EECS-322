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
        for (auto const &reg : registers) {
            set<string> s = registers;
            s.erase(reg);
            graph[reg] = s;
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
            Instruction *inst = f->instructions[i];
            for (auto const &k : kill[i]) {
                for (auto const &o : out[i]) {
                    if (inst->operators.size() == 1 && inst->operators.front() == Operator_Type::MOVQ &&
                        k == inst->operands[0] && o == inst->operands[1]) {
                        continue;
                    } else {
                        add_into_graph(graph, k, o);
                    }
                }
            }
        }

        for (int i = 0; i < kill.size(); i++) {
            Instruction *inst = f->instructions[i];
            if ((inst->operators[0] == Operator_Type::SALQ || inst->operators[0] == Operator_Type::SARQ) &&
                    !isNumber(inst->operands[1])) {
                set<string> s = registers;
                s.erase("rcx");
                for (auto const &reg : s) {
                    add_into_graph(graph, inst->operands[1], reg);
                }
            }
        }

        return graph;

    }
}