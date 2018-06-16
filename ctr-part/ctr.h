// class for ctr
//
#ifndef CTR_H
#define CTR_H

#include "utils.h"
#include "corpus.h"
#include "data.h"
#include "graph.h"

struct ctr_hyperparameter {
  double a;
  double b;
  double lambda_w;
  double lambda_t;
  double lambda_e;
  double lambda_l;
  int    random_seed;
  int    max_iter;
  int    save_lag;
  int    theta_opt;
  int    ctr_run;
  
  void set(double aa, double bb, 
           double lw,double lt,double le,double ll, 
           int rs,    int mi, 
           int sl,    int to,
           int cr) {
    a = aa; b = bb; 
    lambda_w = lw; lambda_t = lt; lambda_e=le;lambda_l=ll;
    random_seed = rs; max_iter = mi;
    save_lag = sl; theta_opt = to;
    ctr_run = cr;
  }

  void save(char* filename) {
    FILE * file = fopen(filename, "w");
    fprintf(file, "a = %.4f\n", a);
    fprintf(file, "b = %.4f\n", b);
    fprintf(file, "lambda_w = %.4f\n", lambda_w);
    fprintf(file, "lambda_t = %.4f\n", lambda_t);
    fprintf(file, "lambda_e = %.4f\n", lambda_e);
    fprintf(file, "lambda_l = %.4f\n", lambda_l);
    fprintf(file, "random seed = %d\n", (int)random_seed);
    fprintf(file, "max iter = %d\n", max_iter);
    fprintf(file, "save lag = %d\n", save_lag);
    fprintf(file, "theta opt = %d\n", theta_opt);
    fprintf(file, "ctr run = %d\n", ctr_run);
    fclose(file);
  }
};

class c_ctr {
public:
  c_ctr();
  ~c_ctr();
  void read_init_information(const char* theta_init_path);

  void set_model_parameters(int num_factors, 
                            int num_items);

  void learn_map_estimate(const c_graph* graph, 
                          const c_graph* tgraph, 
                          const ctr_hyperparameter* param,
                          const char* directory);
  void init_model(int ctr_run);

public:
  gsl_matrix* m_theta;
  gsl_vector* m_etap;   // eta+//added

  gsl_matrix* m_W;
  gsl_matrix* m_T;

  int m_num_factors; // m_num_topics
  int m_num_items; // m_num_docs
};

#endif // CTR_H
