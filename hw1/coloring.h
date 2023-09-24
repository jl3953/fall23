#ifndef COLORING_H_
#define COLORING_H_

#include <vector>
#include "minisat/core/Solver.h"

using namespace std;

// since the minisat solver reverses truth values
const bool TRUE = false;
const bool FALSE = true;

// ***************************************************************************
// A graph class. 
// Note that when adding an edge (n1,n2) n1 must be less or 
// equal to n2. This is only done for simplicity and a more compact 
// implementation.
// ***************************************************************************
class Graph {
public:
    Graph(int nNumberOfNodes) : m_nNumberOfNodes(nNumberOfNodes) {
        m_graph.resize(nNumberOfNodes);
    }

    int getNumberOfNodes() const { return m_nNumberOfNodes; }

    // Not efficient for large graphs
    vector<int> getEdgesForNode(int node) const {
        assert(node < m_nNumberOfNodes);
        assert(node < m_graph.size());
        return m_graph[node];
    }

    // For now allowing duplication
    void addEdge(int n1, int n2) {
        assert(n1 < m_nNumberOfNodes &&
               n2 < m_nNumberOfNodes);
        assert(n1 <= n2);

        // Make sure that the vector can contain the first node
        if (m_graph.size() <= n1)
            m_graph.resize(n1 + 1);

        // Take care of the first node
        m_graph[n1].push_back(n2);
    }

private:
    int m_nNumberOfNodes;
    // A vector of vectors to represent the adjacency list
    // The outer vector is mapping a node (by index) to its
    // vector which represents a container of adjacent nodes.
    vector <vector<int>> m_graph;
};

// ***************************************************************************
// A class modeling the k-coloring problem.
// ***************************************************************************
class Coloring {
public:
    Coloring(const Graph &g, int nNumberOfColors) :
            m_graph(g), m_nNumberOfColors(nNumberOfColors), m_solver() {
        // Prepare the solver with the needed variables
        int nodes = m_graph.getNumberOfNodes();
        for (int c = 0; c < m_nNumberOfColors; c++) {
            for (int n = 0; n < nodes; n++) {
                m_solver.newVar();
            }
        }
    }

    void addOneColorConstraints(int node) {
        assert(node < m_graph.getNumberOfNodes());

        for (int c1 = 0; c1 < m_nNumberOfColors - 1; c1++) {
            for (int c2 = c1 + 1; c2 < m_nNumberOfColors; c2++) {
                Minisat::Var v_n_c1 = node * m_nNumberOfColors + c1;
                Minisat::Var v_n_c2 = node * m_nNumberOfColors + c2;
                printf("coloring: v_%d_%d %d = FALSE or v_%d_%d %d = FALSE\n", node, c1, v_n_c1, node, c2, v_n_c2);
                m_solver.addClause(Minisat::mkLit(v_n_c1, FALSE),
                                   Minisat::mkLit(v_n_c2, FALSE));
            }
        }

        Minisat::vec<Minisat::Lit> clause;
        for (int c = 0; c < m_nNumberOfColors; c++) {
            clause.push(Minisat::mkLit(node * m_nNumberOfColors + c));
            printf("v_%d_%d = TRUE or ", node, c);
        }
        printf("\n");
        m_solver.addClause(clause);

    }

    void addEdgeColoringConstraints(int n1, int n2) {
        assert(n1 < m_graph.getNumberOfNodes() &&
               n2 < m_graph.getNumberOfNodes());
        assert(n1 <= n2);

        for (int c = 0; c < m_nNumberOfColors; c++) {
            Minisat::Var v_n1_c = n1 * m_nNumberOfColors + c;
            Minisat::Var v_n2_c = n2 * m_nNumberOfColors + c;

            printf("edge: v_%d_%d %d= FALSE or v_%d_%d %d= FALSE\n", n1, c, v_n1_c, n2, c, v_n2_c);
            m_solver.addClause(Minisat::mkLit(v_n1_c, FALSE),
                               Minisat::mkLit(v_n2_c, FALSE));
        }


    }

    bool isColorable() {
        // Go over all nodes
        for (int n = 0; n < m_graph.getNumberOfNodes(); n++) {
            // Add the constraints for the node
            addOneColorConstraints(n);

            // Now add constraints for the edges
            vector<int> edges = m_graph.getEdgesForNode(n);
            for (int adjcent = 0; adjcent < edges.size(); adjcent++) {
                printf("node %d, adjacent %d\n", n, edges[adjcent]);
                addEdgeColoringConstraints(n, edges[adjcent]);
            }
        }

        m_solver.toDimacs("jenndebug.cnf");

        bool bResult = m_solver.solve();
        return bResult;
    }

    // The function gets allColoring by reference and returns
    // all k-coloring in this vector. Note that the inner vector
    // represents one assignment
    void givemeAllColoring(vector <vector<Minisat::lbool>> &allColoring) {
        // Go over all nodes
        for (int n = 0; n < m_graph.getNumberOfNodes(); n++) {
            // Add the constraints for the node
            addOneColorConstraints(n);

            // Now add constraints for the edges
            vector<int> edges = m_graph.getEdgesForNode(n);
            for (int adjcent = 0; adjcent < edges.size(); adjcent++) {
                addEdgeColoringConstraints(n, edges[adjcent]);
            }
        }

        bool res = m_solver.solve();
        while (res) {
            vector<Minisat::lbool> solution;
            Minisat::vec <Minisat::Lit> clause;
            for (int n = 0; n < m_graph.getNumberOfNodes(); n++) {
                for (int c = 0; c < m_nNumberOfColors; c++) {

                    Minisat::Var v_n_c = n * m_nNumberOfColors + c;

                    // add coloring to solution
                    solution.push_back(m_solver.modelValue(v_n_c));

                    if (m_solver.modelValue(n * m_nNumberOfColors + c) == Minisat::l_True) {
                        printf("node %d is color %d, v_%d_%d true\n", n, c, n, c);

                        // add to clause
                        clause.push(Minisat::mkLit(v_n_c, FALSE));
                    }
                }
            }
            allColoring.push_back(solution);
            m_solver.addClause(clause);
            res = m_solver.solve();
        }
    }

private:
    Minisat::Var getNodeHasColorVar(int node, int color) {
        assert(node < m_graph.getNumberOfNodes() &&
               color < m_nNumberOfColors);

        return (color * m_graph.getNumberOfNodes()) + node;
    }

private:
    const Graph &m_graph;
    int m_nNumberOfColors;

    Minisat::Solver m_solver;
};

#endif // COLORING_H_
