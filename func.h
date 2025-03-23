#ifndef _FUNC_H_INCLUDED
#define _FUNC_H_INCLUDED


#include "types.h"


TLongDouble Function_j2(TLongDouble A,TLongDouble m,TLongDouble k,TLongDouble omega,TLongDouble phi,TLongDouble C, TLongDouble T);
TLongDouble Function_x0(TLongDouble t,TLongDouble A,TLongDouble m,TLongDouble omega,TLongDouble phi,TLongDouble C);
TLongDouble Function_xk(TLongDouble t,TLongDouble A,TLongDouble m,TLongDouble k,TLongDouble omega,TLongDouble phi,TLongDouble C);
TLongDouble Function_xk_Sub_x0(TLongDouble t,TLongDouble A,TLongDouble m,TLongDouble k,TLongDouble omega,TLongDouble phi,TLongDouble C);


#endif /* _FUNC_H_INCLUDED */
