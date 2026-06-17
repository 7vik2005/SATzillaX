# SATzillaX

<div align="center">

[![C++ Version](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/compiler_support/17)
[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge)](#)
[![Developer](https://img.shields.io/badge/Developer-Satvik%20Jambagi-orange?style=for-the-badge)](https://github.com/7vik2005)
[![GitHub Repository](https://img.shields.io/badge/GitHub-7vik2005%2FSATzillaX-blue?style=for-the-badge&logo=github)](https://github.com/7vik2005/SATzillaX)

</div>

---

## 🚀 Overview

**SATzillaX** is a high-performance **Conflict-Driven Clause Learning (CDCL)** Boolean Satisfiability (SAT) solver written in pure C++17. Designed to be lightweight, clean, and highly efficient, SATzillaX implements the industry-standard algorithms and heuristics found in modern industrial SAT solvers (like Minisat and Chaff) from scratch.

Boolean Satisfiability is an NP-complete problem, making it a foundation of computer science. SATzillaX tackles hard combinatorial and industrial formulas by utilizing dynamic clause learning, non-chronological backtracking, and variable activity scoring.

---

## ✨ Features

- **Conflict-Driven Clause Learning (CDCL)**: Learns new clauses from conflicts to avoid exploring the same unsatisfiable search spaces again.
- **1st UIP (Unique Implication Point) Analysis**: Infers optimal conflict clauses by analyzing the implication graph.
- **Two-Watched Literals (2WL)**: Efficient boolean constraint propagation (BCP) that tracks only two literals per clause, reducing overhead significantly during search.
- **Non-Chronological Backtracking (Backjumping)**: Backtracks multiple decision levels at once based on conflict analysis, rather than backtracking step-by-step.
- **VSIDS (Variable State Independent Decaying Sum)**: A dynamic decision branching heuristic using a binary heap (`VarOrderHeap`) to focus on variables involved in recent conflicts.
- **Phase Saving**: Saves the last assigned truth value of a variable to guide future decisions.
- **Geometric Restarts**: Automatically resets the search tree periodically to escape local minima in the search space, while maintaining learned clauses and VSIDS activities.
- **Robust DIMACS Parser**: Token-based parser handling comments, extra whitespace, and missing problem lines gracefully.

---

## 📂 Project Structure

```
SATzillaX/
├── src/
│   ├── main.cpp         # Entrypoint parsing arguments and invoking the solver
│   ├── parser.h         # DIMACS parser interface and data structures
│   ├── parser.cpp       # Robust token-based CNF parser implementation
│   ├── solver.h         # SATSolver class declaration, structures, and helper classes
│   └── solver.cpp       # CDCL search, unit propagation, conflict analysis, and VSIDS
├── Makefile             # Build automation script
├── .gitignore           # Git ignore file for compiled/temporary artifacts
└── README.md            # Project documentation (this file)
```

---

## 🛠️ Build and Installation

### Prerequisites

You need a C++ compiler supporting the **C++17** standard (like GCC/g++ 7+, Clang 5+, or MSVC 2017+).

### Compile with `g++`

To compile the project with `-O0` performance optimization, run:

```bash
g++ -std=c++17 -g -O0 src/main.cpp src/parser.cpp src/solver.cpp -o SATzillaX
```

### Compile with `Makefile`

If you have `make` installed:

```bash
make
```

To clean build files:

```bash
make clean
```

---

## 📖 Usage & DIMACS Format

SATzillaX accepts input files in the standard **DIMACS CNF format**.

### Executing the Solver

```bash
./SATzillaX <path_to_cnf_file>
```

_(On Windows, run `SATzillaX.exe <path_to_cnf_file>`)_

### Output Format

The output follows standard SAT competition formatting:

- If the formula is satisfiable:
  ```
  SAT
  <assigned_literals> 0
  ```
  _(Positive integers represent `true` variables, negative integers represent `false` variables, terminated by a `0`)_
- If the formula is unsatisfiable:
  ```
  UNSAT
  ```

---

## 💡 Examples

### Example 1: Satisfiable Formula (SAT)

Consider a simple CNF formula representing $(x_1 \lor x_2) \land (\neg x_1 \lor x_3) \land (\neg x_2 \lor \neg x_3)$.

**`example_sat.cnf`:**

```text
c Example satisfiable formula
p cnf 3 3
1 2 0
-1 3 0
-2 -3 0
```

**Running the Solver:**

```bash
./SATzillaX example_sat.cnf
```

**Expected Output:**

```text
SAT
1 -2 3 0
```

_(Meaning: $x_1 = \text{true}$, $x_2 = \text{false}$, $x_3 = \text{true}$ satisfies the formula)_

---

### Example 2: Unsatisfiable Formula (UNSAT)

Consider a simple contradiction: $x_1 \land \neg x_1$.

**`example_unsat.cnf`:**

```text
c Example unsatisfiable formula
p cnf 1 2
1 0
-1 0
```

**Running the Solver:**

```bash
./SATzillaX example_unsat.cnf
```

**Expected Output:**

```text
UNSAT
```

---

## ⚙️ How it Works under the Hood

1. **Pre-processing / Parsing**: The input DIMACS CNF file is tokenized into raw integers. Single-literal unit clauses are propagated immediately.
2. **Decide (Branching)**: Variables are picked based on their VSIDS scores (variables involved in conflicts get higher weights).
3. **Propagate (BCP)**: The 2-watched-literals scheme propagates assignments. If a conflict occurs:
4. **Analyze Conflict**: The solver backtracks along the implication graph to find the **1st UIP**, constructs a learned clause, and adds it to the database.
5. **Backjump**: The solver performs non-chronological backtracking to the decision level where the learned clause becomes unit, enabling immediate propagation of the implication.
6. **Restart**: Periodically, the solver clears assignments and restarts the search from decision level 0, using the learned clauses and saved phases to explore a different region of the search space.

---

## 📜 License

This project is licensed under the **MIT License**. See below or refer to the repository LICENSE file for details.

```text
MIT License

Copyright (c) 2026 Satvik Jambagi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 👤 Developer

- **Developer**: Satvik Jambagi
- **GitHub**: [@7vik2005](https://github.com/7vik2005)
