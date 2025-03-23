
#include <string.h>
#include "gfx.h"
#include "plot_fnt.h"
#include <GL/gl.h>


void TPlotFixedFont::Load(unsigned char *FontStream)
{
    TFixedFontInfo &FontInfo = (TFixedFontInfo &) FontStream[0];

    FontHeight = FontInfo.Height;
    FontBitmapList = new unsigned char[FontInfo.BitmapsBytes];
    FontStream += sizeof(TFixedFontInfo);
    memcpy(FontBitmapList, FontStream, FontInfo.BitmapsBytes);
    FontStream += FontInfo.BitmapsBytes;
    memcpy(FontCharInfoList, FontStream, sizeof(TFixedFontCharInfo) * 0x100);
}

TPlotFixedFont::~TPlotFixedFont()
{
    delete FontBitmapList;
}

void TPlotFixedFont::GetExtentPoint(char *Message, int &Width, int &Height)
{
    Width = 0;
    Height = FontHeight;
    char ch;
    while (ch = *Message ++)
        Width += FontCharInfoList[ch].Width;
}

void TPlotFixedFont::SetColor(TFloat r, TFloat g, TFloat b, TFloat a)
{
    TextColor[0] = r;
    TextColor[1] = g;
    TextColor[2] = b;
    TextColor[3] = a;
}

void TPlotFixedFont::SetJustify(int Width, int Height)
{
    TextJustifyWidth = Width;
    TextJustifyHeight = Height;
}

void TPlotFixedFont::Print(char *Message)
{
    int Width, Height;
    int WidthShift, HeightShift;

    GetExtentPoint(Message, Width, Height);

    switch (TextJustifyWidth)
    {
    case TextJustifyLeft:       WidthShift = 0;         break;
    case TextJustifyCenter:     WidthShift = Width / 2; break;
    case TextJustifyRight:      WidthShift = Width - 1; break;
    }

    switch (TextJustifyHeight)
    {
    case TextJustifyTop:        HeightShift = Height - 1; break;
    case TextJustifyCenter:     HeightShift = Height / 2; break;
    case TextJustifyBottom:     HeightShift = 0;          break;
    }
    
    glColor4f(TextColor[0], TextColor[1], TextColor[2], TextColor[3]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    while (char ch = *Message ++)
    {
        TBYTE *CharBitmap = FontBitmapList + FontCharInfoList[ch].BitmapOffset;
        int CharWidth = FontCharInfoList[ch].Width;
        glBitmap(CharWidth, FontHeight, 
                 (GLfloat) WidthShift, (GLfloat) HeightShift,
                 (GLfloat) CharWidth, 0.0f, CharBitmap);
    }
}
