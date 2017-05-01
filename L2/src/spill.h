#pragma once

#include <map>
#include <set>
#include <string>

#include "L2.h"

using namespace std;

namespace L2 {
    Function *replace_and_spill(Function *f, const map<string, string> &reg_map, const set<string> &spill);
}