#include <string>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;

namespace L2 {

    const int k = 15;

    const vector<string> ordered_registers = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx",
                                              "rsi", "r12", "r13", "r14", "r15", "rbp", "rbx"};

    const set<string> registers(ordered_registers.begin(), ordered_registers.end());

    inline bool is_register(const string &s) {
        return s == "rsp" || registers.find(s) != registers.end();
    }

    void rebuild_graph(const map<string, set<string>> &graph, map<string, string> &reg_map,
                       stack<string> &variable_stack, set<string> &spill) {
        set<string> adjacent_colors;
        string node;
        while (!variable_stack.empty()) {
            node = variable_stack.top();
            variable_stack.pop();
            if (!is_register(node)) {
                adjacent_colors.clear();
                for (auto const &entry : graph.at(node)) {
                    if (is_register(entry)) {
                        adjacent_colors.insert(entry);
                    } else {
                        if (reg_map.find(entry) != reg_map.end()) {
                            adjacent_colors.insert(reg_map[entry]);
                        }
                    }
                }
                if (adjacent_colors.size() < k) {
                    for (auto const &reg : ordered_registers) {
                        if (adjacent_colors.find(reg) == adjacent_colors.end()) {
                            reg_map[node] = reg;
                            break;
                        }
                    }
                } else {
                    spill.insert(node);
                }
            }
        }
    }

    struct cmp {
        bool operator() (const pair<string, set<string>> &left, const pair<string, set<string>> &right) {
            return left.second.size() < right.second.size();
        }
    };

    void graph_coloring(const map<string, set<string>> &graph, map<string, string> &reg_map, set<string> &spill) {
        vector<pair<string, set<string>>> ordered_graph;
        stack<string> variable_stack;
        for (auto const &entry : graph) {
            ordered_graph.push_back(make_pair(entry.first, entry.second));
        }
        sort(ordered_graph.begin(), ordered_graph.end(), cmp());
        int index = 0;
        while (ordered_graph[index].second.size() < k) {
            index++;
        }
        for (int i = k - 1; i >= 0; i--) {
            variable_stack.push(ordered_graph[i].first);
        }
        for (int i = ordered_graph.size() - 1; i >= k; i--) {
            variable_stack.push(ordered_graph[i].first);
        }

        rebuild_graph(graph, reg_map, variable_stack, spill);
    }
}
