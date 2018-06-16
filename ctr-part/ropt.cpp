#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include "graph.h"
#include "ropt.h"
#include "utils.h"
#include "gsl/gsl_sf_log.h"
#include <math.h>
#include "utdb.h"

void optimizew(int j,gsl_vector* w, gsl_matrix* m_W,c_neighborhood* nbh,c_neighborhood* tnbh,gsl_vector *etap, gsl_vector* v, double lambda_w, double lambda_l){
    // if not rated, ignore the last term of the likelihood
    // during the computation of likelihood and gradient

    // calculate negation of likelihood w.r.t w
    double f_old = f_likeli(j,w,m_W,nbh,tnbh,etap,v,lambda_w,lambda_l);
    size_t size = w->size;
    gsl_vector* g = gsl_vector_alloc(size); //gradient
    gsl_vector* w_old = gsl_vector_alloc(size);
    gsl_vector_memcpy(w_old,w);
    // calculate gradient
    // ignore the last term if not rated
    // calculate the gradient of the negative log-likelihood
    df_likeli(j,w,m_W,nbh,tnbh,etap,lambda_w,lambda_l,v,g);
    // rescale the gradient, don't know if necessary
    /*double ab_sum = gsl_blas_dasum(g);
    if (ab_sum>1.0) gsl_vector_scale(g,1.0/ab_sum);*/

    //double r = 0;
    //gsl_blas_ddot(g,g,&r);
    //r *= -0.5;

    double beta = 0.5;
    double f_new;
    double t = 1; // default: 1
    int iter = 0;
    //printf("%f old bingo\n",f_old);
    while(++iter<100){
        gsl_vector_memcpy(w,w_old);
        gsl_blas_daxpy(-1*t,g,w);

        // calculate the negative log-likelihood
        f_new = f_likeli(j,w,m_W,nbh,tnbh,etap,v,lambda_w,lambda_l);
        if (f_new>f_old || !gsl_finite(f_new)) t = t*beta;
        else break;
    }
    if(f_new>f_old)
        printf("%f new bingo\n",f_new);

    gsl_vector_free(g);
    gsl_vector_free(w_old);
}

double optimizeetap(const c_graph *graph,const c_graph *tgraph,gsl_matrix* m_W,gsl_vector* etap,double lambda_e,double lambda_l){
    size_t size = etap->size;
    gsl_vector* g = gsl_vector_calloc(size);
    gsl_vector* etap_old = gsl_vector_alloc(size);
    double f_old = f_likeli_etap(graph,tgraph,m_W,etap,lambda_e,lambda_l);

    // calculate the gradient of negative likelihood with respect to etap
    df_likeli_etap(graph,tgraph,m_W,etap,lambda_e,lambda_l,g);
    // rescale the gradient, don't know if necessary
    //double ab_sum = gsl_blas_dasum(g);
    //if (ab_sum>1.0) gsl_vector_scale(g,1.0/ab_sum);

    double r = 0;
    gsl_blas_ddot(g,g,&r);
    r *= -0.5;

    double beta = 0.5;
    double f_new;
    double t = 1; // default: 1
    int iter = 0;
    gsl_vector_memcpy(etap_old,etap);
    //printf("%f old bingo\n",f_old);
    //printf("bingo old\n");
    //printv(etap,1,10);
    while(++iter<100){
        gsl_vector_memcpy(etap,etap_old);
        gsl_blas_daxpy(-1*t,g,etap);

        // calculate the negative log-likelihood with respect to etap
        f_new = f_likeli_etap(graph,tgraph,m_W,etap,lambda_e,lambda_l);
        //if (f_new>f_old+r*t) t = t*beta;
        if (f_new>f_old || !gsl_finite(f_new)) t = t*beta;
        else break;
    }
    if(f_new>f_old)
        printf("%f new bingo\n",f_new);
    if(!gsl_finite(f_new)) printf("bingo nan\n");
    //printf("%f new bingo\n",f_new);
    //printf("bingo new\n");
    //printv(etap,1,10);

    gsl_vector_free(g);
    gsl_vector_free(etap_old);
    return f_new;

}

