#ifndef _EXTREMUM_H_INCLUDED
#define _EXTREMUM_H_INCLUDED


#include "types.h"
#include "key_pnt.h"


void FunctionMinMax(TDouble (*)(TDouble), TDouble, TDouble,
                    TDouble, TDouble, int, int, TKeyPoints &);
void FunctionZeros(TDouble (*)(TDouble), TDouble, TDouble,
                   TDouble, TDouble, TKeyPoints &);


#endif /* _EXTREMUM_H_INCLUDED */
