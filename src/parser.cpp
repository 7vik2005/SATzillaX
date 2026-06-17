#include "parser.h"

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

CNFFormula parseDIMACS(const string& filename)
{
    CNFFormula formula;
    formula.numVariables = 0;
    formula.numClauses = 0;

    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }

    string token;
    vector<int> current_clause;

    while (file >> token)
    {
        if (token.empty())
        {
            continue;
        }

        if (token[0] == 'c')
        {
            string dummy;
            getline(file, dummy);
            continue;
        }

        if (token[0] == 'p')
        {
            string format;
            file >> format >> formula.numVariables >> formula.numClauses;
            continue;
        }

        try
        {
            int literal = stoi(token);
            if (literal == 0)
            {
                if (!current_clause.empty())
                {
                    formula.clauses.push_back(current_clause);
                    current_clause.clear();
                }
            }
            else
            {
                current_clause.push_back(literal);
            }
        }
        catch (const exception& e)
        {
            // Ignore any parsing issues for non-integer tokens
        }
    }

    if (!current_clause.empty())
    {
        formula.clauses.push_back(current_clause);
    }

    file.close();

    // Auto-infer number of variables and clauses
    int max_var = 0;
    for (const auto& clause : formula.clauses)
    {
        for (int lit : clause)
        {
            max_var = max(max_var, abs(lit));
        }
    }
    formula.numVariables = max(formula.numVariables, max_var);
    formula.numClauses = formula.clauses.size();

    return formula;
}
