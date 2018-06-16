#include "graph.h"
#include <assert.h>
#include <stdio.h>

c_graph::c_graph(){
    m_num_node = 0;
    m_2num_edge = 0;
}

c_graph::~c_graph(){
    for (int i = 0;i<m_num_node;i++){
        c_neighborhood * node = m_nodes[i];
        delete node;
    }
    m_nodes.clear();

    m_num_node = 0;
}

void c_graph::read_data(const char *data_filename, int OFFSET){
    int numnode, length = 0, nb = 0, n;
    FILE * fileptr;
    numnode = 0;
    fileptr = fopen(data_filename,"r");

    printf("reading data from %s\n",data_filename);
    while((fscanf(fileptr,"%10d",&length)!=EOF)){
        c_neighborhood * nbh = new c_neighborhood(length);
        nbh->m_length = length;
        m_2num_edge += length;
        for (n = 0;n<length;n++){
            fscanf(fileptr,"%10d",&nb);
            nb = nb-OFFSET;
            nbh->m_nbs[n] = nb;
        }
        m_nodes.push_back(nbh);
        numnode++;
    }
    fclose(fileptr);
    m_num_node = numnode;
    printf("number of nodes: %d\n",numnode);
    printf("number of edges: %d\n",m_2num_edge/2);
}

void c_graph::display()const{
    int ndis = m_num_node;
    int i,j,mlen;
    for(i = 0;i<ndis;i++){
        c_neighborhood* nbhp = m_nodes[i];
        mlen = nbhp->m_length;
        printf("%d:\t",mlen);
        for(j = 0;j<mlen;j++){
            printf("%d\t",nbhp->m_nbs[j]);
        }
        printf("\n");
    }
}
