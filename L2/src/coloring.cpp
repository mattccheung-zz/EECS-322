#include <string>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <stack>

using namespace std;

namespace L2 {

    const int k = 15;

    const vector<string> ordered_registers = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx",
                                              "rsi", "r12", "r13", "r14", "r15", "rbp", "rbx"};

    const set<string> registers(ordered_registers.begin(), ordered_registers.end());

    inline bool is_register(const string &s) {
        return registers.find(s) != registers.end();
    }

    bool rebuild_graph(const map<string, set<string>> &graph, map<string, set<string>> &graph_built,
                       map<string, string> &reg_map, stack<string> &variable_stack) {
        while (!variable_stack.empty()) {
            string node = variable_stack.top();
            variable_stack.pop();
            set<string> adjacent_colors;
            set<string> adjacent_variables;
            for (auto const &entry : graph_built) {
                if (graph.at(entry.first).find(node) != graph.at(entry.first).end()) {
                    if (is_register(entry.first)) {
                        adjacent_colors.insert(entry.first);
                    } else {
                        adjacent_colors.insert(reg_map[entry.first]);
                    }
                    adjacent_variables.insert(entry.first);
                }
            }
            if ((is_register(node) && adjacent_colors.find(node) != adjacent_colors.end()) || adjacent_colors.size() >= k) {
                return false;
            } else {
                if (!is_register(node)) {
                    for (auto const &reg : ordered_registers) {
                        if (adjacent_colors.find(reg) == adjacent_colors.end()) {
                            reg_map[node] = reg;
                            break;
                        }
                    }
                }
                graph_built[node] = adjacent_variables;
                for (auto const &variable : adjacent_variables) {
                    graph_built[variable].insert(node);
                }
            }
        }
        return true;
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

        map<string, set<string>> graph_built;
        if (!rebuild_graph(graph, graph_built, reg_map, variable_stack)) {
            reg_map.clear();
            for (auto const &entry : graph) {
                if (!is_register(entry.first)) {
                    spill.insert(entry.first);
                }
            }
        }
    }
}