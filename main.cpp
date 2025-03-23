
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include <stdio.h>
#include "plot.h"
#include "resource.h"
#include "func.h"
#include "key_pnt.h"
#include "extremum.h"

#define EPSILON             1e-10
#define FUNCTION_EPSILON    1e-8
#define INPUT_EPSILON       1e-6
#define TWO_PI              6.28318530717958647692

static HINSTANCE hInstance;
static HWND hWaitDialog = 0;

#define Plot3DGridSize              80, 80

#define PlotWindowBorderWidth       1
#define PlotWindowBorderHeight      1

static int PlotDialogMinSizeX;
static int PlotDialogMinSizeY;

static int WindowIndex = -1;

struct TPlot3DOrientation {
    TFloat Alpha;
    TFloat Beta;
    TFloat Gamma;
    int Valid;
};

struct TPlotFunction {
    int Type;                   /* 0 - Plot2D, 1 - Plot3D */
    void (*InitPlotClass)(); 
    char *Name;
};

static TPlot2D *Plot2DClass;
static TPlot2DState Plot2DClassState;
static int Plot2DClassValidState = 0;
static TPlot3D *Plot3DClass;
static TPlot3DState Plot3DClassState;
static int Plot3DClassValidState = 0;

struct {
    TDouble A;
    TDouble m;
    TDouble k;
    TDouble Omega;
    TDouble Phi;
    TDouble C1;
    TDouble C2;
    TDouble T;
    int Valid;
    TDouble MinDeltaC;
} Coeffs, PrevCoeffs;

static TDouble Function_f(TDouble t)
{
    return Coeffs.A * cos(Coeffs.Omega * t + Coeffs.Phi);
}

static TDouble PlotFunction_f(TDouble t, TDouble C)
{
    return Function_f(t + C);
}

static void InitPlotClass_f()
{
    Plot3DClass->AttachFunction(PlotFunction_f, TPoint2D(0.0, Coeffs.C1), 
                                                TPoint2D(Coeffs.T, Coeffs.C2));
    Plot3DClass->SetAxesNames("t", "c", "f");
    Plot3DClass->SetGridSize(Plot3DGridSize);
}

static TDouble Function_j1(TDouble t, TDouble C)
{
    return Function_xk_Sub_x0(t + C, Coeffs.A, Coeffs.m, Coeffs.k, 
                              Coeffs.Omega, Coeffs.Phi, C);
}

#if 0
static TDouble PlotFunction_j1_Valid(TDouble C)
{
    int i, Iterations = 256;
    TDouble t = 0.0, Step = Coeffs.T / (Iterations - 1), MaxValue;
    for (i = 0; i < Iterations; i ++)
    {
        TDouble Value = Function_j1(t, C);
        t += Step;

        if (i == 0) 
            MaxValue = Value;
        else    
        if (MaxValue < Value)
            MaxValue = Value;       
    }
    
    return MaxValue;
}
#endif

static TDouble Function_j1_C;

static TDouble Function_j1_t(TDouble t)
{
    return Function_xk_Sub_x0(t + Function_j1_C, Coeffs.A, Coeffs.m, Coeffs.k, 
                              Coeffs.Omega, Coeffs.Phi, Function_j1_C);
}

static TDouble PlotFunction_j1(TDouble C)
{
    TKeyPoints KeyPoints;
    TDouble Step = 0.04 / Coeffs.Omega;
    if (Coeffs.T / Step < 500.0)
        Step = Coeffs.T / 500.0;
            
    Function_j1_C = C;

    FunctionMinMax(Function_j1_t, 0.0, Coeffs.T, Step, EPSILON, 1, 1, KeyPoints);
    KeyPoints.Insert(0.0);
    KeyPoints.Insert(Coeffs.T);
    KeyPoints.Function(Function_j1_t);
    KeyPoints.Abs();
   
    return KeyPoints.Max();
}

static void InitPlotClass_j1()
{
    TRGBColor Color = {0.0, 0.0, 1.0};
    Plot2DClass->SetAxesNames("c", "j1");
    Plot2DClass->AttachFunction(Coeffs.C1, Coeffs.C2, PlotFunction_j1, Color);
    Plot2DClass->EnableZoom(Coeffs.MinDeltaC);
}

static TDouble PlotFunction_j2(TDouble C)
{
    return Function_j2(Coeffs.A, Coeffs.m, Coeffs.m, Coeffs.Omega,
                       Coeffs.Phi, C, Coeffs.T);
}

static void InitPlotClass_j2()
{
    TRGBColor Color = {0.0, 0.0, 1.0};
    Plot2DClass->SetAxesNames("c", "j2");
    Plot2DClass->AttachFunction(Coeffs.C1, Coeffs.C2, PlotFunction_j2, Color);
    Plot2DClass->EnableZoom(Coeffs.MinDeltaC);
}

static TDouble PlotFunction_x0(TDouble t, TDouble C)
{
    return Function_x0(t + C, Coeffs.A, Coeffs.m, Coeffs.Omega, Coeffs.Phi, C);
}

static void InitPlotClass_x0()
{
    Plot3DClass->SetAxesNames("t", "c", "x0");
    Plot3DClass->AttachFunction(PlotFunction_x0, TPoint2D(0.0, Coeffs.C1), 
                                                 TPoint2D(Coeffs.T, Coeffs.C2));
    Plot3DClass->SetGridSize(Plot3DGridSize);
}

