#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "l3.h"
#include "parser.h"


using namespace L3;
using namespace std;


int main(int argc, char **argv) {
    bool verbose;

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " SOURCE [-v]" << endl;
        return 1;
    }
    int32_t opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            default:
                cerr << "Usage: " << argv[0] << "[-v] SOURCE" << endl;
                return 1;
        }
    }

    ofstream output;
    output.open("prog.L2");
    Program p = L3ParseFile(argv[optind]);
    vector <string> l2 = p.toL2();
    for (auto const &s : l2) {
        output << s << endl;
    }
    output.close();
    return 0;
}