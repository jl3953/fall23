from z3 import *
import sys


# A variable representing the value of a specific cell
def matrixvar(i, j):
    return Int("x_%s_%s" % (i, j))


# Create a 9x9 matrix of integer variables
def getMatrix():
    return [[matrixvar(i + 1, j + 1) for j in range(9)]
            for i in range(9)]


# Students should add their code in the following 4 functions
# (instead of the 'return True' statement)

# Add constraints such that each cell contains a value in {1, ..., 9} 
def addCellConstraints(X):
    constraints = []
    for i in range(9):
        for j in range(9):
            constraints.append(And([1 <= X[i][j], X[i][j] <= 9]))
    return constraints


# Add constraints such that each row contains a digit at most once
def addRowConstraints(X):
    constraints = []
    for row in range(9):
        for col in range(9):
            for digit in range(1, 10):
                constraints.append(
                    Implies(X[row][col] == digit,
                            And([Or([X[row][other_col] < digit,
                                     X[row][other_col] > digit]) for other_col
                                 in range(9) if other_col is not col])))
    return constraints


# Add constraints such that each column contains a digit at most once
def addColumnConstraints(X):
    constraints = []
    for col in range(9):
        for row in range(9):
            for digit in range(1, 10):
                constraints.append(
                    Implies(X[row][col] == digit,
                            And([Or([X[other_row][col] < digit,
                                    X[other_row][col] > digit]) for other_row
                                 in range(9) if other_row is not row])))
    return constraints


# Add constraints such that each 3x3 square contains a digit at most once
def addSquareConstraints(X):
    constraints = []
    for block_row in range(3):
        for block_col in range(3):
            for row in range(block_row*3, (block_row+1)*3):
                for col in range(block_col * 3, (block_col + 1)*3):
                    for digit in range(1, 10):
                        constraints.append(Implies(X[row][col] == digit,
                            And([Or([X[other_row][other_col] < digit,
                                     X[other_row][other_col] > digit])
                                 for other_row in range(block_row*3, (block_row+1)*3) if other_row is not row
                                 for other_col in range(block_col*3, (block_col+1)*3) if other_col is not col])))
    return constraints


def solveSudoku(instance):
    X = getMatrix()

    # Create the initial constraints of the puzzle
    # based on the input instance. Note that '0' represents
    # an empty cell
    instance_c = [If(instance[i][j] == 0,
                     True,
                     X[i][j] == instance[i][j])
                  for i in range(9) for j in range(9)]

    # Create the Z3 solver
    s = Solver()

    # Add all needed constraints
    s.add(instance_c)
    s.add(addCellConstraints(X))
    s.add(addRowConstraints(X))
    s.add(addColumnConstraints(X))
    s.add(addSquareConstraints(X))

    # If the problem is satisfiable, a solution exists
    if s.check() == sat:
        m = s.model()
        r = [[m.evaluate(X[i][j]) for j in range(9)]
             for i in range(9)]
        print_matrix(r)
    else:
        print("failed to solve")