static TDouble PlotFunction_xk(TDouble t, TDouble C)
{
    return Function_xk(t + C, Coeffs.A, Coeffs.m, Coeffs.k, Coeffs.Omega, Coeffs.Phi, C);
}

static void InitPlotClass_xk()
{
    Plot3DClass->SetAxesNames("t", "c", "xk");
    Plot3DClass->AttachFunction(PlotFunction_xk, TPoint2D(0.0, Coeffs.C1), 
                                                 TPoint2D(Coeffs.T, Coeffs.C2));
    Plot3DClass->SetGridSize(Plot3DGridSize);
}

static TDouble PlotFunction_Abs_xk_Sub_x0(TDouble t, TDouble C)
{
    return fabs(Function_xk_Sub_x0(t + C, Coeffs.A, Coeffs.m, Coeffs.k, 
                                   Coeffs.Omega, Coeffs.Phi, C));
}

static TDouble PlotFunction_xk_Sub_x0(TDouble t, TDouble C)
{
    return Function_xk_Sub_x0(t + C, Coeffs.A, Coeffs.m, Coeffs.k, 
                              Coeffs.Omega, Coeffs.Phi, C);
}

static TDouble PlotFunction_Neg_xk_Sub_x0(TDouble t, TDouble C)
{
    return -Function_xk_Sub_x0(t + C, Coeffs.A, Coeffs.m, Coeffs.k, 
                               Coeffs.Omega, Coeffs.Phi, C);
}

static void InitPlotClass_Abs_xk_Sub_x0()
{
    Plot3DClass->SetAxesNames("t", "c", "");
    Plot3DClass->SetGridSize(Plot3DGridSize);
    Plot3DClass->AttachFunction(PlotFunction_xk_Sub_x0, TPoint2D(0.0, Coeffs.C1), 
                                TPoint2D(Coeffs.T, Coeffs.C2));
    Plot3DClass->AttachFunction(PlotFunction_Neg_xk_Sub_x0, TPoint2D(0.0, Coeffs.C1), 
                                                            TPoint2D(Coeffs.T, Coeffs.C2));
    Plot3DClass->FlushMesh();
    TPoint3D Min, Max;
    Plot3DClass->GetMinMax(Min, Max);

    Min.z = fabs(Min.z);
    Max.z = fabs(Max.z);
    Max.z = (Min.z < Max.z) ? Max.z : Min.z;
    Min.z = 0.0;

    Plot3DClass->SetMinMax(Min, Max);
}

const int PlotFunctions = 6;
int PlotFunctionIndex = 0;

static TPlotFunction PlotFunctionList[PlotFunctions] = {
    {1, InitPlotClass_f,                "f(c+t)"                  },
    {1, InitPlotClass_x0,               "x0(c+t,c)"               },
    {1, InitPlotClass_xk,               "xk(c+t,c)"               },
    {1, InitPlotClass_Abs_xk_Sub_x0,    "|xk(c+t,c)-x0(c+t,c)|"   },
    {0, InitPlotClass_j1,               "j1(c)"                   },
    {0, InitPlotClass_j2,               "j2(c)"                   }
};

static TPlot3DOrientation Plot3DOrientationList[PlotFunctions];

static HFONT CreateSymbolFont(int Size)
{
    return CreateFont(Size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET,
                      OUT_DEFAULT_PRECIS,
                      CLIP_DEFAULT_PRECIS,
                      NONANTIALIASED_QUALITY,
                      0, "Symbol");
}

static int APIENTRY 
WaitDlgBoxProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HCURSOR hWaitCursor;
    
    switch (msg)
    {
    case WM_INITDIALOG:
        hWaitCursor = LoadCursor(0, IDC_WAIT);
        SetCursor(hWaitCursor);
        break;
    }

    return 0;
}

static void CreateWaitDlgBox()
{
    hWaitDialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_WAIT_DIALOG), 0, WaitDlgBoxProc);
    SetWindowText(hWaitDialog, "Math");
    ShowWindow(hWaitDialog, SW_SHOW);
    UpdateWindow(hWaitDialog);  
}

static void DestroyWaitDlgBox()
{
    if (hWaitDialog)
    {
        DestroyWindow(hWaitDialog);
        hWaitDialog = 0;
    }
}

struct {
    char *Text1;
    char *CoeffName;
    char *Text2;
    char *Caption;
    int Symbol;
} CoeffMsgBoxParams;

