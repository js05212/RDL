#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "ctr.h"
#include "graph.h"

gsl_rng * RANDOM_NUMBER = NULL;

void print_usage_and_exit() {
  // print usage information
  printf("**************************relational collaborative topic models for recommendations************************\n");
  printf("usage:\n");
  printf("      ctr [options]\n");
  printf("      --help:           print help information\n");

  printf("\n");
  printf("      --directory:      save directory, required\n");

  printf("\n");
  printf("      --graph:          graph file, required\n");//added
  printf("      --tgraph:         tag graph file, required\n");//added
  printf("      --a:              positive item weight, default 1\n");
  printf("      --b:              negative item weight, default 0.01 (b < a)\n");
  printf("      --lambda_w:       item vector regularizer, default xxx\n");
  printf("      --lambda_t:       item vector regularizer, default xxx\n");
  printf("      --lambda_e:       item vector regularizer, default xxx\n");
  printf("      --lambda_l:       item vector regularizer, default xxx\n");
  printf("\n");

  printf("      --random_seed:    the random seed, default from the current time\n");
  printf("      --save_lag:       the saving lag, default 20 (-1 means no savings for intermediate results)\n");
  printf("      --max_iter:       the max number of iterations, default 200\n");
  printf("\n");

  printf("      --num_factors:    the number of factors, default 200\n");
  printf("      --num_items:    the number of items, default 200\n");
  printf("      --mult:           mult file, in lda-c format, optional, if not provided, it's the matrix factorization\n");
  printf("      --theta_init:     topic proportions file from lda, optional (required if mult file is provided)\n");
  printf("      --beta_init:      topic distributions file from lda, optional (required if mult file is provided)\n");
  printf("      --theta_opt:      optimize theta or not, optional, default not\n");

  printf("*******************************************************************************************************\n");

  exit(0);
}

