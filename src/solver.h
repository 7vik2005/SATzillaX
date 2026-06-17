#pragma once

#include "parser.h"
#include <vector>
#include <string>
#include <cmath>

using namespace std;

enum SolverResult
{
    SAT,
    UNSAT,
    UNRESOLVED
};

struct VarOrderHeap
{
    const vector<double>& activity;
    vector<int> heap;
    vector<int> indices;

    VarOrderHeap(const vector<double>& act);
    bool lt(int x, int y) const;
    void shrink(int i);
    void grow(int i);
    void insert(int v);
    void update(int v);
    int pop();
    bool empty() const;
};

class SATSolver
{
private:
    int numVariables;
    int numClauses;
    vector<vector<int>> clauses;

    // Solver state: variables are 0-indexed internally
    vector<int> assignments;      // -1: unassigned, 0: false, 1: true
    vector<int> decisionLevels;   // var -> level
    vector<int> reasons;          // var -> clause index (-1 if none)
    vector<int> phases;           // var -> saved phase (0 or 1)
    
    // Trail for backtracking
    vector<int> trail;
    vector<int> trail_lim;        // start index of each decision level in trail
    int currentDecisionLevel;
    int qhead;

    // Two-Watched Literals (indexed by litId)
    vector<vector<int>> watches;  // litId -> list of clause indices

    // VSIDS Heuristic
    vector<double> activity;
    double var_inc;
    double var_decay;
    VarOrderHeap heap;

    int conflictClauseIndex;

    // Conflict analysis temp helper
    vector<bool> seen;

    // Helper functions
    inline int litToId(int lit) const
    {
        return lit > 0 ? (lit << 1) : ((-lit) << 1 | 1);
    }

    inline int negLit(int lit) const
    {
        return -lit;
    }

    inline bool isSat(int lit) const
    {
        int var = abs(lit) - 1;
        int val = assignments[var];
        if (val == -1) return false;
        return (lit > 0) ? (val == 1) : (val == 0);
    }

    inline bool isFalsified(int lit) const
    {
        int var = abs(lit) - 1;
        int val = assignments[var];
        if (val == -1) return false;
        return (lit > 0) ? (val == 0) : (val == 1);
    }

    inline bool isUnassigned(int lit) const
    {
        return assignments[abs(lit) - 1] == -1;
    }

    void assignLiteral(int lit, int level, int reason);
    void backtrack(int level);
    SolverResult unitPropagate(int level);
    int analyzeConflict(int conflictClauseIdx, vector<int>& out_learned);
    int pickBranchingVariable();
    bool initialize();

public:
    SATSolver(const CNFFormula& formula);
    SolverResult CDCL();
    void printResult(SolverResult result);
};