static int APIENTRY 
CoeffMessageBoxDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HFONT hFont;
    static HFONT hSymbolFont;
    static int TextWidth, TextHeight, TextTop;
    static int Text1Width, Text2Width, TextShift;
    static int CoeffWidth, CoeffHeight, CoeffShift;
    const int BorderWidth = 12;
    
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            NONCLIENTMETRICS nclm;
            nclm.cbSize = sizeof(NONCLIENTMETRICS);
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &nclm, 0);
            hFont = CreateFontIndirect(&nclm.lfMessageFont);
            hSymbolFont = CreateSymbolFont(nclm.lfMessageFont.lfHeight - 3);
            HDC hDC = GetDC(hDlg);

            RECT Rect;
            Rect.left = 0;
            Rect.top = 0;           
            
            HFONT hPrevFont = (HFONT) SelectObject(hDC, hFont);
            
            TEXTMETRIC TextMetric;
            GetTextMetrics(hDC, &TextMetric);
            
            DrawText(hDC, CoeffMsgBoxParams.Text1, strlen(CoeffMsgBoxParams.Text1), &Rect, DT_LEFT | DT_VCENTER | DT_CALCRECT);
            Text1Width = Rect.right;
            TextShift = TextMetric.tmHeight - TextMetric.tmInternalLeading;
            TextHeight = TextMetric.tmHeight;
            
            if (CoeffMsgBoxParams.CoeffName)
            {
                if (CoeffMsgBoxParams.Symbol)
                {
                    SelectObject(hDC, hSymbolFont);
                    GetTextMetrics(hDC, &TextMetric);
                    CoeffShift = TextMetric.tmHeight - TextMetric.tmInternalLeading;
                    if (TextHeight < TextMetric.tmHeight)
                        TextHeight = TextMetric.tmHeight;
                    DrawText(hDC, CoeffMsgBoxParams.CoeffName, -1, &Rect, DT_LEFT | DT_CALCRECT);
                    SelectObject(hDC, hFont);
                }
                else
                {
                    DrawText(hDC, CoeffMsgBoxParams.CoeffName, -1, &Rect, DT_LEFT | DT_CALCRECT);
                    CoeffShift = TextShift;
                }
                    
                CoeffWidth = Rect.right;
            }
            else
            {
                CoeffWidth = 0;
            }

            if (CoeffMsgBoxParams.Text2)
            {
                DrawText(hDC, CoeffMsgBoxParams.Text2, -1, &Rect, DT_LEFT | DT_CALCRECT);
                Text2Width = Rect.right;
            }
            else
            {
                Text2Width = 0;
            }

            int MaxShift = (TextShift > CoeffShift) ? TextShift : CoeffShift;
            TextShift = MaxShift - TextShift;
            CoeffShift = MaxShift - CoeffShift;
                        
            TextWidth = Text1Width + CoeffWidth + Text2Width;
            
            SelectObject(hDC, hPrevFont);
            ReleaseDC(hDlg, hDC);

            GetWindowRect(hDlg, &Rect);
            int DialogWidth = TextWidth + 2 * BorderWidth;
            int DialogHeight = Rect.bottom - Rect.top;
            SetWindowPos(hDlg, 0, 
                         (GetSystemMetrics(SM_CXSCREEN) - DialogWidth) / 2, 
                         (GetSystemMetrics(SM_CYSCREEN) - DialogHeight) / 2, 
                         DialogWidth, DialogHeight, 0);
            
            HWND hButton = GetDlgItem(hDlg, IDOK);
            GetWindowRect(hButton, &Rect);
            MapWindowPoints(NULL, hDlg, (LPPOINT) &Rect, 2);
            SetWindowPos(hButton, NULL, (TextWidth - (Rect.right - Rect.left)) / 2 + BorderWidth, Rect.top,
                         Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOZORDER);
    
            SetWindowText(hDlg, CoeffMsgBoxParams.Caption);

            HWND hText = GetDlgItem(hDlg, IDC_MSGBOX_TEXT);
            GetWindowRect(hText, &Rect);
            MapWindowPoints(NULL, hDlg, (LPPOINT) &Rect, 2);

            TextTop = (Rect.top + Rect.bottom - TextHeight) / 2;
        }   
        break;
    
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hDC;

            hDC = BeginPaint(hDlg, &ps);
            SetBkMode(hDC, TRANSPARENT);
            SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

            RECT Rect;
            Rect.left = BorderWidth;
            Rect.top = TextTop;
            Rect.right = Rect.left + TextWidth;
            Rect.bottom = TextTop + TextHeight;       
        
            Rect.top += TextShift;
            
            HFONT hPrevFont;
            hPrevFont = (HFONT) SelectObject(hDC, hFont);
            DrawText(hDC, CoeffMsgBoxParams.Text1, -1, &Rect, DT_LEFT);
            Rect.left += Text1Width;

            if (CoeffMsgBoxParams.Symbol)
            {
                SelectObject(hDC, hSymbolFont);
                Rect.top += CoeffShift - TextShift;
                DrawText(hDC, CoeffMsgBoxParams.CoeffName, -1, &Rect, DT_LEFT);
                Rect.left += CoeffWidth;
                Rect.top -= CoeffShift - TextShift;
                SelectObject(hDC, hFont);
            }
            else
            {
                DrawText(hDC, CoeffMsgBoxParams.CoeffName, -1, &Rect, DT_LEFT);
                Rect.left += CoeffWidth;
            }

            DrawText(hDC, CoeffMsgBoxParams.Text2, -1, &Rect, DT_LEFT);

            SelectObject(hDC, hPrevFont);
            EndPaint(hDlg, &ps);
        }
        break;
    
    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
            DeleteObject(hFont);
            DeleteObject(hSymbolFont);
            EndDialog(hDlg, 1);
            break;
        }
        break;

    case WM_CLOSE:
        DeleteObject(hFont);
        DeleteObject(hSymbolFont);
        EndDialog(hDlg, 0);
        break;  
    }

    return 0;
}

static void CoeffMessageBox(HWND Parent, char *Text1, char *CoeffName, 
                            char *Text2, char *Caption)
{
    int Symbol = 0;

    if (CoeffName)
    {
        if (strcmp(CoeffName, "Omega") == 0)
            Symbol = 1, CoeffName = "w";
        if (strcmp(CoeffName, "Phi") == 0)
            Symbol = 1, CoeffName = "j";
    }
    
    CoeffMsgBoxParams.Text1 = Text1;
    CoeffMsgBoxParams.CoeffName = CoeffName;
    CoeffMsgBoxParams.Symbol = Symbol;
    CoeffMsgBoxParams.Text2 = Text2;
    CoeffMsgBoxParams.Caption = Caption;

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_COEFF_MSGBOX), Parent, CoeffMessageBoxDlgProc);
}

