// class for relation matrix, in lda-c format

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <cstddef>

using namespace std;

class c_neighborhood{
public:
    int * m_nbs;    // neighbors
    double * m_weights;
    int m_length;
public:
    c_neighborhood(){
        m_nbs = NULL;
        m_weights = NULL;
        m_length = 0;
    }

    c_neighborhood(int len){
        m_length = len;
        m_nbs = new int[len];
        m_weights = new double[len];
    }

    ~c_neighborhood(){
        if (m_nbs!=NULL){
            delete [] m_nbs;
            delete [] m_weights;
            m_length = 0;
        }
    }
};

class c_graph{
public:
    c_graph();
    ~c_graph();
    void read_data(const char *data_filename, int OFFSET=0);
    int max_node_num() const;
    void display() const;
public:
    int m_num_node;
    int m_2num_edge;
    vector<c_neighborhood*> m_nodes;
};

#endif // GRAPH_H