int main(int argc, char* argv[]) {
  if (argc < 2) print_usage_and_exit();

  char filename[500];
  int theta_opt = 0;

  const char* const short_options = "hd:x:i:g:f:a:b:u:v:w:j:c:l:r:s:m:k:t:e:y";//added
  const struct option long_options[] = {
    {"help",        no_argument,       NULL, 'h'},
    {"directory",   required_argument, NULL, 'd'},
    {"graph",       required_argument, NULL, 'g'},//added
    {"tgraph",      required_argument, NULL, 'f'},//added
    {"a",           required_argument, NULL, 'a'},
    {"b",           required_argument, NULL, 'b'},
    {"lambda_w",    required_argument, NULL, 'w'},
    {"lambda_t",    required_argument, NULL, 'j'},
    {"lambda_e",    required_argument, NULL, 'c'},
    {"lambda_l",    required_argument, NULL, 'l'},
    {"random_seed", required_argument, NULL, 'r'},
    {"save_lag",    required_argument, NULL, 's'},
    {"max_iter",    required_argument, NULL, 'm'},
    {"num_factors", required_argument, NULL, 'k'},
    {"num_items",   required_argument, NULL, 'v'},
    {"mult",        required_argument, NULL, 't'},
    {"theta_init",  required_argument, NULL, 'e'},
    {"beta_init",   required_argument, NULL, 'y'},
    {"theta_opt",   no_argument, &theta_opt, 1},
    {NULL, 0, NULL, 0}};

  char*  directory = NULL;

  char*  graph_path = NULL;//added
  char*  tgraph_path = NULL;//added
  double a = 1.0;
  double b = 0.01;
  double lambda_w = 0.01;
  double lambda_t = 0.01;
  double lambda_e = 100;
  double lambda_l = 100;

  time_t t; time(&t);
  long   random_seed = (long) t;
  int    save_lag = 20;
  int    max_iter = 200;

  int    num_factors = 200;
  int    num_items = 200;
  char*  mult_path = NULL;
  char*  theta_init_path = NULL;
  char*  beta_init_path = NULL;

  int cc = 0; 
  while(true) {
    cc = getopt_long(argc, argv, short_options, long_options, NULL);
    switch(cc) {
      case 'h':
        print_usage_and_exit();
        break;
      case 'd':
        directory = optarg;
        break;
      case 'g'://added
        graph_path = optarg;
        break;
      case 'f'://added
        tgraph_path = optarg;
        break;
      case 'a':
        a = atof(optarg);
        break;
      case 'b':
        b = atof(optarg);
        break;
      case 'w':
        lambda_w = atof(optarg);
        break;
      case 'j':
        lambda_t = atof(optarg);
        break;
      case 'c':
        lambda_e = atof(optarg);
        break;
      case 'l':
        lambda_l = atof(optarg);
        break;
      case 'r':
        random_seed = atoi(optarg);
        break;
      case 's':
        save_lag = atoi(optarg);
        break;
      case 'm':
        max_iter =  atoi(optarg);
        break;    
      case 'k':
        num_factors = atoi(optarg);
        break;
      case 'v':
        num_items = atoi(optarg);
        break;
      case 't':
        mult_path = optarg;
        break;
      case 'e':
        theta_init_path = optarg;
        break;
      case 'y':
        beta_init_path = optarg;
        break;
      case -1:
        break;
      case '?':
        print_usage_and_exit();
        break;
      default:
        break;
    }
    if (cc == -1)
      break;
  }

  /// print information
  printf("\n************************************************************************************************\n");
  
  if (!dir_exists(directory)) make_directory(directory);
  printf("result directory: %s\n", directory);


  if (!file_exists(graph_path)) {
    printf("graph file %s doesn't exist! quit ...\n", graph_path);
    exit(-1);
  }
  printf("graph file: %s\n",graph_path);

  if (!file_exists(tgraph_path)) {
    printf("tag graph file %s doesn't exist! quit ...\n", tgraph_path);
    exit(-1);
  }
  printf("tag graph file: %s\n",tgraph_path);

  printf("a: %.4f\n", a);
  printf("b: %.4f\n", b);
  printf("lambda_w: %.4f\n", lambda_w);
  printf("lambda_t: %.4f\n", lambda_t);
  printf("lambda_e: %.4f\n", lambda_e);
  printf("lambda_l: %.4f\n", lambda_l);
  printf("random seed: %d\n", (int)random_seed);
  printf("save lag: %d\n", save_lag);
  printf("max iter: %d\n", max_iter);
  printf("number of factors: %d\n", num_factors);
  printf("number of items: %d\n", num_items);

  if (theta_init_path == NULL) {
    printf("topic proportions file must be provided ...\n");
    exit(-1);
  }
  if (!file_exists(theta_init_path)) {
    printf("topic proportions file %s doesn't exist! quit ...\n", theta_init_path);
    exit(-1);
  }
  printf("topic proportions file: %s\n", theta_init_path);

  printf("\n");

  /// save the settings
  int ctr_run = 1;
  if (mult_path == NULL) ctr_run = 0;
  ctr_hyperparameter ctr_param;
  ctr_param.set(a, b, lambda_w, lambda_t, lambda_e, lambda_l, random_seed, max_iter, save_lag, theta_opt, ctr_run);
  sprintf(filename, "%s/settings.txt", directory); 
  ctr_param.save(filename);
  
  /// init random numbe generator
  RANDOM_NUMBER = new_random_number_generator(random_seed);

  // read graph //added
  printf("reading graph matrix from %s ...\n", graph_path);
  c_graph* graph = new c_graph();
  graph->read_data(graph_path);

  // read tag graph //added
  printf("reading tag graph matrix from %s ...\n", tgraph_path);
  c_graph* tgraph = new c_graph();
  tgraph->read_data(tgraph_path);

  // create model instance
  c_ctr* ctr = new c_ctr();
  ctr->set_model_parameters(num_factors, num_items);

  ctr->read_init_information(theta_init_path);

  ctr->learn_map_estimate(graph, tgraph, &ctr_param, directory);

  free_random_number_generator(RANDOM_NUMBER);

  delete ctr;
  delete graph;
  delete tgraph;
  return 0;
}
