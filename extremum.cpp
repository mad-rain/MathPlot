
#include <math.h>
#include <assert.h>
#include "extremum.h"


static int ScanFunctionExtremum(TDouble (*Func)(TDouble), TDouble Left, TDouble Right,
                                TDouble Step, TDouble Mult, TDouble Epsilon,
                                TDouble &NewLeft, TDouble &Min)
{
    TDouble y0, y1 = Func(Left) * Mult;
    TDouble x = Left;
    int Iter = 0;
        
    do
    {
        do
        {
            x += Step;
            if (x > Right)
                return 0;

            y0 = y1;
            y1 = Func(x) * Mult;

        } while (y1 < y0);

        if (Iter == 0)
            NewLeft = x;

        Step *= -0.2;
        Iter ++;

    } while (fabs(Step) > Epsilon);

    Min = x;

    return 1;
}

static int ScanFunctionZero(TDouble (*Func)(TDouble), TDouble Left, TDouble Right,
                            TDouble Step, TDouble Epsilon,
                            TDouble &NewLeft, TDouble &Root)
{
    TDouble y0, y1 = Func(Left);
    TDouble x = Left;
    int Iter = 0;

    if (y1 == 0.0)
    {
        Root = Left;
        NewLeft = Left + Step;      
        return 1; 
    }

    do
    {
        do
        {
            x += Step;
            if (x > Right)
                return 0;
            y0 = y1; 
            y1 = Func(x);

        } while (y1 * y0 > 0);

        if (Iter == 0)
            NewLeft = x;

        Step *= -0.2; 
        Iter ++;

    } while (fabs(Step) > Epsilon);

    Root = x;

    return 1;
}

void FunctionZeros(TDouble (*Func)(TDouble), TDouble Left, TDouble Right,
                   TDouble Step, TDouble Epsilon, TKeyPoints &KeyPoints)
{
    TDouble NewLeft, Root;
    int Found;

    if (Left + Step > Right)
        return;
        
    do 
    {
        Found = ScanFunctionZero(Func, Left, Right, Step, Epsilon, NewLeft, Root);

        if (Found)
            KeyPoints.Insert(Root);

        assert(NewLeft >= Root);

        Left = NewLeft;

    } while (Left < Right && Found);
}

void FunctionMinMax(TDouble (*Func)(TDouble), TDouble Left, TDouble Right,
                    TDouble Step, TDouble Epsilon, int pMin, int pMax, TKeyPoints &KeyPoints)
{
    TDouble NewLeft, Min;
    int Found;
    TDouble Mult = -1.0;

    if (Left + Step > Right)
        return;
        
    Mult = (Func(Left) < Func(Left + Step)) ? -1.0 : 1.0;
        
    do 
    {
        Found = ScanFunctionExtremum(Func, Left, Right, Step, Mult, Epsilon, NewLeft, Min);

        if (Found)
            if ((Mult > 0 && pMin) || (Mult < 0 && pMax))
                KeyPoints.Insert(Min);

        assert(NewLeft >= Min);

        Mult = -Mult;
        Left = NewLeft;

    } while (Left < Right && Found);
}
