#pragma once

#include <map>
#include <set>
#include <string>

#include "L2.h"

using namespace std;

namespace L2 {
    map<string, set<string>> compute_interference_graph(Function *f);
}