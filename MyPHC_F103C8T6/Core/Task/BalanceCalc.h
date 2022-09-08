#ifndef __H_BALANCECALC_H__
#define __H_BALANCECALC_H__

#include "stdint.h"

typedef struct LQR_T
{
    float MatrixK[12];
    float StateFeedBack[6];
    float StateRef[6];
    float Output[2];
    float Outputmax;
}LQR;

void lqr_set_matrix_K(LQR* obj,float* matrix_k);
void lqr_calc(LQR*obj);

#endif