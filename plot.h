#ifndef _PLOT_H_INCLUDED
#define _PLOT_H_INCLUDED


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "types.h"
#include "plot_fnt.h"


#define PLOT3D_AXES_BOXED       0
#define PLOT3D_AXES_FRAMED      1
#define PLOT3D_AXES_NORMAL      2
#define PLOT3D_AXES_NONE        3

#define PLOT3D_COLOR_NO                 0
#define PLOT3D_COLOR_XYZ                1
#define PLOT3D_COLOR_XY                 2
#define PLOT3D_COLOR_Z                  3
#define PLOT3D_COLOR_Z_HUE              4
#define PLOT3D_COLOR_Z_GRAYSCALE        5

#define PLOT3D_PERSPECTIVE_NO           0
#define PLOT3D_PERSPECTIVE_NEAR         1
#define PLOT3D_PERSPECTIVE_MEDIUM       2
#define PLOT3D_PERSPECTIVE_FAR          3

#define PLOT3D_PROJECTION_UNCONSTRAINED     0
#define PLOT3D_PROJECTION_CONSTRAINED       1

#define PLOT3D_STYLE_PATCH                  0
#define PLOT3D_STYLE_PATCH_WITHOUT_GRID     1
#define PLOT3D_STYLE_PATCH_AND_CONTOUR      2
#define PLOT3D_STYLE_HIDDEN_LINE            3
#define PLOT3D_STYLE_CONTOUR                4
#define PLOT3D_STYLE_WIREFRAME              5
#define PLOT3D_STYLE_POINT                  6


struct TPlot2DState {
    int AxesStyle;
    int ConstrainedProjection;
    int PlotStyle;
};

class TPlot2D {

    struct TPlot2DLineStripListItem {
        TRGBColor Color;
        TPoint2D *PointList;
        int Points;
    };

    struct {
        TPlot2DLineStripListItem *ItemList;
        int Size;
        int MaxSize;        
    } PlotLineStripList;

    struct TPlot2DPointListItem {
        TRGBColor Color;
        TPoint2D *PointList;
        int Points;
    };

    struct {
        TPlot2DPointListItem *ItemList;
        int Size;
        int MaxSize;
    } PlotPointList;
        
    struct TPlot2DKeyPointListItem {
        TRGBColor TextColor;
        TPoint2D Point;
        char *Text;
        int TextJustifyWidth;
        int TextJustifyHeight;
    };

    struct {
        TPlot2DKeyPointListItem *ItemList;
        int Size;
        int MaxSize;
    } PlotKeyPointList;

    struct TPlot2DFunctionListItem {
        TDouble Left;
        TDouble Right;
        TDouble (*Func)(TDouble);
        TRGBColor Color;                
        int Points;
        int LineStripListIndex;
    };
    
    struct {
        TPlot2DFunctionListItem *ItemList;
        int Size;
        int MaxSize;
    } PlotFunctionList;

    int ConstrainedProjection;

    int PlotStyle;
    
    TPlotFixedFont Font;

    TPoint2D Min, Max;
    TPoint2D MinMaxEpsilon;
    int InvalidMinMax;

    int AxesStyle;
    TPoint2D AxesPos;
    TPoint2D AxesSize;
    TPoint2D AxesMin, AxesMax;
    TPoint2D AxesCenter;
    TPoint2D AxesLineSize;

    TDouble Plot2DAxisYNameShift;

    TDouble PixelZoomX;
    TDouble PixelZoomY;
        
    int AxisXStepE;
    int AxisXStepM;
    TDouble AxisXScale;

    int AxisYStepE;
    int AxisYStepM;
    TDouble AxisYScale;
    
    int DrawZeroX;
    int DrawZeroY;

    int AxisXNumbersDist;
    int AxisYNumbersDist;
    const char *AxisXName;
    const char *AxisYName;

    struct {
        TDouble Left;
        TDouble Right;
        TDouble Width;
        TDouble Top;
        TDouble Bottom;
        TDouble Height;
    } Frustum;

    struct TZoomHistory {
        TDouble MinX;
        TDouble MaxX;
    };

