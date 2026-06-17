#include "solver.h"
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

// --- VarOrderHeap Implementation ---

VarOrderHeap::VarOrderHeap(const vector<double>& act) : activity(act) {}

bool VarOrderHeap::lt(int x, int y) const
{
    return activity[x] > activity[y]; // Max heap: higher activity comes first
}

void VarOrderHeap::shrink(int i)
{
    while (i > 0)
    {
        int p = (i - 1) >> 1;
        if (lt(heap[i], heap[p]))
        {
            swap(heap[i], heap[p]);
            indices[heap[i]] = i;
            indices[heap[p]] = p;
            i = p;
        }
        else
        {
            break;
        }
    }
}

void VarOrderHeap::grow(int i)
{
    while ((i << 1 | 1) < static_cast<int>(heap.size()))
    {
        int left = i << 1 | 1;
        int right = left + 1;
        int best = i;
        if (lt(heap[left], heap[best])) best = left;
        if (right < static_cast<int>(heap.size()) && lt(heap[right], heap[best])) best = right;
        if (best != i)
        {
            swap(heap[i], heap[best]);
            indices[heap[i]] = i;
            indices[heap[best]] = best;
            i = best;
        }
        else
        {
            break;
        }
    }
}

void VarOrderHeap::insert(int v)
{
    if (indices[v] != -1) return;
    indices[v] = heap.size();
    heap.push_back(v);
    shrink(indices[v]);
}

void VarOrderHeap::update(int v)
{
    if (indices[v] == -1) return;
    shrink(indices[v]);
    grow(indices[v]);
}

int VarOrderHeap::pop()
{
    int res = heap[0];
    indices[res] = -1;
    if (heap.size() > 1)
    {
        heap[0] = heap.back();
        indices[heap[0]] = 0;
        heap.pop_back();
        grow(0);
    }
    else
    {
        heap.pop_back();
    }
    return res;
}

bool VarOrderHeap::empty() const
{
    return heap.empty();
}

// --- SATSolver Implementation ---

SATSolver::SATSolver(const CNFFormula& formula)
    : numVariables(formula.numVariables),
      numClauses(formula.numClauses),
      clauses(formula.clauses),
      currentDecisionLevel(0),
      qhead(0),
      activity(numVariables, 0.0),
      var_inc(1.0),
      var_decay(0.95),
      heap(activity),
      conflictClauseIndex(-1),
      seen(numVariables, false)
{
    assignments.resize(numVariables, -1);
    decisionLevels.resize(numVariables, -1);
    reasons.resize(numVariables, -1);
    phases.resize(numVariables, 0);

    watches.resize(2 * numVariables + 2);

    heap.indices.resize(numVariables, -1);
    for (int i = 0; i < numVariables; i++)
    {
        heap.insert(i);
    }
}

bool SATSolver::initialize()
{
    assignments.assign(numVariables, -1);
    decisionLevels.assign(numVariables, -1);
    reasons.assign(numVariables, -1);
    phases.assign(numVariables, 0);
    trail.clear();
    trail_lim.clear();
    trail_lim.push_back(0);
    currentDecisionLevel = 0;
    qhead = 0;

    for (auto& list : watches)
    {
        list.clear();
    }

    for (size_t i = 0; i < clauses.size(); i++)
    {
        const auto& clause = clauses[i];
        if (clause.empty())
        {
            return false;
        }
        if (clause.size() == 1)
        {
            int lit = clause[0];
            int var = abs(lit) - 1;
            if (assignments[var] != -1)
            {
                bool is_true = (lit > 0) ? (assignments[var] == 1) : (assignments[var] == 0);
                if (!is_true)
                {
                    return false;
                }
            }
            else
            {
                assignLiteral(lit, 0, -1);
            }
        }
        else
        {
            watches[litToId(clause[0])].push_back(i);
            watches[litToId(clause[1])].push_back(i);
        }
    }
    return true;
}

