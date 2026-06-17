#include "parser.h"
#include "solver.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "Usage: ./SATzillaX <cnf_file>" << endl;
        return 1;
    }

    string filename = argv[1];

    CNFFormula formula = parseDIMACS(filename);

    SATSolver solver(formula);

    SolverResult result = solver.CDCL();

    solver.printResult(result);

    return 0;
}
