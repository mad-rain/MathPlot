/*                                                                   */
/* Warning!!! This file is auto-generated by Maple, DON'T EDIT IT!!! */
/*                                                                   */

#include <math.h>
#include "func.h"


TLongDouble Function_j2(TLongDouble A,TLongDouble m,TLongDouble k,TLongDouble omega,TLongDouble phi,TLongDouble C,TLongDouble T)
{
  TLongDouble t1;
  TLongDouble t100;                         
  TLongDouble t109;
  TLongDouble t118;
  TLongDouble t119;
  TLongDouble t12;
  TLongDouble t126;
  TLongDouble t13;
  TLongDouble t130;
  TLongDouble t131;
  TLongDouble t133;
  TLongDouble t136;
  TLongDouble t137;
  TLongDouble t139;
  TLongDouble t14;
  TLongDouble t140;
  TLongDouble t141;
  TLongDouble t145;
  TLongDouble t146;
  TLongDouble t147;
  TLongDouble t148;
  TLongDouble t149;
  TLongDouble t151;
  TLongDouble t153;
  TLongDouble t159;
  TLongDouble t16;
  TLongDouble t188;
  TLongDouble t189;
  TLongDouble t19;
  TLongDouble t192;
  TLongDouble t2;
  TLongDouble t200;
  TLongDouble t21;
  TLongDouble t23;
  TLongDouble t240;
  TLongDouble t243;
  TLongDouble t248;
  TLongDouble t25;
  TLongDouble t27;
  TLongDouble t272;
  TLongDouble t28;
  TLongDouble t29;
  TLongDouble t3;
  TLongDouble t35;
  TLongDouble t37;
  TLongDouble t4;
  TLongDouble t41;
  TLongDouble t42;
  TLongDouble t43;
  TLongDouble t44;
  TLongDouble t45;
  TLongDouble t47;
  TLongDouble t48;
  TLongDouble t5;
  TLongDouble t51;
  TLongDouble t53;
  TLongDouble t54;
  TLongDouble t56;
  TLongDouble t57;
  TLongDouble t59;
  TLongDouble t6;
  TLongDouble t63;
  TLongDouble t7;
  TLongDouble t70;
  TLongDouble t74;
  TLongDouble t75;
  TLongDouble t77;
  TLongDouble t8;
  TLongDouble t81;
  TLongDouble t84;
  TLongDouble t85;
  TLongDouble t88;
  TLongDouble t9;
  TLongDouble t90;
  TLongDouble t93;
  TLongDouble t94;
  TLongDouble t99;
  {
    t1 = A*A;
    t2 = -T+C;
    t3 = t2*k;
    t4 = t3+m;
    t5 = k*k;
    t6 = omega*omega;
    t7 = m*m;
    t8 = t6*t7;
    t9 = t5+t8;
    t12 = k/m;
    t13 = t12*C;
    t14 = exp(-t13);
    t16 = t7*m;
    t19 = exp(-t12*T);
    t21 = t4*t9*t14-t6*t16*t19;
    t23 = t5*t5;
    t25 = omega*T;
    t27 = omega*C;
    t28 = t25+2.0*phi+t27;
    t29 = cos(t28);
    t35 = t5*k;
    t37 = sin(t28);
    t41 = t7*t7;
    t42 = t41*m;
    t43 = t5-t8;
    t44 = cos(phi);
    t45 = t44*t44;
    t47 = m*omega;
    t48 = sin(phi);
    t51 = t47*t48*t44*k;
    t53 = t8/2.0;
    t54 = t5/2.0;
    t56 = cos(t27);
    t57 = t56*t56;
    t59 = sin(t27);
    t63 = -t43;
    t70 = t63/2.0;
    t74 = t6*t6;
    t75 = t74*omega;
    t77 = exp(2.0*t13);
    t81 = t9*t9;
    t84 = C*C;
    t85 = T*T;
    t88 = (-T*C+t84+t85/3.0)*t5;
    t90 = -T/2.0+C;
    t93 = 2.0*t90*m*k;
    t94 = t88+t93+t7;
    t99 = 2.0*t27+2.0*phi;
    t100 = cos(t99);
    t109 = sin(t99);
    t118 = t25-t27;
    t119 = sin(t118);
    t126 = cos(t118);
    t130 = t23*t5;
    t131 = t23*t6;
    t133 = -t130+t131*t7;
    t136 = 2.0*t25+2.0*phi;
    t137 = sin(t136);
    t139 = t23*k;
    t140 = m*t139;
    t141 = cos(t136);
    t145 = m*t6;
    t146 = t145*C;
    t147 = -t146+k;
    t148 = t147*t45;
    t149 = t48*omega;
    t151 = m+k*C;
    t153 = t149*t151*t44;
    t159 = omega*t151;
    t188 = t6*omega;
    t189 = exp(t13);
    t192 = k*t151;
    t200 = t9*t5;
    t240 = t81*T*(t94*t6-t5)*omega*t100/8.0-T*t6*(t90*k+m)*t81*k*t109/4.0+(t4*m
*t6+2.0*t5)*t9*t5*t119/4.0+t9*(t3-m)*t35*omega*t126/4.0+t133*t137/16.0+t140*
t141*omega/8.0+(t9*(t148+t153+t146/2.0-k/2.0)*t57+((t159*t45-t48*t147*t44-t159/
2.0)*t9*t59-t35*t45/2.0-t47*t5*t48*t44/2.0)*t56+(-t47*t5*t45/2.0+t35*t48*t44/
2.0)*t59-t9*(t148+t153-k)/2.0)*t16*t188*t189+(t192*t45+m*t48*t159*t44-t5*C)*
omega*t200*t56/2.0-t200*(-t145*t151*t45+t149*t192*t44-t5)*t59/2.0-t139*omega*m*
t45/4.0-t133*t48*t44/8.0-(T*t41*t94*t74*t6+2.0*T*t5*t7*(t88+t93+3.0/2.0*t7)*t74
+T*(t88+t93+4.0*t7)*t131-t140+2.0*t130*T)*omega/8.0;
    t243 = t14*t14;
    t248 = m*t2*t6;
    t272 = t19*t19;
    return(-4.0*t1*(-omega*t21*t14*t23*t29/4.0-t6*t21*t14*m*t35*t37/4.0+(-t42*(
(t43*t45+2.0*t51+t53-t54)*t57+t59*(2.0*t47*t45*k+t63*t48*t44-t47*k)*t56+t70*t45
-t51+t54)*t75*t77/4.0+k*t240)*t243-k*t16*t19*(-t9*(t248-k)*t100+t4*omega*t9*
t109+omega*t5*m*t119-t35*t126+t9*(t248+k))*t188*t14/4.0+(-t70*t100+t53+t47*t109
*k+t54)*t42*t272*t75/8.0)/t7/t81/t75/t35/t243);
  }
}