    struct {
        TZoomHistory *ItemList;
        int Size;
        int MaxSize;        
    } ZoomHistoryList;

    int ZoomEnabled;
    int ZoomLevel;
    int MaxZoom;
    int SelectionMinPixelSize;
    TDouble SelectionMinSize;
                    
    struct {
        int Left;
        int Right;
        int Top;
        int Bottom;     
    } PlotRect;

    int RedrawAll;
    int DisplayAllowed;

    HWND ParentWindow;
    HDC hDC;
    HGLRC hGLRC;
    HINSTANCE hInstance;
    int winWidth, winHeight;
    int MenuFirstCall;
     
    void ComputeAxesPos();
    void ComputeAxesSize();
    int ComputeAxisPos(TDouble, TDouble, TDouble, int, 
                       TDouble &, TDouble &, TDouble &, TDouble &);
    void ComputeFunctions();
    void CorrectSelection(int &, int &);
    void CreatePlot2DWindow(int, int, int, int, LPSTR);
    void DrawAxes();
    void DrawAxis(TDouble, TDouble, TDouble, TDouble, TDouble, TDouble,
                  TPoint2D, TPoint2D, const char *, int, int, int, 
                  int, int, TDouble);
    void DrawMesh();
    void DrawSelection(int, int, int);
    void GetViewportSize(int &, int &);
    void PrepareFrustum();
    void RebuildMesh();
    void RegisterPlot2DClass();
    void Render();
    void SetupPixelFormat(HDC);

public:

    HWND hWnd;

    TPlot2D(HINSTANCE, int, int, int, int, char *, HWND = NULL);
    ~TPlot2D();

    void AttachFunction(TDouble, TDouble, TDouble (*)(TDouble), TRGBColor &, int = 128);
    void AttachKeyPoint(TPoint2D &, char *, int, int, TRGBColor &);
    void AttachListStrip(TPoint2D *, int, TRGBColor &);
    void AttachPoints(TPoint2D *, int, TRGBColor &);
    void Display();
    void EnableZoom(TDouble);
    void Flush();
    void GetMinMax(TPoint2D &, TPoint2D &);
    void LoadState(const TPlot2DState &);

    void SetAxesNames(const char *X, const char *Y)
    {
        AxisXName = X;
        AxisYName = Y;
    }

    void SetConstrainedProjection(int Type)
    {
        ConstrainedProjection = Type;
    }

    void SetMinMax(const TPoint2D &, const TPoint2D &);
    void StoreState(const TPlot2DState &);
    void UnionMinMax(const TPoint2D &, const TPoint2D &);
    
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

class TPlot3DFunction {

    TDouble *ValuesGrid;
    TDouble (*Function)(TDouble, TDouble);

    int GridSizeX;
    int GridSizeY;

    TPoint3D FunctionsMin, FunctionsMax;

    typedef void (*TColoringProc)(TFloat, TFloat, TFloat, TFloat &, TFloat &, TFloat &);
        
    void BuildVertexList(TPoint3D &);
    void BuildVertexColorList(TPoint3D &, int);
    void BuildNormalList(int);
    void BuildFaceList();
    void BuildContourLineList();
    void ComputeColor(TColoringProc, TFloat, TFloat, TFloat, 
                      TBYTE &, TBYTE &, TBYTE &);
    void InsertContourLine(TPoint3D &, TPoint3D &);
    void LineInterpolation(TPoint3D &, TPoint3D &, TDouble, TPoint3D &);

public:

    int MeshVertices;
    TFloat (*MeshVertexList)[3];
    TBYTE (*MeshVertexColorList)[3];
    int MeshFaces;
    int (*MeshFaceList)[4];
    TFloat (*MeshNormalList)[3];
    int MeshContourLines;
    int MeshMaxContourLines;
    TFloat (*MeshContourLineList)[2][3];    
    TBYTE (*MeshContourLineColorList)[2][3];

    TPoint3D Min, Max;

