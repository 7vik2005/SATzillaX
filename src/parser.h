#pragma once

#include <vector>
#include <string>

using namespace std;

/*
 * CNF Formula Representation
 *
 * Example:
 *
 * p cnf 3 2
 * 1 -2 0
 * 2 3 0
 *
 * variables = 3
 * clauses = {
 *     {1, -2},
 *     {2, 3}
 * }
 */
struct CNFFormula
{
    int numVariables;
    int numClauses;

    vector<vector<int>> clauses;
};

/*
 * Reads a DIMACS CNF file and returns
 * the parsed formula.
 */
CNFFormula parseDIMACS(const string& filename);