// get the negation of the likilihood regarding w
double f_likeli(int j,gsl_vector* w,gsl_matrix* m_W,c_neighborhood* nbh,c_neighborhood* tnbh,gsl_vector *etap, gsl_vector* v, double lambda_w, double lambda_l){
    double f;
    //double term_l = lambda_l*linkll(j,nbh,m_W,etap); // term link
    double term_l = linkll(j,nbh,tnbh,lambda_l,m_W,etap); // term link
    double term_n;  // term norm
    // calculate term_n
    size_t size = w->size;
    gsl_vector* tmpvt = gsl_vector_alloc(size);
    gsl_vector_memcpy(tmpvt,w);
    gsl_vector_sub(tmpvt,v);
    gsl_blas_ddot(tmpvt,tmpvt,&term_n);
    term_n *= -0.5*lambda_w;
    f = term_l+term_n;
    gsl_vector_free(tmpvt);
    return -f;
}

// calculate the negation of the gradient of the likilihood
void df_likeli(int j,gsl_vector* w,gsl_matrix* m_W,c_neighborhood* nbh,c_neighborhood* tnbh,gsl_vector *etap,double lambda_w,double lambda_l,gsl_vector* v,gsl_vector* g){
    size_t size = w->size;
    int numn = nbh->m_length;
    int i;
    //g = gsl_vector_calloc(size);
    gsl_vector* w_12 = gsl_vector_alloc(size);
    gsl_vector* w_12p = gsl_vector_alloc(size+1);
    gsl_vector_view ww;// subvector of v_12p
    gsl_vector_const_view eta = gsl_vector_const_subvector(etap,0,size);
    gsl_vector* tmpvt = gsl_vector_alloc(size);
    double tmp,tmp2;
    // term_l: term link, positive link part
    gsl_vector* term_l = gsl_vector_calloc(size);
    for (i = 0;i<numn;i++){
         gsl_vector_const_view w_2 = gsl_matrix_const_row(m_W,nbh->m_nbs[i]);
         gsl_vector_memcpy(w_12,w);
         gsl_vector_mul(w_12,&w_2.vector);
         ww = gsl_vector_subvector(w_12p,0,size);
         gsl_vector_memcpy(&ww.vector,w_12);
         gsl_vector_set(w_12p,size,1);
         gsl_blas_ddot(w_12p,etap,&tmp);
         tmp = 1-sigmoid(tmp);
         gsl_vector_memcpy(tmpvt,&eta.vector);
         gsl_vector_mul(tmpvt,&w_2.vector);
         gsl_vector_scale(tmpvt,tmp);
         gsl_vector_add(term_l,tmpvt);        
    }
    gsl_vector_scale(term_l,lambda_l);
    // term_l: term link, negative link part
    //numn = tnbh->m_length;
    //for (i = 0;i<numn;i++){
    //     gsl_vector_const_view w_2 = gsl_matrix_const_row(m_W,tnbh->m_nbs[i]);
    //     gsl_vector_memcpy(w_12,w);
    //     gsl_vector_mul(w_12,&w_2.vector);
    //     ww = gsl_vector_subvector(w_12p,0,size);
    //     gsl_vector_memcpy(&ww.vector,w_12);
    //     gsl_vector_set(w_12p,size,1);
    //     gsl_blas_ddot(w_12p,etap,&tmp);
    //     tmp = sigmoid(tmp);
    //     tmp2 = pow(tmp,lambda_l);
    //     tmp = lambda_l*tmp2/(1-tmp2)*(1-tmp);
    //     gsl_vector_memcpy(tmpvt,&eta.vector);
    //     gsl_vector_mul(tmpvt,&w_2.vector);
    //     gsl_vector_scale(tmpvt,tmp);
    //     gsl_vector_add(term_l,tmpvt);        
    //}
    // term_n: term norm
    gsl_vector* term_n = gsl_vector_alloc(size);
    gsl_vector_memcpy(term_n,w);
    gsl_vector_sub(term_n,v);
    gsl_vector_scale(term_n,-lambda_w);

    gsl_vector_add(g,term_l);
    gsl_vector_add(g,term_n);
    
    // negation
    gsl_vector_scale(g,-1);

    gsl_vector_free(tmpvt);
    gsl_vector_free(term_n);
    gsl_vector_free(term_l);  
    gsl_vector_free(w_12);
    gsl_vector_free(w_12p);
}


