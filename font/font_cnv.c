
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


void errx(char *message, ...)
{
    va_list arg;
    
    va_start(arg, message);
    fprintf(stderr, "error:");
    vfprintf(stderr, message, arg);
    fputc('\n', stderr);
    exit(1);
}

typedef struct {
    int Width;
    unsigned char *Bitmap;
} TFontCharInfo;

typedef struct {
    int Height;
    TFontCharInfo *CharInfoList[0x100];
} TFont;

void ReadFont(char *filename, TFont *Font)
{
    TFontCharInfo *CharInfo;
    unsigned char Char, *CharBitmap, *CharDest;
    int Bit, CharWidth, FontHeight;
    int i, j, k;
    FILE *fp;

    fp = fopen(filename, "rt");
    if (!fp)
        errx("can't open font file '%s'", filename);
    fscanf(fp, "%d\n\n", &FontHeight);
    Font->Height = FontHeight;
    memset(Font->CharInfoList, 0, sizeof(TFontCharInfo *) * 0x100);
    while (!feof(fp))
    {
        int CharWidthBytes;
        if (fscanf(fp, "%d '%c'\n\n", &CharWidth, &Char) != 2)
            errx("invalid font file format");
        CharInfo = Font->CharInfoList[Char] = malloc(sizeof(TFontCharInfo));
        CharInfo->Width = CharWidth;
        CharWidthBytes = (CharWidth + 7) / 8;
        CharBitmap = malloc(sizeof(char) * FontHeight * CharWidthBytes);
        for (i = 0; i < FontHeight; i ++)
        {
            char Str[64];
            char *StrSrc = Str;
            fscanf(fp, "%s", Str);
            CharDest = CharBitmap + (FontHeight - 1 - i) * CharWidthBytes;
            for (j = 0; j < CharWidthBytes; j ++)
            {
                int Count = CharWidth - j * 8;
                if (Count > 8) Count = 8;
                *CharDest = 0;
                for (k = 0; k < Count; k ++)
                {
                    if (*StrSrc ++ != '0')
                        *CharDest |= 0x80 >> k;
                }
                CharDest ++;
            }
            fscanf(fp, "\n");
        }
        fscanf(fp, "\n");
        CharInfo->Bitmap = CharBitmap;
    }
    fclose(fp);
}

void StoreFont(char *Filename, TFont *Font)
{
    FILE *fp;
    typedef struct {
        int BitmapOffset;
        int Width;
    } TShortCharInfo;
    struct TFontInfo {
        int Height;
        int BitmapsBytes;
    } FontInfo;
    TShortCharInfo ShortCharInfoList[0x100];
    TFontCharInfo *CharInfo;
    int Offset = 0, i;

    FontInfo.Height = Font->Height;
    // FontInfo.Bitmaps = 0;
    for (i = 0; i < 0x100; i ++)
    {
        if (CharInfo = Font->CharInfoList[i])
        {
            ShortCharInfoList[i].Width = CharInfo->Width;
            ShortCharInfoList[i].BitmapOffset = Offset;
            Offset += Font->Height * ((CharInfo->Width + 7) / 8);
            // FontInfo.Bitmaps ++;
        }
        else
        {
            ShortCharInfoList[i].Width = 0;
            ShortCharInfoList[i].BitmapOffset = -1;
        }
    }
    FontInfo.BitmapsBytes = Offset;

    fp = fopen(Filename, "wb");
    fwrite(&FontInfo, sizeof(struct TFontInfo), 1, fp);
    for (i = 0; i < 0x100; i ++)
        if (CharInfo = Font->CharInfoList[i])
            fwrite(CharInfo->Bitmap, Font->Height * 
                   ((CharInfo->Width + 7) / 8), 1, fp);
    fwrite(ShortCharInfoList, sizeof(TShortCharInfo), 0x100, fp);
    fclose(fp);
}

int main(int argc, char **argv)
{
    TFont Font;

    if (argc != 3)
        puts("usage: text_font_file bin_font_file\n");
    else
    {
        ReadFont(argv[1], &Font);
        StoreFont(argv[2], &Font);
    }

    return 0;
}
