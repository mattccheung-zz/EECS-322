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

    for (int i = 0; i < instructions.size(); i++) {

    }

    print_result(in, out, n);

    return 0;
}
