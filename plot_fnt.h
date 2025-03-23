#ifndef _PLOT_FONT_H_INCLUDED
#define _PLOT_FONT_H_INCLUDED


#include "gfx.h"


#define TextJustifyLeft     0
#define TextJustifyTop      0
#define TextJustifyCenter   1
#define TextJustifyRight    2
#define TextJustifyBottom   2


class TPlotFixedFont {

    struct TFixedFontCharInfo {
        int BitmapOffset;
        int Width;
    };

    struct TFixedFontInfo {
        int Height;
        int BitmapsBytes;
    };

    TFixedFontCharInfo FontCharInfoList[0x100];
    unsigned char *FontBitmapList;
    int FontHeight;

    TFloat TextColor[4];

    int TextJustifyWidth;
    int TextJustifyHeight;

public:

    TPlotFixedFont()
    {
        TextJustifyWidth = TextJustifyLeft;
        TextJustifyHeight = TextJustifyTop;
        SetColor(0.0, 0.0, 0.0, 1.0);
    }
    
    ~TPlotFixedFont();

    void GetExtentPoint(const char *, int &, int &);

    int GetMaxHeight() const
    {
        return FontHeight;
    }

    void Load(unsigned char *);
    void Print(const char *);
    void SetColor(TFloat, TFloat, TFloat, TFloat);
    void SetJustify(int, int);
};


#endif /* _PLOT_FONT_H_INCLUDED */