TLongDouble Function_x0(TLongDouble t,TLongDouble A,TLongDouble m,TLongDouble omega,TLongDouble phi,TLongDouble C)
{
  TLongDouble t1;
  TLongDouble t12;
  TLongDouble t13;
  TLongDouble t17;
  TLongDouble t2;
  TLongDouble t3;
  TLongDouble t4;
  TLongDouble t7;
  {
    t1 = 1/m;
    t2 = t1*A;
    t3 = omega*omega;
    t4 = 1/t3;
    t7 = cos(omega*t+phi);
    t12 = omega*C+phi;
    t13 = sin(t12);
    t17 = cos(t12);
    return(-t2*t4*t7-t2/omega*t13*t+A*(t17+omega*t13*C)*t1*t4);
  }
}

TLongDouble Function_xk(TLongDouble t,TLongDouble A,TLongDouble m,TLongDouble k,TLongDouble omega,TLongDouble phi,TLongDouble C)
{
  TLongDouble t1;
  TLongDouble t12;
  TLongDouble t15;
  TLongDouble t17;
  TLongDouble t19;
  TLongDouble t2;
  TLongDouble t20;
  TLongDouble t24;
  TLongDouble t3;
  TLongDouble t32;
  TLongDouble t37;
  TLongDouble t39;
  TLongDouble t40;
  TLongDouble t44;
  TLongDouble t6;
  TLongDouble t8;
  TLongDouble t9;
  {
    t1 = k*k;
    t2 = omega*omega;
    t3 = m*m;
    t6 = 1/(t1+t2*t3);
    t8 = omega*C+phi;
    t9 = cos(t8);
    t12 = sin(t8);
    t15 = A*(k*t9+m*omega*t12);
    t17 = k/m;
    t19 = exp(-t17*C);
    t20 = 1/t19;
    t24 = exp(-t17*t);
    t32 = 1/k;
    t37 = 1/omega;
    t39 = omega*t+phi;
    t40 = sin(t39);
    t44 = cos(t39);
    return(t6*(t15*t20*t6*k*m*t24+t15*t20*t6*t2*t3*m*t32*t24+A*k*t37*t40-A*m*
t44)-A*t12*t32*t37);
  }
}

TLongDouble Function_xk_Sub_x0(TLongDouble t,TLongDouble A,TLongDouble m,TLongDouble k,TLongDouble omega,TLongDouble phi,TLongDouble C)
{
  TLongDouble t1;
  TLongDouble t12;
  TLongDouble t15;
  TLongDouble t16;
  TLongDouble t17;
  TLongDouble t19;
  TLongDouble t2;
  TLongDouble t20;
  TLongDouble t24;
  TLongDouble t3;
  TLongDouble t32;
  TLongDouble t37;
  TLongDouble t39;
  TLongDouble t40;
  TLongDouble t44;
  TLongDouble t51;
  TLongDouble t52;
  TLongDouble t6;
  TLongDouble t8;
  TLongDouble t9;
  {
    t1 = k*k;
    t2 = omega*omega;
    t3 = m*m;
    t6 = 1/(t1+t2*t3);
    t8 = omega*C+phi;
    t9 = cos(t8);
    t12 = sin(t8);
    t15 = A*(k*t9+m*omega*t12);
    t16 = 1/m;
    t17 = k*t16;
    t19 = exp(-t17*C);
    t20 = 1/t19;
    t24 = exp(-t17*t);
    t32 = 1/k;
    t37 = 1/omega;
    t39 = omega*t+phi;
    t40 = sin(t39);
    t44 = cos(t39);
    t51 = t16*A;
    t52 = 1/t2;
    return(t6*(t15*t20*t6*k*m*t24+t15*t20*t6*t2*t3*m*t32*t24+A*k*t37*t40-A*m*
t44)-A*t12*t32*t37+t51*t52*t44+t51*t37*t12*t-A*(t9+omega*t12*C)*t16*t52);
  }
}
