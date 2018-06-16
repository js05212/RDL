#ifndef ROPT_H
#define ROPT_H
#include <gsl/gsl_vector.h>
#include "graph.h"

void optimizew(int j,gsl_vector* w,gsl_matrix* m_W,c_neighborhood* nbh,c_neighborhood* tnbh, gsl_vector *etap, gsl_vector* v, double lambda_w,double lambda_l);


double f_likeli(int j,gsl_vector* w,gsl_matrix* m_W,c_neighborhood* nbh,c_neighborhood* tnbh, gsl_vector *etap, gsl_vector* v, double lambda_w,double lambda_l);

void df_likeli(int j,gsl_vector* w,gsl_matrix* m_W,c_neighborhood* nbh,c_neighborhood* tnbh, gsl_vector *etap, double lambda_w, double lambda_l, gsl_vector* v, gsl_vector* g);

double optimizeetap(const c_graph *graph,const c_graph *tgraph,gsl_matrix* m_W,gsl_vector* etap,double lambda_e,double lambda_l);

double f_likeli_etap(const c_graph *graph,const c_graph *tgraph,gsl_matrix* m_W,gsl_vector* etap,double lambda_e,double lambda_l);

void df_likeli_etap(const c_graph *graph,const c_graph *tgraph,gsl_matrix* m_W,gsl_vector* etap,double lambda_e,double lambda_l,gsl_vector* g);

double sigmoid(double x);

double linkll(int j,c_neighborhood* nbh,c_neighborhood* tnbh,double lambda_l,gsl_matrix* m_W,gsl_vector* etap);

#endif 
