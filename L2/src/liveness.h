#pragma once

#include <vector>
#include <set>

#include "L2.h"

using namespace std;

namespace L2 {
    void live_analysis(Function *f, vector<set<string>> &gen, vector<set<string>> &kill, vector<set<string>> &in, vector<set<string>> &out);
}