void SATSolver::assignLiteral(int lit, int level, int reason)
{
    int var = abs(lit) - 1;
    assignments[var] = (lit > 0) ? 1 : 0;
    decisionLevels[var] = level;
    reasons[var] = reason;
    trail.push_back(lit);
}

void SATSolver::backtrack(int level)
{
    if (currentDecisionLevel <= level) return;

    int limit = trail_lim[level + 1];
    while (static_cast<int>(trail.size()) > limit)
    {
        int lit = trail.back();
        trail.pop_back();
        int var = abs(lit) - 1;
        assignments[var] = -1;
        reasons[var] = -1;
        decisionLevels[var] = -1;
        phases[var] = (lit > 0) ? 1 : 0;
        heap.insert(var);
    }

    trail_lim.resize(level + 1);
    currentDecisionLevel = level;
    qhead = trail.size();
}

SolverResult SATSolver::unitPropagate(int level)
{
    while (qhead < static_cast<int>(trail.size()))
    {
        int lit = trail[qhead++];
        int false_lit = -lit;
        int false_lit_id = litToId(false_lit);


        auto& ws = watches[false_lit_id];
        size_t i = 0, j = 0;

        while (i < ws.size())
        {
            int clause_idx = ws[i];
            auto& clause = clauses[clause_idx];

            if (clause[0] == false_lit)
            {
                swap(clause[0], clause[1]);
            }

            if (isSat(clause[0]))
            {
                ws[j++] = clause_idx;
                i++;
                continue;
            }

            bool found_new_watch = false;
            for (size_t k = 2; k < clause.size(); k++)
            {
                if (!isFalsified(clause[k]))
                {
                    swap(clause[1], clause[k]);
                    watches[litToId(clause[1])].push_back(clause_idx);
                    found_new_watch = true;
                    break;
                }
            }

            if (found_new_watch)
            {
                i++;
                continue;
            }

            if (isUnassigned(clause[0]))
            {
                ws[j++] = clause_idx;
                assignLiteral(clause[0], level, clause_idx);
            }
            else
            {
                while (i < ws.size())
                {
                    ws[j++] = ws[i++];
                }
                ws.resize(j);
                conflictClauseIndex = clause_idx;
                return UNSAT;
            }
            i++;
        }
        ws.resize(j);
    }
    return UNRESOLVED;
}

int SATSolver::analyzeConflict(int conflictClauseIdx, vector<int>& out_learned)
{
    out_learned.clear();
    out_learned.push_back(0); // UIP placeholder

    int path_cnt = 0;
    int p = (int)trail.size() - 1;
    int c_index = conflictClauseIdx;
    int pivot_var = -1;
    int last_pivot_lit = 0;

    do
    {
        if (c_index < 0 || c_index >= (int)clauses.size())
        {
            break;
        }
        const auto& clause = clauses[c_index];
        for (int lit : clause)
        {
            int v = abs(lit) - 1;
            if (v < 0 || v >= numVariables) continue;
            if (v == pivot_var) continue;
            if (decisionLevels[v] == 0) continue;

            if (!seen[v])
            {
                seen[v] = true;
                activity[v] += var_inc;
                heap.update(v);

                if (decisionLevels[v] == currentDecisionLevel)
                {
                    path_cnt++;
                }
                else
                {
                    out_learned.push_back(lit);
                }
            }
        }

        while (p >= 0)
        {
            int trail_var = abs(trail[p]) - 1;
            if (trail_var >= 0 && trail_var < numVariables && seen[trail_var])
            {
                break;
            }
            p--;
        }
        if (p < 0) break;

        last_pivot_lit = trail[p];
        pivot_var = abs(last_pivot_lit) - 1;
        if (pivot_var < 0 || pivot_var >= numVariables)
        {
            break;
        }
        c_index = reasons[pivot_var];
        seen[pivot_var] = false;
        path_cnt--;
        p--;
    } while (path_cnt > 0);

    if (last_pivot_lit == 0)
    {
        return 0;
    }

    out_learned[0] = -last_pivot_lit;

    for (size_t i = 1; i < out_learned.size(); i++)
    {
        int var = abs(out_learned[i]) - 1;
        if (var >= 0 && var < numVariables)
        {
            seen[var] = false;
        }
    }
    int uip_var = abs(out_learned[0]) - 1;
    if (uip_var >= 0 && uip_var < numVariables)
    {
        seen[uip_var] = false;
    }

    var_inc /= var_decay;
    if (var_inc > 1e100)
    {
        for (int i = 0; i < numVariables; i++)
        {
            activity[i] *= 1e-100;
        }
        var_inc *= 1e-100;
    }

    int backtrack_level = 0;
    if (out_learned.size() > 1)
    {
        for (size_t i = 1; i < out_learned.size(); i++)
        {
            int var = abs(out_learned[i]) - 1;
            if (var >= 0 && var < numVariables)
            {
                int lvl = decisionLevels[var];
                if (lvl > backtrack_level)
                {
                    backtrack_level = lvl;
                }
            }
        }
    }
    return backtrack_level;
}

