#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

#include "parser.h"

using namespace std;

int main(int argc, char **argv) {
    bool verbose;

    /* Check the input.
     */
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

    /* Parse the L1 program.
     */
    L1::Program p = L1::L1_parse_file(argv[optind]);

    /* Generate x86_64 code
     */

    const char* MAPS[] {
            "EMPTY", "MOVEQ", "ADDQ", "SUBQ", "IMULQ", "ANDQ", "SALQ", "SARQ",
            "CJUMP", "LABEL", "JMP", "RETURN", "CALL", "PRINT", "ALLOCATE",
            "ARRAY_ERROR", "CISC", "MEM", "INC", "DEC", "LQ", "EQ", "LEQ"
    };

    for (auto f : p.functions) {
        cout << f->name << " has " << f->arguments << " parameters and " << f->locals << " local variables" << endl;
        for (auto i : f->instructions) {
            cout << "types: " << i->operators.size() << endl;
            for (auto t : i->operators) {
                cout << "\t" << MAPS[t];
            }
            cout << endl << "operands: " << i->operands.size() << endl;
            for (auto o : i->operands) {
                cout << "\t" << o;
            }
            cout << endl;
        }
    }

    return 0;
}