double f_likeli_etap(const c_graph *graph,const c_graph *tgraph,gsl_matrix* m_W,gsl_vector* etap,double lambda_e,double lambda_l){
    size_t sizep = etap->size;
    gsl_vector* w_12p = gsl_vector_alloc(sizep);
    gsl_vector* t_12p = gsl_vector_alloc(sizep);

    double term_l = 0;
    double res;
    int i,j,numnode,numnb,w_2,w_1;
    numnode = graph->m_num_node;
    c_neighborhood* nbhp; // pointer of neighborhood
    // link term-l, positive link part
    for(i = 0;i<numnode;i++){
        w_1 = i;
        nbhp = graph->m_nodes[i];
        numnb = nbhp->m_length;
        for(j = 0;j<numnb;j++){
            w_2 = nbhp->m_nbs[j];
            gsl_vector_const_view ww_1 = gsl_matrix_const_row(m_W,w_1);
            gsl_vector_const_view ww_2 = gsl_matrix_const_row(m_W,w_2);
            gsl_vector_view ww_12 = gsl_vector_subvector(w_12p,0,sizep-1);
            gsl_vector_memcpy(&ww_12.vector,&ww_1.vector);
            gsl_vector_mul(&ww_12.vector,&ww_2.vector);
            gsl_vector_set(w_12p,sizep-1,1);
            gsl_blas_ddot(w_12p,etap,&res);
            res = sigmoid(res);
            term_l += log(res); 
        }
    }
    term_l *= lambda_l;
    // link term-l, negative link part
    //numnode = tgraph->m_num_node;
    //for(i = 0;i<numnode;i++){
    //    w_1 = i;
    //    nbhp = tgraph->m_nodes[i];
    //    numnb = nbhp->m_length;
    //    for(j = 0;j<numnb;j++){
    //        w_2 = nbhp->m_nbs[j];
    //        gsl_vector_const_view ww_1 = gsl_matrix_const_row(m_W,w_1);
    //        gsl_vector_const_view ww_2 = gsl_matrix_const_row(m_W,w_2);
    //        gsl_vector_view ww_12 = gsl_vector_subvector(w_12p,0,sizep-1);
    //        gsl_vector_memcpy(&ww_12.vector,&ww_1.vector);
    //        gsl_vector_mul(&ww_12.vector,&ww_2.vector);
    //        gsl_vector_set(w_12p,sizep-1,1);
    //        gsl_blas_ddot(w_12p,etap,&res);
    //        // exact
    //        res = sigmoid(res);
    //        term_l += log(1-pow(res,lambda_l)); 
    //    }
    //}


    //term_l = lambda_l*term_l+lambda_t*term_t;
    // regularization term
    double reta;
    gsl_blas_ddot(etap,etap,&reta);
    term_l += -0.5*lambda_e*reta;

    gsl_vector_free(w_12p);
    gsl_vector_free(t_12p);
    // return negation
    return -term_l;
}

