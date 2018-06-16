#include "ctr.h"
#include "opt.h"
#include "ropt.h"
#include "utdb.h"
#include <cstring>

extern gsl_rng * RANDOM_NUMBER;
int min_iter = 1;

c_ctr::c_ctr() {
  m_theta = NULL;
  m_W = NULL;
  m_T = NULL;

  m_num_factors = 0; // m_num_topics
  m_num_items = 0; // m_num_docs
}

c_ctr::~c_ctr() {
  // free memory
  if (m_theta != NULL) gsl_matrix_free(m_theta);
  if (m_W != NULL) gsl_matrix_free(m_W);
  if (m_T != NULL) gsl_matrix_free(m_T);
}

void c_ctr::read_init_information(const char* theta_init_path) {
  m_theta = gsl_matrix_alloc(m_num_items, m_num_factors);
  printf("\nreading theta initialization from %s\n", theta_init_path);
  FILE * f = fopen(theta_init_path, "r");
  mtx_fscanf(f, m_theta);
  fclose(f);

  //normalize m_theta, in case it's not
  for (size_t j = 0; j < m_theta->size1; j ++) {
    gsl_vector_view theta_v = gsl_matrix_row(m_theta, j);
    vnormalize(&theta_v.vector);
  }

}

void c_ctr::set_model_parameters(int num_factors, 
                                 int num_items) {
  m_num_factors = num_factors;
  m_num_items = num_items;
}

void c_ctr::init_model(int ctr_run) {

  m_W = gsl_matrix_calloc(m_num_items, m_num_factors);
  m_T = gsl_matrix_calloc(m_num_items, m_num_factors);
  // initialize eta+ //added
  m_etap = gsl_vector_calloc(m_num_factors+1);
  for (size_t i = 0;i<m_etap->size;i++){
      gsl_vector_set(m_etap,i,runiform());
  }

  gsl_matrix_memcpy(m_W, m_theta);
  gsl_matrix_memcpy(m_T, m_theta);
}

void c_ctr::learn_map_estimate(const c_graph* graph,
                               const c_graph* tgraph,
                               const ctr_hyperparameter* param,
                               const char* directory) {
  // init model parameters
  printf("\ninitializing the model ...\n");
  init_model(1);
  // filename
  char name[500];

  // start time
  time_t start, current;
  time(&start);
  int elapsed = 0;

  int iter = 0;
  double likelihood_out;
  double likelihood = -exp(50), likelihood_old;
  double converge = 1.0;

  /// create the state log file 
  sprintf(name, "%s/state.log", directory);
  FILE* file = fopen(name, "w");
  fprintf(file, "iter time likelihood converge\n");


  /* alloc auxiliary variables */
  gsl_matrix* XX = gsl_matrix_alloc(m_num_factors, m_num_factors);
  gsl_matrix* A  = gsl_matrix_alloc(m_num_factors, m_num_factors);
  gsl_matrix* B  = gsl_matrix_alloc(m_num_factors, m_num_factors);
  gsl_vector* x  = gsl_vector_alloc(m_num_factors);

  /* tmp variables for indexes */
  //int i, l, m, n;
  int j, k;
  c_neighborhood* nbh;
  c_neighborhood* tnbh;


  // read init W from file: just for RDL
  // for standalone CTR, this part should be deleted
  printf("\nreading W initialization from %s/final-W.dat\n", directory);
  sprintf(name, "%s/final-W.dat", directory);
  FILE * file_init_W = fopen(name, "r");
  mtx_fscanf(file_init_W, m_W);
  fclose(file_init_W);
    
  printf("\nreading etap initialization from %s/final-etap.dat\n", directory);
  sprintf(name, "%s/final-etap.dat", directory);
  FILE * file_init_e = fopen(name, "r");
  vct_fscanf(file_init_e, m_etap);
  fclose(file_init_e);



  while ((iter < param->max_iter and converge > 1e-6 ) or iter < min_iter) {


    likelihood_old = likelihood;
    likelihood = 0.0;
    likelihood_out = 0.0;


    // update W
    for (int kk = 0;kk<1;kk++)// bingo
        for (j = 0; j < m_num_items; j ++) {
          gsl_vector_view w = gsl_matrix_row(m_W, j);
          gsl_vector_view theta_v = gsl_matrix_row(m_theta, j);


          nbh = graph->m_nodes[j];//added//neighborhood
          tnbh = tgraph->m_nodes[j];//added//neighborhood

          optimizew(j,&w.vector,m_W,nbh,tnbh,m_etap,&theta_v.vector,param->lambda_w,param->lambda_l);//added
        }

    // update eta//added
    likelihood -= optimizeetap(graph,tgraph,m_W,m_etap,param->lambda_e,param->lambda_l);

    // regularziation term for eta
    double reta;
    gsl_blas_ddot(m_etap,m_etap,&reta);
    //likelihood += -0.5*param->lambda_e*reta;

    time(&current);
    elapsed = (int)difftime(current, start);

    iter++;
    converge = fabs((likelihood-likelihood_old)/likelihood_old);

    if (likelihood < likelihood_old) printf("likelihood is decreasing!\n");

    fprintf(file, "%04d %06d %10.5f %.10f\n", iter, elapsed, likelihood, converge);
    fflush(file);

    // mine
    const char* respnt = strchr(directory,'p');

    printf("%s: iter=%04d, time=%06d, likelihood=%.5f, converge=%.10f\n", respnt, iter, elapsed, likelihood, converge);

  }

  // save final results
  sprintf(name, "%s/final-etap.dat", directory);
  FILE * file_e = fopen(name, "w");
  vct_fprintf(file_e, m_etap);
  fclose(file_e);

  sprintf(name, "%s/final-W.dat", directory);
  FILE * file_W = fopen(name, "w");
  mtx_fprintf(file_W, m_W);
  fclose(file_W);

  sprintf(name, "%s/final-likelihood.dat", directory);
  FILE * file_L = fopen(name, "w");
  fprintf(file_L,"%f",likelihood);
  fclose(file_L);

  // free memory
  gsl_matrix_free(XX);
  gsl_matrix_free(A);
  gsl_matrix_free(B);
  gsl_vector_free(x);

}

