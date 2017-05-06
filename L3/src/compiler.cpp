#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>


#include "l3.h"
#include "parser.h"


using namespace L3;
using namespace std;


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

    ofstream output;
    output.open("prog.L2");
    Program p = L3ParseFile(argv[optind]);
    output.close();
    return 0;
}