void df_likeli_etap(const c_graph *graph,const c_graph *tgraph,gsl_matrix* m_W,gsl_vector* etap,double lambda_e,double lambda_l,gsl_vector* g){
    
    size_t sizep = etap->size;
    gsl_vector* tmpvt = gsl_vector_alloc(sizep);
    gsl_vector* w_12p = gsl_vector_alloc(sizep);
    gsl_vector_view ww_12;  // first sizep-1 elements of w_12p

    double res,res2;
    int i,j,numnode,numnb,w_2,w_1;
    numnode = graph->m_num_node;
    c_neighborhood* nbhp; // pointer of neighborhood
    // link term-l, positive link part
    for(i = 0;i<numnode;i++){
        w_1 = i;
        nbhp = graph->m_nodes[i];
        numnb = nbhp->m_length;
        for(j = 0;j<numnb;j++){
            w_2 = nbhp->m_nbs[j];
            gsl_vector_const_view ww_1 = gsl_matrix_const_row(m_W,w_1);
            gsl_vector_const_view ww_2 = gsl_matrix_const_row(m_W,w_2);
            ww_12 = gsl_vector_subvector(w_12p,0,sizep-1);
            gsl_vector_memcpy(&ww_12.vector,&ww_1.vector);
            gsl_vector_mul(&ww_12.vector,&ww_2.vector);
            gsl_vector_set(w_12p,sizep-1,1);
            gsl_blas_ddot(w_12p,etap,&res);
            res = 1-sigmoid(res);
            gsl_vector_memcpy(tmpvt,w_12p);
            gsl_vector_scale(tmpvt,res*lambda_l);
            gsl_vector_sub(g,tmpvt);
        }
    }
    // link term-l, negative link part
    //numnode = tgraph->m_num_node;
    //for(i = 0;i<numnode;i++){
    //    w_1 = i;
    //    nbhp = tgraph->m_nodes[i];
    //    numnb = nbhp->m_length;
    //    for(j = 0;j<numnb;j++){
    //        w_2 = nbhp->m_nbs[j];
    //        gsl_vector_const_view ww_1 = gsl_matrix_const_row(m_W,w_1);
    //        gsl_vector_const_view ww_2 = gsl_matrix_const_row(m_W,w_2);
    //        ww_12 = gsl_vector_subvector(w_12p,0,sizep-1);
    //        gsl_vector_memcpy(&ww_12.vector,&ww_1.vector);
    //        gsl_vector_mul(&ww_12.vector,&ww_2.vector);
    //        gsl_vector_set(w_12p,sizep-1,1);
    //        gsl_blas_ddot(w_12p,etap,&res);

    //        // exact
    //        res = sigmoid(res);
    //        res2 = pow(res,lambda_l);
    //        res = lambda_l*res2/(1-res2)*(1-res);
    //        // approximate
    //        //res = -lambda_l*(1-sigmoid(res));
    //        //printf("bingo res %f\n", res);

    //        gsl_vector_memcpy(tmpvt,w_12p);
    //        gsl_vector_scale(tmpvt,res);
    //        gsl_vector_sub(g,tmpvt);
    //    }
    //}

    // regularization term
    gsl_vector_memcpy(tmpvt,etap);
    gsl_vector_scale(tmpvt,lambda_e);
    gsl_vector_add(g,tmpvt);

    gsl_vector_free(tmpvt);
    gsl_vector_free(w_12p);

}

double sigmoid(double x){
    double tmp;
    tmp = 1+exp(-x);
    tmp = 1.0/tmp;
    return tmp;
}

double linkll(int j,c_neighborhood* nbh,c_neighborhood* tnbh,double lambda_l,gsl_matrix* m_W,gsl_vector* etap){
    size_t size = etap->size-1;
    gsl_vector* w_12p = gsl_vector_alloc(size+1);
    int w_2,i,nbn;
    nbn = nbh->m_length;
    gsl_vector_const_view ww_1 = gsl_matrix_const_row(m_W,j);
    double res;
    double ll = 0;
    //if(j==0) printf("bingo len: %d\n",nbn);
    // cal' the positive links
    for(i = 0;i<nbn;i++){
        // first size elements of v_12p
        gsl_vector_view ww_12 = gsl_vector_subvector(w_12p,0,size);
        w_2 = nbh->m_nbs[i];
        gsl_vector_const_view ww_2 = gsl_matrix_const_row(m_W,w_2);
        gsl_vector_memcpy(&ww_12.vector,&ww_1.vector);
        gsl_vector_mul(&ww_12.vector,&ww_2.vector);
        gsl_vector_set(w_12p,size,1);
        gsl_blas_ddot(w_12p,etap,&res);
        //if(j==0) printf("bingo res: %f\n",res);
        res = sigmoid(res);
        ll += log(res); 
    }
    ll *= lambda_l;
    // cal' the negative links
    //nbn = tnbh->m_length; // reuse nbn
    //for(i = 0;i<nbn;i++){
    //    // first size elements of v_12p
    //    gsl_vector_view ww_12 = gsl_vector_subvector(w_12p,0,size);
    //    w_2 = tnbh->m_nbs[i];
    //    gsl_vector_const_view ww_2 = gsl_matrix_const_row(m_W,w_2);
    //    gsl_vector_memcpy(&ww_12.vector,&ww_1.vector);
    //    gsl_vector_mul(&ww_12.vector,&ww_2.vector);
    //    gsl_vector_set(w_12p,size,1);
    //    gsl_blas_ddot(w_12p,etap,&res);
    //    //if(j==0) printf("bingo res: %f\n",res);
    //    res = sigmoid(res);
    //    ll += log(1-pow(res,lambda_l)); 
    //}

    gsl_vector_free(w_12p);

    return ll;
}
