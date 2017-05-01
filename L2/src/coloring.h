#pragma once

#include <string>
#include <map>
#include <set>

using namespace std;

namespace L2 {
    void graph_coloring(const map<string, set<string>> &graph, map<string, string> &reg_map, set<string> &spill);
}