static void WrongCoeff(HWND hDlg, int DlgItem, char *Text1, char *CoeffName, char *Text2)
{
    SetFocus(GetDlgItem(hDlg, DlgItem));
    CoeffMessageBox(hDlg, Text1, CoeffName, Text2, "Ошибка");   
    SetFocus(GetDlgItem(hDlg, DlgItem)); /* Wine bug */
}

static void WrongCoeffGEqual(HWND hDlg, int DlgItem, char *Name, char *Limit)
{
    char StrBuffer[128];
    sprintf(StrBuffer, " должен быть больше либо равен %s.", Limit);
    WrongCoeff(hDlg, DlgItem, "Коэффициент ", Name, StrBuffer);
}

static void WrongCoeffLEqual(HWND hDlg, int DlgItem, char *Name, char *Limit)
{
    char StrBuffer[128];
    sprintf(StrBuffer, " должен быть меньше либо равен %s.", Limit);
    WrongCoeff(hDlg, DlgItem, "Коэффициент ", Name, StrBuffer);
}

static void WrongCoeffGEqual(HWND hDlg, int DlgItem, char *Name, TDouble Limit)
{
    char StrBuffer[32];
    sprintf(StrBuffer, "%.8lg", Limit);
    WrongCoeffGEqual(hDlg, DlgItem, Name, StrBuffer);
}

static void WrongCoeffLEqual(HWND hDlg, int DlgItem, char *Name, TDouble Limit)
{
    char StrBuffer[32];
    sprintf(StrBuffer, "%.8lg", Limit);
    WrongCoeffLEqual(hDlg, DlgItem, Name, StrBuffer);
}

static int ReadNumber(HWND hDlg, int DlgItem, TDouble &Number, char *CoeffName)
{
    char StrBuffer[128];
    GetDlgItemText(hDlg, DlgItem, StrBuffer, sizeof(StrBuffer));
    if (sscanf(StrBuffer, "%lf", &Number) != 1)
    {
        SetFocus(GetDlgItem(hDlg, DlgItem));
        CoeffMessageBox(hDlg, "Неверно указан коэффициент ", CoeffName, ", повторите ввод.", "Ошибка");
        SetFocus(GetDlgItem(hDlg, DlgItem)); /* Wine bug */
        return 0;
    }
    return 1;
}

static void WriteNumber(HWND hDlg, int DlgItem, TDouble Number)
{
    char StrBuffer[128];
    sprintf(StrBuffer, "%.8lg", Number);
    SetDlgItemText(hDlg, DlgItem, StrBuffer);
}

