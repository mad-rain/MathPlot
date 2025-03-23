
#include "key_pnt.h"


static int PointCmp(const void *_p0, const void *_p1)
{
    TDouble p0 = *((TDouble *) _p0);
    TDouble p1 = *((TDouble *) _p1);
    return (p0 == p1) ? 0 : ((p0 < p1) ? -1 : 1);
}

void TKeyPoints::ExpandList(int Index)
{
    if (Index >= MaxCount)
    {
        if (MaxCount == 0)
            MaxCount = 2;
            do MaxCount *= 2; while (MaxCount <= Index);
        List = (TDouble *) realloc(List, sizeof(TDouble) * MaxCount);
    }
}

void TKeyPoints::Sort()
{
    if (Count)
        qsort(List, Count, sizeof(TDouble), PointCmp);
}