    TPlot3DFunction(TDouble (*)(TDouble, TDouble), const TPoint2D &, const TPoint2D &);
    ~TPlot3DFunction();

    void BuildMesh(TPoint3D &, int, int);
    void BuildValuesGrid();
    void SetGridSize(int, int);
    void SetFunctionsBounds(const TPoint3D &, const TPoint3D &);
};

struct TPlot3DState {
    int AxesStyle;
    int Coloring;
    int ConstrainedProjection;
    int FaceSide;
    int Fog;
    int LightScheme;
    int PerspectiveType;
    int PlotStyle;
};

class TPlot3D {

    TDouble *FunctionValueGrid;

    TPoint3D Min, Max;
    TPoint3D MinMaxEpsilon;
    TPoint3D AxesMin, AxesMax;
    
    TPlot3DFunction **FunctionList;
    int Functions;
    int MaxFunctions;

    int GridSizeX;
    int GridSizeY;

    const char *AxisXName;
    const char *AxisYName;
    const char *AxisZName;

    struct {
        TFloat Alpha;
        TFloat Beta;
        TFloat Gamma;
    } Orientation;

    struct {
        TFloat Left;
        TFloat Top;
    } Frustum;

    TFloat ModelDist;
    TFloat Perspective;

    TPoint3D AxesSize;

    int AxesStyle;
    int Coloring;
    int ConstrainedProjection;
    int FaceSide;
    int Fog;
    int LightScheme;
    int PerspectiveType;
    int PlotStyle;

    int InvalidMinMax;
    int MeshFlushed;

    int DisplayAllowed;
    int RedrawAll;

    int winWidth, winHeight;
    HDC hDC;
    HGLRC hGLRC;
    HWND ParentWindow;
    HINSTANCE hInstance;

    TPlotFixedFont Font;

    TMatrix ModelViewMatrix;
    TMatrix glModelViewMatrix;

    void CreateModelViewMatrix();
    void CreatePlot3DWindow(int, int, int, int, LPSTR);
    void ComputeAxesSize();
    int ComputeAxisPos(TDouble, TDouble, TDouble,
                       TDouble &, TDouble &, TDouble &, TDouble &);
    void ComputeFunctionValueGrid();
    void ComputeMinMax();
    void DrawAxes();
    void DrawAxis(TDouble, TDouble, TDouble, TPoint3D, TPoint3D, const char *, int);
    void DrawContour(int);
    void DrawWireframe(int);
    void GetViewportSize(int &, int &);
    void InitLight();
    void InitMaterial();    
    void SetMeshMinMax();
    void SetupPixelFormat(HDC);
    void RebuildMesh();
    void RegisterPlot3DClass();

    int MenuFirstCall;
    void UpdateMenu(HMENU);
         
public:

    HWND hWnd;

    TPlot3D(HINSTANCE, int, int, int, int, char *, HWND = NULL);
    ~TPlot3D();
    
    void AttachFunction(TDouble (*)(TDouble, TDouble), const TPoint2D &, const TPoint2D &);
    void BuildMesh();

    void Display();
    void Flush();
    void FlushMesh();
    void GetMinMax(TPoint3D &, TPoint3D &);
    void GetOrientation(TFloat &, TFloat &, TFloat &);
    void LoadState(TPlot3DState &);
    void Render();
    
    void SetAxes(int _AxesStyle)
    {
        AxesStyle = _AxesStyle;
    }

    void SetAxesNames(const char *X, const char *Y, const char *Z)
    {
        AxisXName = X;
        AxisYName = Y;
        AxisZName = Z;
    }

    void SetConstrainedProjection(int Type)
    {
        ConstrainedProjection = Type;
    }

    void SetPerspective(TFloat);

    void SetStyle(int _PlotStyle)
    {
        PlotStyle = _PlotStyle;
    }

    void SetGridSize(int, int);
    void SetMinMax(TPoint3D &, TPoint3D &);
    void SetOrientation(TFloat, TFloat, TFloat);
    void SetPerspectiveType(int);
    void StoreState(TPlot3DState &);
    
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};


#endif /* _PLOT_H_INCLUDED */