static int APIENTRY 
MainDlgBoxProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TDouble LimitC2, Limit1C2, Limit2C2;
    TDouble LimitT, Limit1T, Limit2T;
    static HFONT hSymbolFont;
    char StrBuffer[128];

    switch (msg) 
    {
    case WM_INITDIALOG:
        hSymbolFont = CreateSymbolFont(-12);

        if (Coeffs.Valid)
        {
            WriteNumber(hDlg, IDC_EDIT_A,     Coeffs.A    );
            WriteNumber(hDlg, IDC_EDIT_M,     Coeffs.m    );
            WriteNumber(hDlg, IDC_EDIT_K,     Coeffs.k    );
            WriteNumber(hDlg, IDC_EDIT_OMEGA, Coeffs.Omega);
            WriteNumber(hDlg, IDC_EDIT_PHI,   Coeffs.Phi  );
            WriteNumber(hDlg, IDC_EDIT_C1,    Coeffs.C1   );
            WriteNumber(hDlg, IDC_EDIT_C2,    Coeffs.C2   );
            WriteNumber(hDlg, IDC_EDIT_T,     Coeffs.T    );
        }

        SendDlgItemMessage(hDlg, IDC_STATIC_OMEGA, WM_SETFONT, (UINT) hSymbolFont, TRUE);
        SendDlgItemMessage(hDlg, IDC_STATIC_PHI,   WM_SETFONT, (UINT) hSymbolFont, TRUE);

        EnableWindow(GetDlgItem(hDlg, IDC_BACK), WindowIndex >= 0);
        break;

    case WM_CLOSE:
        DeleteObject(hSymbolFont);
        EndDialog(hDlg, IDC_QUIT);
        break;

    case WM_COMMAND:
        switch (wParam) 
        {
        case IDC_NEXT:
        case IDC_BACK:
            if (!ReadNumber(hDlg, IDC_EDIT_A,     Coeffs.A,     "A"    )) break;
            if (!ReadNumber(hDlg, IDC_EDIT_M,     Coeffs.m,     "m"    )) break;
            if (!ReadNumber(hDlg, IDC_EDIT_K,     Coeffs.k,     "k"    )) break;
            if (!ReadNumber(hDlg, IDC_EDIT_OMEGA, Coeffs.Omega, "Omega")) break;
            if (!ReadNumber(hDlg, IDC_EDIT_PHI,   Coeffs.Phi,   "Phi"  )) break;
            if (!ReadNumber(hDlg, IDC_EDIT_C1,    Coeffs.C1,    "c1"   )) break;
            if (!ReadNumber(hDlg, IDC_EDIT_C2,    Coeffs.C2,    "c2"   )) break;
            if (!ReadNumber(hDlg, IDC_EDIT_T,     Coeffs.T,     "T"    )) break;

            Limit1C2 = 20.0 * Coeffs.m / Coeffs.k;
            Limit2C2 = TWO_PI / Coeffs.Omega;
            LimitC2 = (Limit1C2 < Limit2C2) ? Limit1C2 : Limit2C2;

            Limit1T = 0.05;
            Limit2T = 0.1 / Coeffs.Omega;
            LimitT = (Limit1T > Limit2T) ? Limit1T : Limit2T;

            Coeffs.MinDeltaC = 0.01 / Coeffs.Omega;

            if (!(Coeffs.A >= 0.01 - INPUT_EPSILON))
                WrongCoeffGEqual(hDlg, IDC_EDIT_A, "A", "0.01");
            else
            if (!(Coeffs.A <= 100.0 + INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_A, "A", "100");
            else
            if (!(Coeffs.m >= 0.01 - INPUT_EPSILON))
                WrongCoeffGEqual(hDlg, IDC_EDIT_M, "m", "0.01");
            else 
            if (!(Coeffs.m <= 100.0 + INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_M, "m", "100");
            else 
            if (!(Coeffs.k >= 0.01 - INPUT_EPSILON)) 
                WrongCoeffGEqual(hDlg, IDC_EDIT_K, "k", "0.01");
            else
            if (!(Coeffs.k <= 100.0 + INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_K, "k", "100");
            else 
            if (!(Coeffs.Omega >= 0.01 - INPUT_EPSILON))
                WrongCoeffGEqual(hDlg, IDC_EDIT_OMEGA, "Omega", "0.01");
            else
            if (!(Coeffs.Omega <= 100.0 + INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_OMEGA, "Omega", "100");
            else
            if (!(Coeffs.Phi >= 0.0))
                WrongCoeffGEqual(hDlg, IDC_EDIT_PHI, "Phi", "0");
            else 
            if (!(Coeffs.Phi <= TWO_PI + INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_PHI, "Phi", TWO_PI);
            else
            if (!(Coeffs.C1 >= 0.0))
                WrongCoeffGEqual(hDlg, IDC_EDIT_C1, "c1", "0");
            else
            if (!(Coeffs.C1 + Coeffs.MinDeltaC <= Coeffs.C2 + INPUT_EPSILON))
            {
                sprintf(StrBuffer, "Коэффициент c2 должен быть больше c1 хотя бы на %.8lg.", 0.01 / Coeffs.Omega);
                WrongCoeff(hDlg, IDC_EDIT_C2, StrBuffer, NULL, NULL);
            }
            else
            if (!(Coeffs.C2 <= LimitC2 + INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_C2, "c2", LimitC2);
            else
            if (!(Coeffs.T >= LimitT - INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_T, "T", LimitT);
            else
            if (!(Coeffs.T * Coeffs.Omega <= 40.0 + INPUT_EPSILON))
                WrongCoeffLEqual(hDlg, IDC_EDIT_T, "T", 40.0 / Coeffs.Omega);
            else 
            {
                Coeffs.Valid = 1;
                DeleteObject(hSymbolFont);
                EndDialog(hDlg, wParam);
            }
            break;

        case IDC_QUIT:
            DeleteObject(hSymbolFont);
            EndDialog(hDlg, IDC_QUIT);
            break;
        }
        break;
    }

    return 0;
}

static RECT ButtonBackRect;
static RECT ButtonNextRect;
static RECT ButtonQuitRect;
static RECT ButtonBeginRect;
static RECT PlotWindowRect;
static RECT PlotDialogRect;
static RECT PrevPlotDialogRect;
static int PrevPlotDialogRectValid = 0;
static int PlotDialogFirstCall = 1;

static void RepaintPlotWindow()
{
    if (Plot2DClass)
        Plot2DClass->Display();

    if (Plot3DClass)
        Plot3DClass->Display();
}

static void ScreenRectToClient(HWND hWnd, RECT &Rect)
{
    POINT Point;

    Point.x = Rect.left;
    Point.y = Rect.top;
    ScreenToClient(hWnd, &Point);
    Rect.left = Point.x;
    Rect.top = Point.y;

    Point.x = Rect.right;
    Point.y = Rect.bottom;
    ScreenToClient(hWnd, &Point);
    Rect.right = Point.x;
    Rect.bottom = Point.y;
}

static void GetPlotWindowRect(HWND hDlg, RECT &Rect)
{
    GetWindowRect(hDlg, &PlotDialogRect);
    ScreenRectToClient(hDlg, PlotDialogRect);
    
    Rect.left = PlotDialogRect.left + PlotWindowRect.left;
    Rect.top = PlotDialogRect.top + PlotWindowRect.top;
    Rect.right = PlotDialogRect.right - PlotWindowRect.right;
    Rect.bottom = PlotDialogRect.bottom - PlotWindowRect.bottom;        

    Rect.right -= Rect.left;
    Rect.bottom -= Rect.top;
}

static void ResizePlotDialog(HWND hDlg)
{
    GetWindowRect(hDlg, &PlotDialogRect);
    ScreenRectToClient(hDlg, PlotDialogRect);
    
    SetWindowPos(GetDlgItem(hDlg, IDC_BEGIN), 0,
                 PlotDialogRect.left + ButtonBeginRect.left,
                 PlotDialogRect.bottom - ButtonBeginRect.top,
                 ButtonBeginRect.right,
                 ButtonBeginRect.bottom, SWP_NOSIZE | SWP_NOACTIVATE);      
    
    SetWindowPos(GetDlgItem(hDlg, IDC_BACK), 0,
                 (PlotDialogRect.left + PlotDialogRect.right) / 2 - ButtonNextRect.right - 1,
                 PlotDialogRect.bottom - ButtonBackRect.top,
                 ButtonBackRect.right,
                 ButtonBackRect.bottom, SWP_NOSIZE | SWP_NOACTIVATE);       

    SetWindowPos(GetDlgItem(hDlg, IDC_NEXT), 0,
                 (PlotDialogRect.left + PlotDialogRect.right) / 2,
                 PlotDialogRect.bottom - ButtonNextRect.top,
                 ButtonNextRect.right,
                 ButtonNextRect.bottom, SWP_NOSIZE | SWP_NOACTIVATE);       

    SetWindowPos(GetDlgItem(hDlg, IDC_QUIT), 0,
                 PlotDialogRect.right - ButtonQuitRect.left,
                 PlotDialogRect.bottom - ButtonQuitRect.top,
                 ButtonQuitRect.right,
                 ButtonQuitRect.bottom, SWP_NOSIZE | SWP_NOACTIVATE);
    
    RECT PlotWindowRect;
    GetPlotWindowRect(hDlg, PlotWindowRect);

    SetWindowPos(GetDlgItem(hDlg, IDC_PLOT_WINDOW), 0,
                 PlotWindowRect.left, PlotWindowRect.top,
                 PlotWindowRect.right, PlotWindowRect.bottom, SWP_NOACTIVATE);

    if (Plot2DClass || Plot3DClass)
    {
        HWND hPlotWnd = Plot2DClass ? Plot2DClass->hWnd : Plot3DClass->hWnd;
        SetWindowPos(hPlotWnd, 0,
                     PlotWindowBorderWidth, PlotWindowBorderHeight,
                     PlotWindowRect.right - PlotWindowBorderWidth * 2, 
                     PlotWindowRect.bottom - PlotWindowBorderHeight * 2, 0);
        InvalidateRect(hPlotWnd, NULL, TRUE);
    }

    InvalidateRect(GetDlgItem(hDlg, IDC_PLOT_WINDOW), NULL, TRUE);
    InvalidateRect(GetDlgItem(hDlg, IDC_BACK), NULL, TRUE);
    InvalidateRect(GetDlgItem(hDlg, IDC_NEXT), NULL, TRUE);
    // InvalidateRect(GetDlgItem(hDlg, IDC_QUIT), NULL, TRUE);
    // InvalidateRect(GetDlgItem(hDlg, IDC_BEGIN), NULL, TRUE);
}

static void DeletePlotClass()
{
    if (Plot2DClass)
    {
        Plot2DClass->StoreState(Plot2DClassState);
        Plot2DClassValidState = 1;
        delete Plot2DClass;
        Plot2DClass = NULL;
    }   

    if (Plot3DClass)
    {   
        Plot3DClass->StoreState(Plot3DClassState);
        Plot3DClassValidState = 1;

        TPlot3DOrientation &Orientation = Plot3DOrientationList[PlotFunctionIndex];
        Plot3DClass->GetOrientation(Orientation.Alpha, Orientation.Beta, Orientation.Gamma);
        Orientation.Valid = 1;

        delete Plot3DClass;
        Plot3DClass = NULL;
    }
}

static void UpdateFunctionName(HWND hDlg)
{
    char StrBuffer[128];
    sprintf(StrBuffer, "График функции %s", PlotFunctionList[PlotFunctionIndex].Name);
    SetDlgItemText(hDlg, IDC_FUNC_NAME, StrBuffer);
}

static void InitPlotClass(HWND hDlg)
{
    RECT PlotWindowRect;    
    GetPlotWindowRect(hDlg, PlotWindowRect);

    HWND hPlotWnd = GetDlgItem(hDlg, IDC_PLOT_WINDOW);
    
    if (PlotFunctionList[PlotFunctionIndex].Type == 0)
    {
        Plot2DClass = new TPlot2D(hInstance, 
                                  PlotWindowBorderWidth,
                                  PlotWindowBorderHeight,
                                  PlotWindowRect.right - PlotWindowBorderWidth * 2,
                                  PlotWindowRect.bottom - PlotWindowBorderHeight * 2,
                                  "",
                                  hPlotWnd);
    }
    else
    {
        Plot3DClass = new TPlot3D(hInstance, 
                                  PlotWindowBorderWidth,
                                  PlotWindowBorderHeight,
                                  PlotWindowRect.right - PlotWindowBorderWidth * 2,
                                  PlotWindowRect.bottom - PlotWindowBorderHeight * 2,
                                  "",
                                  hPlotWnd);
    }

    PlotFunctionList[PlotFunctionIndex].InitPlotClass();
    
    if (Plot2DClass)
    {
        if (Plot2DClassValidState)
            Plot2DClass->LoadState(Plot2DClassState);
        Plot2DClass->Flush();
    }

    if (Plot3DClass)
    {
        if (Plot3DClassValidState)
            Plot3DClass->LoadState(Plot3DClassState);
        TPlot3DOrientation &Orientation = Plot3DOrientationList[PlotFunctionIndex];
        if (Orientation.Valid)
            Plot3DClass->SetOrientation(Orientation.Alpha, Orientation.Beta, Orientation.Gamma); 
        Plot3DClass->Flush();
    }
}

static void ComputeWindowShift(RECT &Window, RECT &Dialog, int WidthJustify)
{
    Window.right -= Window.left;
    Window.bottom -= Window.top;

    if (WidthJustify)
            Window.left = Dialog.right - Window.left;
    else    Window.left = Window.left - Dialog.left;
    Window.top = Dialog.bottom - Window.top;
}

static int APIENTRY 
PlotDlgBoxProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{   
    switch (msg) 
    {
    case WM_INITDIALOG:
        {       
            if (PlotDialogFirstCall) 
            {
                GetWindowRect(GetDlgItem(hDlg, IDC_BACK), &ButtonBackRect);
                GetWindowRect(GetDlgItem(hDlg, IDC_NEXT), &ButtonNextRect);
                GetWindowRect(GetDlgItem(hDlg, IDC_QUIT), &ButtonQuitRect);
                GetWindowRect(GetDlgItem(hDlg, IDC_BEGIN), &ButtonBeginRect);
                GetWindowRect(GetDlgItem(hDlg, IDC_PLOT_WINDOW), &PlotWindowRect);
                GetWindowRect(hDlg, &PlotDialogRect);

                PlotWindowRect.left -= PlotDialogRect.left;
                PlotWindowRect.top -= PlotDialogRect.top;
                PlotWindowRect.right = PlotDialogRect.right - PlotWindowRect.right;
                PlotWindowRect.bottom = PlotDialogRect.bottom - PlotWindowRect.bottom;
                
                ComputeWindowShift(ButtonBeginRect, PlotDialogRect, 0);
                ComputeWindowShift(ButtonBackRect, PlotDialogRect, 0);
                ComputeWindowShift(ButtonNextRect, PlotDialogRect, 1);
                ComputeWindowShift(ButtonQuitRect, PlotDialogRect, 1);

                PlotDialogMinSizeX = PlotDialogRect.right - PlotDialogRect.left;
                PlotDialogMinSizeY = PlotDialogRect.bottom - PlotDialogRect.top;

                PlotDialogFirstCall = 0;
            }
            
            if (PrevPlotDialogRectValid)
                MoveWindow(hDlg, PrevPlotDialogRect.left, PrevPlotDialogRect.top, 
                                 PrevPlotDialogRect.right - PrevPlotDialogRect.left,
                                 PrevPlotDialogRect.bottom - PrevPlotDialogRect.top, TRUE);

            InitPlotClass(hDlg);
            UpdateFunctionName(hDlg);
            DestroyWaitDlgBox();
        }
        break;

    case WM_SIZE:   
        ResizePlotDialog(hDlg);
        break;

    case WM_GETMINMAXINFO:
        {   
            LPMINMAXINFO MinMax = (LPMINMAXINFO) lParam;
            MinMax->ptMinTrackSize.x = PlotDialogMinSizeX;
            MinMax->ptMinTrackSize.y = PlotDialogMinSizeY;
        }    
        break;
    
    case WM_CLOSE:
        DeletePlotClass();
        EndDialog(hDlg, IDC_QUIT);
        break;

    case WM_COMMAND:
        int Exit = 0;
        switch (wParam) 
        {
        case IDC_BACK:  
            if (PlotFunctionIndex)
            {
                DeletePlotClass();
                PlotFunctionIndex --;
                InitPlotClass(hDlg);
                RepaintPlotWindow();
                UpdateFunctionName(hDlg);
            }
            else
            {
                Exit = wParam;
            }
            break;

        case IDC_NEXT:
            if (PlotFunctionIndex != PlotFunctions - 1)
            {
                DeletePlotClass();
                PlotFunctionIndex ++;
                InitPlotClass(hDlg);                
                RepaintPlotWindow();
                UpdateFunctionName(hDlg);
            }
            else 
            {
                Exit = wParam;
            }
            break;

        case IDC_QUIT:  
        case IDC_BEGIN: 
            Exit = wParam; 
            break;
        }

        if (Exit)
        {
            DeletePlotClass();

            PrevPlotDialogRectValid = 1;
            GetWindowRect(hDlg, &PrevPlotDialogRect);

            EndDialog(hDlg, Exit);
        }
        break;
    }

    return 0;
}

static int PlotDialog()
{
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_PLOT_DIALOG), 0, PlotDlgBoxProc);
}

static TKeyPoints FuncMinC_j1;
static TKeyPoints FuncValueMinC_j1;
static TKeyPoints FuncMinC_j2;
static TKeyPoints FuncValueMinC_j2;

static void BuildPointList(HWND hCount, HWND hList, 
                           TKeyPoints &x, char *AxisX, 
                           TKeyPoints &y, char *AxisY)
{
    char StrBuffer[64];

    sprintf(StrBuffer, "Всего: %d", x.Count);
    SetWindowText(hCount, StrBuffer);
    
    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);

    LVCOLUMN Clm;
    Clm.mask = LVCF_TEXT | LVCF_WIDTH;

    Clm.pszText = "Номер";
    Clm.cx = 51;
    ListView_InsertColumn(hList, 0, &Clm);
                
    Clm.pszText = AxisX;
    Clm.cx = 100;
    ListView_InsertColumn(hList, 1, &Clm);

    Clm.pszText = AxisY;
    Clm.cx = 100;
    ListView_InsertColumn(hList, 2, &Clm);

    LVITEM Item;
    memset(&Item, 0, sizeof(LVITEM));
    Item.mask = LVIF_TEXT;
    for (int i = 0; i < x.Count; i ++)
    {
        Item.iItem = i;
        Item.pszText = StrBuffer;
        sprintf(StrBuffer, "%d", i + 1);
        ListView_InsertItem(hList, &Item);

        sprintf(StrBuffer, "%lf", x[i]);
        ListView_SetItemText(hList, i, 1, StrBuffer);

        sprintf(StrBuffer, "%lf", y[i]);
        ListView_SetItemText(hList, i, 2, StrBuffer);
    }
}

static int APIENTRY 
PointListDlgBoxProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        InitCommonControls();
        BuildPointList(GetDlgItem(hDlg, IDC_POINTS_J1), 
                       GetDlgItem(hDlg, IDC_POINT_LIST_J1), 
                       FuncMinC_j1, "c", FuncValueMinC_j1, "j1(c)");
        BuildPointList(GetDlgItem(hDlg, IDC_POINTS_J2), 
                       GetDlgItem(hDlg, IDC_POINT_LIST_J2), 
                       FuncMinC_j2, "c", FuncValueMinC_j2, "j2(c)");
        break;
    
    case WM_CLOSE:
        EndDialog(hDlg, IDC_QUIT);
        break;

    case WM_COMMAND:
        if (wParam == IDC_BACK ||
            wParam == IDC_NEXT ||
            wParam == IDC_QUIT)
                EndDialog(hDlg, wParam);
        break;
    }

    return 0;
}

static TKeyPoints *PointListDialogInit_j1()
{
    return &FuncMinC_j1;
}

static void Minimize()
{
    TDouble Step = 0.05 / Coeffs.Omega;
    TDouble Range = Coeffs.C2 - Coeffs.C1;
    if (Range / Step < 500.0)
        Step = Range / 500.0;

    FuncMinC_j1.Clear();        
    FunctionMinMax(PlotFunction_j1, Coeffs.C1, Coeffs.C2, Step, EPSILON, 1, 0, FuncMinC_j1);
    FuncValueMinC_j1.Function(FuncMinC_j1, PlotFunction_j1);

    if (FuncMinC_j1.Count)
    {
        TDouble Min = FuncValueMinC_j1.Min();
        int Index = 0;
        do 
        {
            if (FuncValueMinC_j1[Index] - Min > FUNCTION_EPSILON)
            {
                FuncMinC_j1.Delete(Index);
                FuncValueMinC_j1.Delete(Index);
            }   
            else
            {
                Index ++;
            }
        } while (Index < FuncMinC_j1.Count);
    }

    Step = 0.001 / Coeffs.Omega;
    if (Range / Step < 500.0)
        Step = Range / 500.0;

    FuncMinC_j2.Clear();
    FunctionMinMax(PlotFunction_j2, Coeffs.C1, Coeffs.C2, Step, EPSILON, 1, 0, FuncMinC_j2);
    FuncValueMinC_j2.Function(FuncMinC_j2, PlotFunction_j2);

    if (FuncMinC_j2.Count)
    {
        TDouble Min = FuncValueMinC_j2.Min();
        int Index = 0;
        do 
        {
            if (FuncValueMinC_j2[Index] - Min > FUNCTION_EPSILON)
            {
                FuncMinC_j2.Delete(Index);
                FuncValueMinC_j2.Delete(Index);
            }   
            else
            {
                Index ++;
            }
        } while (Index < FuncMinC_j2.Count);
    }
}

int APIENTRY
WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance,
        LPSTR lpszCmdLine, int nCmdShow)
{
    (void) hPrevInstance;
    (void) lpszCmdLine;
    (void) nCmdShow;
    
    hInstance = hCurInstance;

    Coeffs.Valid = 0;
    PrevCoeffs.Valid = 0;

    for (int i = 0; i < PlotFunctions; i ++)
        Plot3DOrientationList[i].Valid = 0;

NewCoeffs:

    switch (DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), 0, MainDlgBoxProc))
    {
    case IDC_BACK: break;
    case IDC_NEXT: WindowIndex =  0; break;
    case IDC_QUIT: goto Quit;
    }

    if (PrevCoeffs.Valid == 0 || memcmp(&Coeffs, &PrevCoeffs, sizeof(Coeffs)))
    {
        PrevCoeffs = Coeffs;
        CreateWaitDlgBox();
        Minimize();
    }

    for (;;)
    {
        if (WindowIndex < PlotFunctions)
        {
            PlotFunctionIndex = WindowIndex;

            switch (PlotDialog())
            {
            case IDC_BEGIN:
            case IDC_BACK: 
                WindowIndex = PlotFunctionIndex;
                goto NewCoeffs;

            case IDC_QUIT: 
                goto Quit;
            }
        }

        WindowIndex = PlotFunctions;
                
        DestroyWaitDlgBox();

        switch (DialogBox(hInstance, MAKEINTRESOURCE(IDD_POINTS_DIALOG),
                          0, PointListDlgBoxProc))
        {
        case IDC_QUIT:
            return 0;

        case IDC_NEXT:
            goto NewCoeffs;

        case IDC_BACK:
            WindowIndex --;
            break;              
        }
    }

Quit:   
    return 0;
}
