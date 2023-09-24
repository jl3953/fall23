from z3 import *
from php import solvePHP

if __name__ == '__main__':
    if (len(sys.argv) == 2):
        n = (int)(sys.argv[1])
        print(n)
        solvePHP(n)
