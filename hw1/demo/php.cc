// ********************************************************************
// Modeling the Pigeon Hole Problem with MiniSAT
// Can p pigeons fit in h holes?
// Run the command: ./php p h
//
// For example, if we have 3 pigeons and 2 holes:
// We introduce a variable for each possible position of the pigeon.
// This means 6 variables:
//          | hole 0 | hole 1
//          -----------------
// pigeon 1 | v_0_0  | v_0_1
// pigeon 2 | v_1_0  | v_1_1
// pigeon 3 | v_2_0  | v_2_1
//
// Now the constraints:
// 1. We want to make sure that a pigeon is in a hole:
//    (v_0_0 or v_0_1) and
//    (v_1_0 or v_1_1) and
//    (v_2_0 or v_2_1)
// 2. We want to make sure only one pigeon is placed in a given hole:
//    (!v_0_0 or !v_1_0) and (!v_0_0 or !v_2_0) and (!v_1_0 or !v_2_0)
//    and
//    (!v_0_1 or !v_1_1) and (!v_0_1 or !v_2_1) and (!v_1_1 or !v_2_1)
// ********************************************************************

#include <stdlib.h>
#include <stdio.h>

#include "minisat/core/Solver.h"
#include "minisat/utils/System.h"

int main(int argc, char** argv) {

  if (argc != 3)
    exit(1);

  int p = atoi(argv[1]);
  int h = atoi(argv[2]);

  if ((p < 1) || (h < 1))
    exit(1);

  printf("Trying to place %d pigeons in %d holes\n", p, h);
  
  Minisat::Solver s;
  s.verbosity = 1;

  // Create the variables
  for (int i=0; i < p; i++) {
    for (int j=0; j < h; j++)
      Minisat::Var v = s.newVar(); // Var h*i + j = pigeon i has a place in hole j;
  }

  // p clauses - each pigeon has to be placed in some hole
  for (int i=0; i < p; i++) {
    Minisat::vec<Minisat::Lit> clause;
    for (int j=0; j < h; j++) {
      Minisat::Var v = h*i + j; // = pigeon i has a place in hole j;
      clause.push(Minisat::mkLit(v, false));
    }
    s.addClause(clause);
  }

  // For each hole make sure that only one pigeon is placed in it.
  for (int j=0; j < h; j++)
    for (int i=0; i < p-1; i++)
      for (int k=i+1; k < p; k++) {
        s.addClause(Minisat::mkLit(h*i + j, true), Minisat::mkLit(h*k + j, true));
      }
  
  double initial_time = Minisat::cpuTime();

  // Write out the SAT problem in DIMACS format
  s.toDimacs("problem.cnf");

  // Call the solver
  bool res = s.solve();

  // Print the result and the satisfying solution (if SAT) 
  if (res) {
    printf("There is a solution!\n");
    for (int i=0; i < p; i++)
      for (int j=0; j < h; j++)
        if (s.modelValue(h*i +j) == Minisat::l_True)
          printf("Pigeon %d in hole %d\n", i, j);
  }
  else printf("UNSAT!\n");

  // Print solver statistics
  printf("Time to solve: %12.2f s\n", Minisat::cpuTime() - initial_time);
  return 0;

}