int SATSolver::pickBranchingVariable()
{
    while (!heap.empty())
    {
        int v = heap.pop();
        if (assignments[v] == -1)
        {
            return (phases[v] == 1) ? (v + 1) : -(v + 1);
        }
    }
    return 0;
}

SolverResult SATSolver::CDCL()
{
    if (!initialize())
    {
        return UNSAT;
    }

    int conflicts = 0;
    int restart_limit = 100;

    if (unitPropagate(0) == UNSAT)
    {
        return UNSAT;
    }

    while (true)
    {
        if (static_cast<int>(trail.size()) == numVariables)
        {
            return SAT;
        }

        int lit = pickBranchingVariable();
        if (lit == 0)
        {
            return SAT;
        }

        trail_lim.push_back(trail.size());
        currentDecisionLevel++;

        assignLiteral(lit, currentDecisionLevel, -1);

        while (true)
        {
            SolverResult res = unitPropagate(currentDecisionLevel);
            if (res == UNSAT)
            {
                if (currentDecisionLevel == 0)
                {
                    return UNSAT;
                }

                vector<int> learned;
                int backtrack_level = analyzeConflict(conflictClauseIndex, learned);

                backtrack(backtrack_level);

                conflicts++;

                if (learned.size() >= 2)
                {
                    int max_i = 1;
                    int max_lvl = decisionLevels[abs(learned[1]) - 1];
                    for (size_t i = 2; i < learned.size(); i++)
                    {
                        int lvl = decisionLevels[abs(learned[i]) - 1];
                        if (lvl > max_lvl)
                        {
                            max_lvl = lvl;
                            max_i = i;
                        }
                    }
                    swap(learned[1], learned[max_i]);
                }

                clauses.push_back(learned);
                int learned_idx = clauses.size() - 1;

                if (learned.size() >= 2)
                {
                    watches[litToId(learned[0])].push_back(learned_idx);
                    watches[litToId(learned[1])].push_back(learned_idx);
                    assignLiteral(learned[0], backtrack_level, learned_idx);
                }
                else
                {
                    assignLiteral(learned[0], 0, -1);
                }

                if (conflicts >= restart_limit)
                {
                    backtrack(0);
                    restart_limit = static_cast<int>(restart_limit * 1.5);
                    conflicts = 0;
                    if (unitPropagate(0) == UNSAT)
                    {
                        return UNSAT;
                    }
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
}

void SATSolver::printResult(SolverResult result)
{
    if (result == SAT)
    {
        cout << "SAT" << endl;
        for (int i = 0; i < numVariables; i++)
        {
            if (assignments[i] == 1)
            {
                cout << i + 1 << " ";
            }
            else
            {
                cout << -(i + 1) << " ";
            }
        }
        cout << "0" << endl;

    }
    else
    {
        cout << "UNSAT" << endl;
    }
}
