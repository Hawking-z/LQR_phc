#include "BalanceCalc.h"
#include "math.h"
void lqr_set_matrix_K(LQR* obj,float* matrix_k)
{
    for(uint8_t i = 0;i<12;i++)
    {
        obj->MatrixK[i] = matrix_k[i];
    }
}

void lqr_calc(LQR*obj)
{
    for(uint8_t i =0;i<2;i++)
    {
        obj->Output[i] = 0;
    }
    for (uint8_t i = 0; i <2; i++)
    {
        for (int j = 0; j <6; j++)
        {
            obj->Output[i] += (-obj->MatrixK[i * 6 + j]) * (obj->StateFeedBack[j] - obj->StateRef[j]);
        }
    }
    for(uint8_t i = 0;i<2;i++)
    {
        if(fabsf(obj->Output[i])>obj->Outputmax)
        {
            obj->Output[i] = obj->Outputmax * obj->Output[i] / fabsf(obj->Output[i]);
        }
    }
}
