#include "utdb.h"

void printv(gsl_vector* vt,int begin,int end){
    size_t size = vt->size;
    for (size_t i = 0;i<size;i++){
        if(int(i)>=begin && int(i)<=end)
            printf("%f ",gsl_vector_get(vt,i));
    }
    printf("\n");
}
