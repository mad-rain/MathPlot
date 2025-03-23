#ifndef _KEY_POINTS_H_INCLUDED
#define _KEY_POINTS_H_INCLUDED


#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"


class TKeyPoints {

    int MaxCount;
    TDouble *List;

    void ExpandList(int Index);

public:

    int Count;

    TDouble operator[](int Index)
    {
        return List[Index];
    }

    void Abs()
    {
        for (int i = 0; i < Count; i ++)
            List[i] = fabs(List[i]);
    }

    void Clear()
    {
        delete List;
        List = NULL;
        Count = 0;
        MaxCount = 0;
    }

    void Function(TDouble (*Func)(TDouble))
    {
        for (int i = 0; i < Count; i ++)
            List[i] = Func(List[i]);
    }
    
    void Function(TKeyPoints &p, TDouble (*Func)(TDouble))
    {
        ExpandList(p.Count - 1);
        Count = p.Count;
        for (int i = 0; i < Count; i ++)
            List[i] = Func(p[i]);
    }
    
    void Insert(TDouble x)
    {
        ExpandList(Count);
        List[Count ++] = x;
    }

    void Delete(int Index)
    {
        assert(Index < Count);
        memcpy(List + Index, List + Index + 1, 
               sizeof(TDouble) * (Count - 1 - Index));
        Count --;
    }
    
    TDouble Last()
    {
        return List[Count - 1];
    }

    TDouble First()
    {
        return List[0];
    }

    void Merge(TKeyPoints &p)
    {
        ExpandList(Count + p.Count - 1);
        for (int i = 0; i < p.Count; i ++)
            List[Count + i] = p[i];
        Count += p.Count;
    }

    void Combine(TDouble Epsilon)
    {
        for (int i = 0; i < Count - 1; i ++)
            if (fabs(List[i + 1] - List[i]) < Epsilon)
            {
                for (int j = i + 1; j < Count - 1; j ++)
                    List[j] = List[j + 1];
                Count --;
            }
    }

    void Sort();

    TDouble Min()
    {
        assert(Count != 0);
        TDouble Min = List[0];
        for (int i = 1; i < Count; i ++)
            if (List[i] < Min)
                Min = List[i];              
        return Min;
    }

    TDouble Max()
    {
        assert(Count != 0);
        TDouble Max = List[0];
        for (int i = 1; i < Count; i ++)
            if (List[i] > Max)
                Max = List[i];              
        return Max;
    }

    TKeyPoints()
    {
        Count = 0;
        MaxCount = 0;
        List = NULL;
    }

    ~TKeyPoints()
    {
        delete List;
    }
};


#endif /* _KEY_POINTS_H_INCLUDED */
