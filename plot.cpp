/*                                                       */
/* Copyright (c) 2002,2003 Mad Rain. Fenomen group.      */
/* Simple ploting engine v0.983 (demo). Using OpenGL 1.1 */
/*                                                       */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL/gl.h>
#include "plot.h"
#include "font.h"
#include "resource.h"


#define PI        3.14159265358979323846

#define sqr(a) ((a) * (a))

#define PlotMinMaxEpsilon           0.0001
#define PlotMinNumbersDist          30
#define PlotNumbersDistScale        0.50

#define Plot2DClassName             "Plot2D"
#define Plot2DAxisShift             0.02
#define Plot2DAxisSmallLineSize     2.5
#define Plot2DAxisNormalLineSize    5.5
#define Plot2DAxisTextShift         7
#define Plot2DAxisXNameShift        35
#define Plot2DAxisYNumbersDist      40
#define Plot2DBorderWidth           30
#define Plot2DBorderHeight          20
#define Plot2DSelectionMinSize      20
#define Plot2DSelectionColor        0x7f7f7f
#define Plot2DMaxZoomThreshold      1.6

const TFloat Plot2DBackGroundColor[4] = {1.0, 1.0, 1.0, 1.0};

#define PLOT2D_DRAW_MESH_ON_TOP


#define Plot3DClassName             "Plot3D"
#define Plot3DContourLevels         15
#define Plot3DDragSpeed             0.005f
#define Plot3DAxisShift             0.01
#define Plot3DAxisSmallLineSize     2.5
#define Plot3DAxisNormalLineSize    5.5
#define Plot3DAxisTextShift         14
#define Plot3DAxisNameShift         15
#define Plot3DNumbersDist           45
#define Plot3DFrustumSize           0.92f
#define Plot3DWireframePersOffset   0.004
#define Plot3DWireframeOffset       0.004

const TFloat Plot3DBackGroundColor[4] = {1.0, 1.0, 1.0, 1.0};


static int Plot2DFirstInstance = 1;
static int Plot3DFirstInstance = 1;


#define PlotComputeLeftM(Left, Epsilon, Scale, StepM) \
                    ((int)((ceil(((Left) - (Epsilon)) / (Scale)) * (StepM))))
#define PlotComputeRightM(Right, Epsilon, Scale, StepM) \
                    ((int)((floor(((Right) + (Epsilon)) / (Scale)) * (StepM))))


#define ListInit(List)          List.ItemList = NULL;       \
                                List.Size = 0;              \
                                List.MaxSize = 0

#define ListDone(List)          delete List.ItemList

#define ListExpand(List, Type)  if (List.Size == List.MaxSize)                                      \
                                {                                                                   \
                                    if (List.MaxSize == 0)                                          \
                                        List.MaxSize = 4;                                           \
                                    List.MaxSize *= 2;                                              \
                                    List.ItemList = (Type *) realloc(List.ItemList,                 \
                                                                     sizeof(Type) * List.MaxSize);  \
                                }

struct TPlotWindowClass {
    HWND hWnd;
    void *Class;
    int Type;
    TPlotWindowClass *Next; 
};

static TPlotWindowClass *PlotWindowList = NULL;

static void RegisterPlotClass(HWND hWnd, void *Class, int Type)
{
    TPlotWindowClass *PlotWindowClass = new TPlotWindowClass;
    PlotWindowClass->hWnd = hWnd;
    PlotWindowClass->Class = Class;
    PlotWindowClass->Type = Type;

    if (PlotWindowList)
            PlotWindowClass->Next = PlotWindowList;
    else    PlotWindowClass->Next = NULL;
    PlotWindowList = PlotWindowClass;
}

static void UnRegisterPlotClass(HWND hWnd)
{
    assert(PlotWindowList != NULL);
    TPlotWindowClass *PlotWindowClass = PlotWindowList;
    TPlotWindowClass *PrevPlotWindowClass = NULL;

    while (PlotWindowClass->hWnd != hWnd)
    {
        PrevPlotWindowClass = PlotWindowClass;
        PlotWindowClass =  PlotWindowClass->Next;
        assert(PlotWindowClass != NULL);
    }   

    if (PrevPlotWindowClass) 
    {
        PrevPlotWindowClass->Next = PlotWindowClass->Next;
        delete PlotWindowClass;
    }
    else
    {
        PlotWindowList = PlotWindowClass->Next;
        delete PlotWindowClass;
    }
}

static TPlotWindowClass *FindPlotClass(HWND hWnd)
{
    if (!PlotWindowList)
        return NULL;

    TPlotWindowClass *PlotWindowClass = PlotWindowList;
    while (PlotWindowClass->hWnd != hWnd)
    {
        PlotWindowClass = PlotWindowClass->Next;
        if (!PlotWindowClass)
            return NULL;
    }   

    return PlotWindowClass;             
}

TPlot3DFunction::TPlot3DFunction(TDouble (*Func)(TDouble, TDouble), TPoint2D &Min, TPoint2D &Max)
{
    Function = Func;
    this->Min = Min;
    this->Max = Max;

    ValuesGrid = NULL;
    MeshVertexList = NULL;
    MeshVertexColorList = NULL;
    MeshFaceList = NULL;
    MeshNormalList = NULL;
    MeshContourLines = 0;
    MeshMaxContourLines = 0;
    MeshContourLineList = NULL;
    MeshContourLineColorList = NULL;
}

void TPlot3DFunction::BuildValuesGrid()
{
    delete ValuesGrid;
    ValuesGrid = (TDouble *) new TDouble[(GridSizeY + 1) * (GridSizeX + 1)];
    TDouble StepX = (Max.x - Min.x) / GridSizeX;
    TDouble StepY = (Max.y - Min.y) / GridSizeY;
    TDouble &FuncMin = Min.z;
    TDouble &FuncMax = Max.z;
    for (int i = 0; i <= GridSizeY; i ++)
    {
        TDouble y = StepY * i + Min.y;
        for (int j = 0; j <= GridSizeX; j ++)
        {
            TDouble x = StepX * j + Min.x;
            TDouble Value = ValuesGrid[i * (GridSizeX + 1) + j] = Function(x, y);
            if (i == 0 && j == 0)
                FuncMin = FuncMax = Value;
            else 
            if (Value < FuncMin)
                FuncMin = Value;
            else 
            if (Value > FuncMax)
                FuncMax = Value;
        }
    }
}

void TPlot3DFunction::SetGridSize(int SizeX, int SizeY)
{
    GridSizeX = SizeX;
    GridSizeY = SizeY;
}

void TPlot3DFunction::SetFunctionsBounds(TPoint3D &Min, TPoint3D &Max)
{
    FunctionsMin = Min;
    FunctionsMax = Max;
}

static void Plot3DColoringXYZ(TFloat x, TFloat y, TFloat z,
                              TFloat &r, TFloat &g, TFloat &b)
{
    r = x;
    g = y;
    b = z;
}

static void Plot3DColoringXY(TFloat x, TFloat y, TFloat z,
                             TFloat &r, TFloat &g, TFloat &b)
{
    (void) z;

    r = 1.0f;
    g = y;
    b = x;
}

static void Plot3DColoringZ(TFloat x, TFloat y, TFloat z,
                            TFloat &r, TFloat &g, TFloat &b)
{
    (void) x;
    (void) y;
    
    r = 1.0f - z;
    b = z;
    g = 0.0f;
}

static void Plot3DColoringZ_Hue(TFloat x, TFloat y, TFloat z,
                                TFloat &r, TFloat &g, TFloat &b)
{
    (void) x;
    (void) y;
    
    TFloat h = z;
    h *= 5.0f;
    int i = (int) h;
    TFloat u = h - (TFloat) i;
    TFloat d = (TFloat) i + 1 - h;   

    if (i < 0) i = 0;
    else if (i > 4) i = 4;

    switch (i)
    {
    case 0: r = 1.0f; g = u;    b = 0.0f; break;
    case 1: r = d;    g = 1.0f; b = 0.0f; break;
    case 2: r = 0.0f; g = 1.0f; b = u;    break;
    case 3: r = 0.0f; g = d;    b = 1.0f; break;
    case 4: r = u;    g = 0.0f; b = 1.0f; break;
    }
}

static void Plot3DColoringZ_Greyscale(TFloat x, TFloat y, TFloat z,
                                      TFloat &r, TFloat &g, TFloat &b)
{
    (void) x;
    (void) y;

    r = z;
    g = z;
    b = z;
}

static void Plot3DColoringNo(TFloat x, TFloat y, TFloat z,
                             TFloat &r, TFloat &g, TFloat &b)
{
    (void) x;
    (void) y;
    (void) z;
    
    r = g = b = 0.0f;
}

void TPlot3DFunction::BuildNormalList(int FaceSide)
{
    delete[] MeshNormalList;
    MeshNormalList = new TFloat[MeshVertices][3];
    int i;

    for (i = 0; i < MeshVertices; i ++)
        MeshNormalList[i][0] =
        MeshNormalList[i][1] =
        MeshNormalList[i][2] = 0.0f;

    for (i = 0; i < (GridSizeX + 1) * GridSizeY; i += GridSizeX + 1)
    {
        for (int j = 0; j < GridSizeX; j ++)
        {
            int v0 = i + j;
            int v1 = v0 + 1;
            int v3 = i + GridSizeX + 1 + j;
            int v2 = v3 + 1;

            TFloat v0x = MeshVertexList[v1][0] - MeshVertexList[v0][0]; 
            TFloat v0y = MeshVertexList[v1][1] - MeshVertexList[v0][1]; 
            TFloat v0z = MeshVertexList[v1][2] - MeshVertexList[v0][2]; 
            TFloat v1x = MeshVertexList[v2][0] - MeshVertexList[v0][0]; 
            TFloat v1y = MeshVertexList[v2][1] - MeshVertexList[v0][1]; 
            TFloat v1z = MeshVertexList[v2][2] - MeshVertexList[v0][2]; 

            TFloat n0x = v0y * v1z - v0z * v1y;
            TFloat n0y = v0z * v1x - v0x * v1z;
            TFloat n0z = v0x * v1y - v0y * v1x;
            TFloat norm = 1.0f / (TFloat) sqrt(n0x * n0x + n0y * n0y + n0z * n0z);
            n0x *= norm;
            n0y *= norm;
            n0z *= norm;

            v0x = MeshVertexList[v2][0] - MeshVertexList[v0][0]; 
            v0y = MeshVertexList[v2][1] - MeshVertexList[v0][1]; 
            v0z = MeshVertexList[v2][2] - MeshVertexList[v0][2]; 
            v1x = MeshVertexList[v3][0] - MeshVertexList[v0][0]; 
            v1y = MeshVertexList[v3][1] - MeshVertexList[v0][1]; 
            v1z = MeshVertexList[v3][2] - MeshVertexList[v0][2]; 

            TFloat n1x = v0y * v1z - v0z * v1y;
            TFloat n1y = v0z * v1x - v0x * v1z;
            TFloat n1z = v0x * v1y - v0y * v1x;
            norm = 1.0f / (TFloat) sqrt(n1x * n1x + n1y * n1y + n1z * n1z);
            n1x *= norm;
            n1y *= norm;
            n1z *= norm;
                   
            MeshNormalList[v0][0] += n0x + n1x;     
            MeshNormalList[v0][1] += n0y + n1y;     
            MeshNormalList[v0][2] += n0z + n1z;     

            MeshNormalList[v1][0] += n0x;       
            MeshNormalList[v1][1] += n0y;       
            MeshNormalList[v1][2] += n0z;       

            MeshNormalList[v2][0] += n0x + n1x;     
            MeshNormalList[v2][1] += n0y + n1y;     
            MeshNormalList[v2][2] += n0z + n1z;     

            MeshNormalList[v3][0] += n1x;       
            MeshNormalList[v3][1] += n1y;       
            MeshNormalList[v3][2] += n1z;       
        }
    }

    TFloat Direction = FaceSide ? -1.0f : 1.0f;
    
    for (i = 0; i < MeshVertices; i ++)
    {
        TFloat norm = Direction / (TFloat) sqrt(MeshNormalList[i][0] * MeshNormalList[i][0] +
                                                MeshNormalList[i][1] * MeshNormalList[i][1] +
                                                MeshNormalList[i][2] * MeshNormalList[i][2]);
        MeshNormalList[i][0] *= norm;   
        MeshNormalList[i][1] *= norm;   
        MeshNormalList[i][2] *= norm;   
    }
}

void TPlot3DFunction::BuildFaceList()
{
    MeshFaces = GridSizeX * GridSizeY;
    delete[] MeshFaceList;
    MeshFaceList = new int[MeshFaces][4];
    int (*Face)[4] = MeshFaceList;
    int i, j;
    for (i = 0; i < GridSizeY; i ++)
    {
        for (j = 0; j < GridSizeX; j ++)
        {
            int v00 = (GridSizeX + 1) *  i      + j    ;
            int v01 = (GridSizeX + 1) *  i      + j + 1;
            int v10 = (GridSizeX + 1) * (i + 1) + j    ;
            int v11 = (GridSizeX + 1) * (i + 1) + j + 1;

            Face[0][0] = v00;
            Face[0][1] = v01;
            Face[0][2] = v10;
            Face[0][3] = v11;

            Face ++;
        }
    }
}

void TPlot3DFunction::ComputeColor(TColoringProc ColoringProc, 
                                   TFloat x, TFloat y, TFloat z, 
                                   TBYTE &r, TBYTE &g, TBYTE &b)
{
    if (x < 0.0f) 
        x = 0.0f;
    else if (x > 1.0f)
        x = 1.0f;
    if (y < 0.0f)
        y = 0.0f;
    else if (y > 1.0f)
        y = 1.0f;
    if (z < 0.0f)
        z = 0.0f;
    else if (z > 1.0f)
        z = 1.0f;

    TFloat fr, fb, fg;      
    ColoringProc(x, y, z, fr, fg, fb);
            
    r = (TBYTE) (fr * 250.0f + 2.0f);
    g = (TBYTE) (fg * 250.0f + 2.0f);
    b = (TBYTE) (fb * 250.0f + 2.0f);
}

void TPlot3DFunction::BuildVertexColorList(TPoint3D &AxesSize, int Coloring)
{
    static TColoringProc ColoringProcList[] = {
        Plot3DColoringNo,
        Plot3DColoringXYZ,
        Plot3DColoringXY,
        Plot3DColoringZ,
        Plot3DColoringZ_Hue,
        Plot3DColoringZ_Greyscale
    };
    TColoringProc ColoringProc = ColoringProcList[Coloring];
    int i;

    delete[] MeshVertexColorList;
    MeshVertexColorList = new TBYTE[MeshVertices][3];
    TBYTE (*VertexColor)[3] = MeshVertexColorList;
    TFloat (*Vertex)[3] = MeshVertexList;
    for (i = 0; i < MeshVertices; i ++)
    {
        TFloat x = (TFloat) (Vertex[0][0] / AxesSize.x) / 2.0f + 0.5f;
        TFloat y = (TFloat) (Vertex[0][1] / AxesSize.y) / 2.0f + 0.5f;
        TFloat z = (TFloat) (Vertex[0][2] / AxesSize.z) / 2.0f + 0.5f;
        Vertex ++;

        ComputeColor(ColoringProc, x, y, z, 
                     VertexColor[0][0], VertexColor[0][1], VertexColor[0][2]);
        VertexColor ++;
    }

    delete[] MeshContourLineColorList;
    MeshContourLineColorList = new TBYTE[MeshContourLines][2][3];
    VertexColor = &MeshContourLineColorList[0][0];
    Vertex = &MeshContourLineList[0][0];
    for (i = 0; i < MeshContourLines * 2; i ++)
    {
        ComputeColor(ColoringProc, Vertex[0][0] + 0.5f, Vertex[0][1] + 0.5f, Vertex[0][2] + 0.5f,
                     VertexColor[0][0], VertexColor[0][1], VertexColor[0][2]);
        Vertex ++;
        VertexColor ++;
    }
}

void TPlot3DFunction::LineInterpolation(TPoint3D &p0, TPoint3D &p1, TDouble Level, TPoint3D &p)
{
    p = (p1 - p0) * ((p0.z - Level) / (p0.z - p1.z)) + p0;
}

void TPlot3DFunction::InsertContourLine(TPoint3D &p0, TPoint3D &p1)
{
    if (MeshContourLines == MeshMaxContourLines)
    {
        if (MeshMaxContourLines == 0)
            MeshMaxContourLines = 4;
        MeshMaxContourLines *= 2;
        MeshContourLineList = (TFloat (*)[2][3]) realloc(MeshContourLineList, 
                                   sizeof(TFloat[2][3]) * MeshMaxContourLines);
    }

    TFloat (*ContourLine)[3] = MeshContourLineList[MeshContourLines ++];

    ContourLine[0][0] = (TFloat) p0.x;
    ContourLine[0][1] = (TFloat) p0.y;
    ContourLine[0][2] = (TFloat) p0.z;

    ContourLine[1][0] = (TFloat) p1.x;
    ContourLine[1][1] = (TFloat) p1.y;
    ContourLine[1][2] = (TFloat) p1.z;
}

void TPlot3DFunction::BuildContourLineList()
{
    const TDouble MinLevel = -0.5;
    const TDouble MaxLevel = 0.5;
    const TDouble Epsilon = 0.001;
    const TDouble LevelSize = (MaxLevel - MinLevel) / Plot3DContourLevels;
    const TDouble StartLevel = MinLevel - LevelSize * 0.5;
    static int ContourEdgeFlagTable[4] = {
        0        ,
        1 + 2    ,
        1     + 4,
            2 + 4
    };
    static int ContourEdgeTable[4][2] = {
        {0, 0},
        {1, 0},
        {2, 0},
        {1, 2},
    };
        
    MeshContourLines = 0;

    for (int i = 0; i < GridSizeY; i ++)
    {
        for (int j = 0; j < GridSizeX; j ++)
        {
            TPoint3D V[2][2];
            V[0][0] = MeshVertexList[ i      * (GridSizeX + 1) + j    ];
            V[0][1] = MeshVertexList[ i      * (GridSizeX + 1) + j + 1];
            V[1][0] = MeshVertexList[(i + 1) * (GridSizeX + 1) + j    ];
            V[1][1] = MeshVertexList[(i + 1) * (GridSizeX + 1) + j + 1];

            for (int k = 0; k < 2; k ++)
            {
                TPoint3D T[3];
                if (k == 0)
                {
                    T[0] = V[0][0];
                    T[1] = V[0][1];
                    T[2] = V[1][0];
                }
                else
                {
                    T[0] = V[0][1];
                    T[1] = V[1][1];
                    T[2] = V[1][0];
                }

                TPoint3D *Vp = &T[1];
                TDouble MinV = T[0].z;
                TDouble MaxV = T[0].z;
                for (int h = 1; h < 3; h ++, Vp ++)
                    if (Vp->z < MinV)
                        MinV = Vp->z;
                    else if (Vp->z > MaxV)
                        MaxV = Vp->z;                                                          

                if (MinV < MaxLevel && MaxV > MinLevel)
                {
                    TDouble Level0 = ceil((MinV - StartLevel - Epsilon) / LevelSize);
                    TDouble Level1 = floor((MaxV - StartLevel + Epsilon) / LevelSize);
                    int Levels = (int) (Level1 - Level0) + 1;
                    TDouble Level = Level0 * LevelSize + StartLevel;
               
                    while ((Levels --) > 0)
                    {
                        int Type = 0;
                        if (T[0].z >= Level) Type  = 0x01;
                        if (T[1].z >= Level) Type |= 0x02;
                        if (T[2].z >= Level) Type ^= 0x03;
                                        
                        TPoint3D MVertex[3];
                        int EdgeFlag = ContourEdgeFlagTable[Type];
                        if (EdgeFlag & 1) LineInterpolation(T[0], T[1], Level, MVertex[0]);
                        if (EdgeFlag & 2) LineInterpolation(T[0], T[2], Level, MVertex[1]);
                        if (EdgeFlag & 4) LineInterpolation(T[1], T[2], Level, MVertex[2]);
                    
                        int *EdgeVertex = &ContourEdgeTable[Type][0];
                        if (EdgeVertex[0]) InsertContourLine(MVertex[EdgeVertex[0]], MVertex[EdgeVertex[1]]);
                    
                        Level += LevelSize;
                    }
                }
            }
        }
    }
}

void TPlot3DFunction::BuildVertexList(TPoint3D &AxesSize)
{
    int i, j;
    MeshVertices = (GridSizeX + 1) * (GridSizeY + 1);
    delete[] MeshVertexList;
    MeshVertexList = new TFloat[MeshVertices][3];
    TFloat MinZ = (GLfloat) FunctionsMin.z;
    TFloat ScaleZ = (GLfloat) (1.0 / (FunctionsMax.z - FunctionsMin.z));
    TFloat (*Vertex)[3] = MeshVertexList;
    TFloat StepX = (TFloat) ((Max.x - Min.x) / (FunctionsMax.x - FunctionsMin.x)) / (TFloat) GridSizeX;
    TFloat StepY = (TFloat) ((Max.y - Min.y) / (FunctionsMax.y - FunctionsMin.y)) / (TFloat) GridSizeY;
    TFloat y = (TFloat) ((Min.y - FunctionsMin.y) / (FunctionsMax.y - FunctionsMin.y)) - 0.5f;
    for (i = 0; i <= GridSizeY; i ++)
    {
        TFloat x = (TFloat) ((Min.x - FunctionsMin.x) / (FunctionsMax.x - FunctionsMin.x)) - 0.5f;
        for (j = 0; j <= GridSizeX; j ++)
        {
            TFloat z = (TFloat) ((ValuesGrid[i * (GridSizeX + 1) + j] - MinZ) * ScaleZ) - 0.5f;
            
            Vertex[0][0] = x * (TFloat) AxesSize.x * 2.0f;
            Vertex[0][1] = y * (TFloat) AxesSize.y * 2.0f;
            Vertex[0][2] = z * (TFloat) AxesSize.z * 2.0f;
            Vertex ++;

            x += StepX;
        }
        y += StepY;
    }
}

void TPlot3DFunction::BuildMesh(TPoint3D &AxesSize, int Coloring, int FaceSide)
{
    BuildVertexList(AxesSize);
    BuildFaceList();
    BuildNormalList(FaceSide);
    BuildContourLineList();
    BuildVertexColorList(AxesSize, Coloring);
}

void TPlot3D::Flush()
{
    if (!MeshFlushed)
        FlushMesh();
    SetMeshMinMax();
    DisplayAllowed = 1;
}

void TPlot3D::FlushMesh()
{
    BuildMesh();
    ComputeMinMax();
    MeshFlushed = 1;
}

TPlot3DFunction::~TPlot3DFunction()
{
    delete[] ValuesGrid;
    delete[] MeshVertexList;
    delete[] MeshVertexColorList;
    delete[] MeshFaceList;
    delete[] MeshNormalList;
    delete[] MeshContourLineList;    
    delete[] MeshContourLineColorList;
}

void TPlot3D::SetGridSize(int SizeX, int SizeY)
{
    GridSizeX = SizeX;
    GridSizeY = SizeY;
}

void TPlot3D::SetPerspective(TFloat _Perspective)
{
    Perspective = _Perspective;
    Frustum.Left = -Plot3DFrustumSize;
    Frustum.Top  = -Plot3DFrustumSize;

#ifdef PLOT3D_VIEWPORT_RATIO
    if (winWidth > winHeight)
        Frustum.Left *= (TFloat) winWidth / winHeight;
    else
        Frustum.Top *= (TFloat) winHeight / winWidth;
#endif /* PLOT3D_VIEWPORT_RATIO */

    if (PerspectiveType)
    {
        Frustum.Left *= Perspective;
        Frustum.Top *= Perspective;
    }
}

void TPlot3D::RebuildMesh()
{
    BuildMesh();
    SetMeshMinMax();
}

void TPlot3D::BuildMesh()
{
    int i;

    for (i = 0; i < Functions; i ++)
    {
        FunctionList[i]->SetGridSize(GridSizeX, GridSizeY);
        FunctionList[i]->BuildValuesGrid();
    }
}

void TPlot3D::ComputeMinMax()
{
    TBox3D BBox(FunctionList[0]->Min, FunctionList[0]->Max);
    for (int i = 1; i < Functions; i ++)
        BBox.Union(BBox, TBox3D(FunctionList[i]->Min, FunctionList[i]->Max));
    Min = BBox.Min;
    Max = BBox.Max;
}

void TPlot3D::SetMeshMinMax()
{
    ComputeAxesSize();

    for (int i = 0; i < Functions; i ++)
    {
        FunctionList[i]->SetFunctionsBounds(Min, Max);
        FunctionList[i]->BuildMesh(AxesSize, Coloring, FaceSide);
    }

    MinMaxEpsilon = (Max - Min) * PlotMinMaxEpsilon;
}

void TPlot3D::GetMinMax(TPoint3D &_Min, TPoint3D &_Max)
{
    _Min = Min;
    _Max = Max;
}

void TPlot3D::SetMinMax(TPoint3D &_Min, TPoint3D &_Max)
{
    Min = _Min;
    Max = _Max;
}

void TPlot3D::SetOrientation(TFloat Alpha, TFloat Beta, TFloat Gamma)
{
    Orientation.Alpha = Alpha;
    Orientation.Beta  = Beta;
    Orientation.Gamma = Gamma;
}

void TPlot3D::GetOrientation(TFloat &Alpha, TFloat &Beta, TFloat &Gamma)
{
    Alpha = Orientation.Alpha;
    Beta = Orientation.Beta;
    Gamma = Orientation.Gamma;
}

static void FindScale(TDouble Left, TDouble Right, TDouble PixelSize, int Dist,
                      int &StepE, int &StepM, TDouble &Scale)
{
    double Range = Right - Left;
    int be = (int) floor(log10(Range)) - 2;
    double s = pow(10.0, be + 1);
    double min_d = 1e10;

    for (int i = 0; i < 4; i ++)
    {
        double ts = s * 0.5;

        for (int j = 0; j < 3; j ++)
        {
            double c = Range / ts;
            double d = fabs(floor(PixelSize / c) - Dist);

            if (min_d > d && ((int) floor(PixelSize / c) > 0.0))
            {
                min_d = d;

                switch (j)
                {
                case 0: StepE = be;     StepM = 5; break;
                case 1: StepE = be + 1; StepM = 1; break;
                case 2: StepE = be + 1; StepM = 2; break;
                }
            }
            ts *= 2.0;
        }
        s *= 10.0;
        be ++;
    }

    Scale = pow(10.0, StepE) * StepM;
}

static void Float2Str(char *out, int e, int m)
{
    if (m == 0) 
    {
        out[0] = '0';
        out[1] = '\0';
    }
    else
    {
        if (m < 0) 
            *out ++ = '-', m = -m;
   
        while (m % 10 == 0)
            m = m / 10, e ++;      
        
        int _m = m;
        int Digits = 0;
        while (_m > 10)
            _m /= 10, Digits ++;            

        if (e < -5 || e + Digits > 5)
        {
            while (m > 100)
            {
                e ++;
                m /= 10;
            }
            if (m >= 10) e ++, out[0] = m / 10 + '0', out[1] = '.',
                         out[2] = m % 10 + '0', out += 3;
            else         *out ++ = m + '0';
            out[0] = 'e';
            if (e < 0) out[1] = '-', e = -e; else out[1] = '+';
            out[2] = e / 10 + '0';
            out[3] = e % 10 + '0';
            out[4] = '\0';
        }
        else
        {
            int v = 1, c = 0;
            while (m >= v)
            {
                v *= 10;
                c ++;
            }
            v /= 10;

            if (e <= -c)
            {
                *out ++ = '0';
                if (e < -c) *out ++ = '.';
                for (int i = 0; i < -e - c; i ++)
                    *out ++ = '0';
            }

            while (v > 0)
            {
                int d = m / v;
                if (e == -c) *out ++ = '.';
                e --;
                *out ++ = d + '0';
                m -= d * v;
                v /= 10;
            }

            while (c > -e)
            {
                *out ++ = '0';
                e --;
            }
            *out = '\0';
        }
    }
}

static int ComputeNumbersDist(TPlotFixedFont &Font, TDouble Left, TDouble Right, 
                              TDouble Epsilon, TDouble PixelSize, int &TextWidth)
{
    int NumbersDist = PlotMinNumbersDist;
    int StepE, StepM;
    TDouble Scale;
    char StrValue[32];
    int TextWidth2;
    int TextHeight;
    do 
    {
        FindScale(Left, Right, PixelSize, NumbersDist, StepE, StepM, Scale);
        Float2Str(StrValue, StepE, StepM);
        Font.GetExtentPoint(StrValue, TextWidth, TextHeight);

        int LeftM = PlotComputeLeftM(Left, Epsilon, Scale, StepM);
        Float2Str(StrValue, StepE, LeftM);
        Font.GetExtentPoint(StrValue, TextWidth2, TextHeight);
        if (TextWidth < TextWidth2)
            TextWidth = TextWidth2;
            
        int RightM = PlotComputeRightM(Right, Epsilon, Scale, StepM);
        Float2Str(StrValue, StepE, RightM);
        Font.GetExtentPoint(StrValue, TextWidth2, TextHeight);
        if (TextWidth < TextWidth2)
            TextWidth = TextWidth2;

        NumbersDist ++;

    } while (NumbersDist < 200 &&
             PixelSize * PlotNumbersDistScale - TextWidth * (Right - Left) / Scale < 0);
    
    return NumbersDist;
}

void TPlot3D::ComputeAxesSize()
{
    if (ConstrainedProjection) 
    {
        TPoint3D Delta = Max - Min;
        TDouble DeltaNorm = Delta.x;
        if (Delta.y > DeltaNorm)
            DeltaNorm = Delta.y;
        else if (Delta.z > DeltaNorm)
            DeltaNorm = Delta.z;
        DeltaNorm *= 2.0;
        Delta.x /= DeltaNorm;
        Delta.y /= DeltaNorm;
        Delta.z /= DeltaNorm;
        AxesSize = Delta;
    }
    else
    {
        AxesSize = TPoint3D(0.5, 0.5, 0.5);
    }
}

void TPlot3D::DrawAxis(TDouble Left, TDouble Right, TDouble Epsilon,
                       TPoint3D _Point0, TPoint3D _Point1, 
                       char *AxisName, int DrawZero)
{
    int Line;
    TDouble Value;

    int ViewportWidth;
    int ViewportHeight;
    GetViewportSize(ViewportWidth, ViewportHeight);

    TPoint3D Point0, Point1;
    Point0 = ModelViewMatrix * _Point0;
    Point1 = ModelViewMatrix * _Point1;

    if (PerspectiveType)
    {
        Point0.x /= -Point0.z;
        Point0.y /= -Point0.z;
    }
    Point0.z = -1.0;
    
    if (PerspectiveType)
    {
        Point1.x /= -Point1.z;
        Point1.y /= -Point1.z;
    }
    Point1.z = -1.0;        

    TPoint3D Shift;
    Shift.x = Point1.y - Point0.y;
    Shift.y = Point0.x - Point1.x;
    Shift.z = 0.0;

    TDouble Norm = sqrt(Shift.x * Shift.x + Shift.y * Shift.y);
    Norm = 1.0 / Norm;

    if (Point0.x * Shift.x + Point0.y * Shift.y < 0)
        Norm = -Norm;

    Shift.x *= Norm / ViewportWidth * -Frustum.Left * 2.0;
    Shift.y *= Norm / ViewportHeight * -Frustum.Top * 2.0;

    int StepE, StepM;
    TDouble Scale;
    TDouble PixelSize = sqrt(sqr((Point1.x - Point0.x) * ViewportWidth / Frustum.Left * 0.5) + 
                             sqr((Point1.y - Point0.y) * ViewportHeight / Frustum.Top * 0.5));
    if (PixelSize < 0.5) 
        PixelSize = 0.5;

    int TextWidth, TextHeight;
    int NumbersDist = ComputeNumbersDist(Font, Left, Right, Epsilon, 
                                         PixelSize, TextWidth);
    FindScale(Left, Right, PixelSize, NumbersDist, StepE, StepM, Scale);

    TDouble Scale5 = Scale / 5.0;
    int Lines0_5 = (int) ceil((Left - Epsilon) / Scale5);
    int Lines1_5 = (int) floor((Right + Epsilon) / Scale5);
    int M = PlotComputeLeftM(Left, Epsilon, Scale, StepM);

    TDouble RndLeft = Lines0_5 * Scale5;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -1.0);
    
    glBegin(GL_LINES);
        glVertex2d(Point0.x, Point0.y);
        glVertex2d(Point1.x, Point1.y);
    glEnd();    

    TPoint3D MaxTextShift;
    MaxTextShift.x = 0.0;
    MaxTextShift.y = 0.0;
    MaxTextShift.z = 0.0;

    for (Line = Lines0_5, Value = RndLeft;  
         Line <= Lines1_5; Line ++, Value += Scale5)
    {
        TDouble LineScale = (Line % 5) ? Plot3DAxisSmallLineSize : Plot3DAxisNormalLineSize;

        TPoint3D Point = (Point1 - Point0) *
                         ((Value - Left) / (Right - Left)) + Point0;

        TPoint3D SPoint = Point + Shift * LineScale;

        glBegin(GL_LINES);
            glVertex2d(Point.x, Point.y);
            glVertex2d(SPoint.x, SPoint.y);
        glEnd();

        if ((Line % 5) == 0)
        {
            if (DrawZero || M)
            {
                char StrValue[32];
                Float2Str(StrValue, StepE, M);

                Font.GetExtentPoint(StrValue, TextWidth, TextHeight);

                TPoint3D TextShift; 
                TextShift.x = Shift.x * (TextWidth * 0.5 + Plot3DAxisNormalLineSize + 1);
                TextShift.y = Shift.y * (TextHeight * 0.5 + Plot3DAxisNormalLineSize + 1);
                TextShift.z = 0.0;

                if (sqr(MaxTextShift.x) + sqr(MaxTextShift.y) <
                    sqr(TextShift.x) + sqr(TextShift.y))
                {
                    MaxTextShift.x = TextShift.x + Shift.x * TextWidth * 0.5;
                    MaxTextShift.y = TextShift.y + Shift.y * TextHeight * 0.5;  
                }                

                TPoint3D TPoint = Point + TextShift;
                glRasterPos2d(TPoint.x, TPoint.y);
                Font.SetJustify(TextJustifyCenter, TextJustifyCenter);
                Font.Print(StrValue);
            }   

            M += StepM;
        }
    }

    if (AxisName) 
    {
        TPoint3D Middle = (Point0 + Point1) * 0.5;
        Middle = Middle + Shift * Plot3DAxisNameShift + MaxTextShift;
        glRasterPos2d(Middle.x, Middle.y);
        Font.SetJustify(TextJustifyCenter, TextJustifyCenter);
        Font.Print(AxisName);
    }
}

int TPlot3D::ComputeAxisPos(TDouble Min, TDouble Max, TDouble AxisSize, 
                            TDouble &Pos, TDouble &AxisMin, TDouble &AxisMax, 
                            TDouble &cAxisSize)
{
    if (Min * Max > 0)
    {
        Pos = -AxisSize - Plot3DAxisShift;
        TDouble Delta = (Max - Min) * Plot3DAxisShift;
        AxisMin = Min - Delta;
        AxisMax = Max + Delta;
        cAxisSize = AxisSize + Plot3DAxisShift;

        return 0;
    }   

    Pos = Min / (Min - Max) * AxisSize * 2 - AxisSize;
    AxisMin = Min;
    AxisMax = Max;
    cAxisSize = AxisSize;

    return 1;
}                            

void TPlot3D::DrawAxes()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0, 0.0, 0.0);

    if (AxesStyle == PLOT3D_AXES_NORMAL)
    {
        TPoint3D cAxesSize, O;
        int DrawZeroX = ComputeAxisPos(Min.x, Max.x, AxesSize.x, O.x, AxesMin.x, AxesMax.x, cAxesSize.x);
        int DrawZeroY = ComputeAxisPos(Min.y, Max.y, AxesSize.y, O.y, AxesMin.y, AxesMax.y, cAxesSize.y);
        int DrawZeroZ = ComputeAxisPos(Min.z, Max.z, AxesSize.z, O.z, AxesMin.z, AxesMax.z, cAxesSize.z);
    
        if (DrawZeroX && DrawZeroY) DrawZeroY = 0;
        if (DrawZeroY && DrawZeroZ) DrawZeroY = 0;
        if (DrawZeroX && DrawZeroZ) DrawZeroX = 0;          

        TPoint3D OX0 = O, OX1 = O; OX0.x = -cAxesSize.x; OX1.x = cAxesSize.x;
        TPoint3D OY0 = O, OY1 = O; OY0.y = -cAxesSize.y; OY1.y = cAxesSize.y;
        TPoint3D OZ0 = O, OZ1 = O; OZ0.z = -cAxesSize.z; OZ1.z = cAxesSize.z;

        DrawAxis(AxesMin.x, AxesMax.x, MinMaxEpsilon.x, OX0, OX1, AxisXName, DrawZeroX);
        DrawAxis(AxesMin.y, AxesMax.y, MinMaxEpsilon.y, OY0, OY1, AxisYName, DrawZeroY);
        DrawAxis(AxesMin.z, AxesMax.z, MinMaxEpsilon.z, OZ0, OZ1, AxisZName, DrawZeroZ);
    } else
    if (AxesStyle == PLOT3D_AXES_BOXED || 
        AxesStyle == PLOT3D_AXES_FRAMED) 
    {
        TPoint3D BoxSize = AxesSize + TPoint3D(Plot3DAxisShift, Plot3DAxisShift, Plot3DAxisShift);
        TPoint3D AxesShift = (Max - Min) * Plot3DAxisShift;
        AxesMin = Min - AxesShift;
        AxesMax = Max + AxesShift;
        TPoint3D BoxVertex[8] = {
            TPoint3D(-BoxSize.x, -BoxSize.y, -BoxSize.z),
            TPoint3D( BoxSize.x, -BoxSize.y, -BoxSize.z),
            TPoint3D(-BoxSize.x,  BoxSize.y, -BoxSize.z),
            TPoint3D( BoxSize.x,  BoxSize.y, -BoxSize.z),
            TPoint3D(-BoxSize.x, -BoxSize.y,  BoxSize.z),
            TPoint3D( BoxSize.x, -BoxSize.y,  BoxSize.z),
            TPoint3D(-BoxSize.x,  BoxSize.y,  BoxSize.z),
            TPoint3D( BoxSize.x,  BoxSize.y,  BoxSize.z)
        };
        static int BoxEdge[12][2] = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {3, 7}, {2, 6}
        };  
        /*
        int VertexNeightborEdges[8][3] = {
            {3, 0, 8}, {0, 1, 9}, {2, 3, 11}, {1, 2, 10},
            {7, 4, 8}, {4, 5, 9}, {6, 7, 11}, {5, 6, 10}
        };
        */
        static int BoxFace[6][3] = {
            {0, 4, 2},
            {0, 1, 4},
            {3, 5, 1},
            {7, 3, 2},
            {0, 2, 1},
            {6, 4, 5}
        };  
        static int EdgeNeightborFaces[12][2] = {
            {1, 4}, {2, 4}, {3, 4}, {0, 4},
            {1, 5}, {2, 5}, {3, 5}, {0, 5},
            {0, 1}, {1, 2}, {2, 3}, {3, 0}
        };
        static int EdgeAxis[12] = {
            0, 1, 0, 1,
            0, 1, 0, 1,
            2, 2, 2, 2
        };
        int i;

        TPoint2D ProjectedBoxVertex[8];
        for (i = 0; i < 8; i ++)
        {
            TPoint3D P = ModelViewMatrix * BoxVertex[i];
            TDouble z = (PerspectiveType) ? P.z : (P.z - 100.0);
            ProjectedBoxVertex[i].x = P.x / z;
            ProjectedBoxVertex[i].y = P.y / z;
        }

        int FaceVisibleFlag[6];
        for (i = 0; i < 6; i ++)
        {
            TPoint2D V0 = ProjectedBoxVertex[BoxFace[i][0]];
            TPoint2D V1 = ProjectedBoxVertex[BoxFace[i][1]] - V0;
            TPoint2D V2 = ProjectedBoxVertex[BoxFace[i][2]] - V0;
            FaceVisibleFlag[i] = (V1.y * V2.x - V1.x * V2.y < 0.0);
        }

        int EdgeVisibleFlag[12];
        for (i = 0; i < 12; i ++)
        {
            EdgeVisibleFlag[i] = FaceVisibleFlag[EdgeNeightborFaces[i][0]] + 
                                 FaceVisibleFlag[EdgeNeightborFaces[i][1]];
        }   

        for (int Axis = 0; Axis < 3; Axis ++)
        {
            int AxisEdge = -1;
            TDouble MaxVisibleLength = -1e10;
            for (i = 0; i < 12; i ++)
            {
                if (EdgeVisibleFlag[i] /* == 1 */ && EdgeAxis[i] == Axis)
                {
                    TPoint2D Delta = ProjectedBoxVertex[BoxEdge[i][1]] -
                                     ProjectedBoxVertex[BoxEdge[i][0]];
                    TDouble VisibleLength = Delta.x * Delta.x + Delta.y * Delta.y;
                    if (MaxVisibleLength < VisibleLength)
                    {
                        AxisEdge = i;
                        MaxVisibleLength = VisibleLength;
                    }   
                }
            }

            if (AxisEdge >= 0)
            {
                TPoint3D &P0 = BoxVertex[BoxEdge[AxisEdge][0]];
                TPoint3D &P1 = BoxVertex[BoxEdge[AxisEdge][1]];

                switch (Axis) 
                {
                case 0: DrawAxis(AxesMin.x, AxesMax.x, MinMaxEpsilon.x, P0, P1, AxisXName, 1); break;
                case 1: DrawAxis(AxesMin.y, AxesMax.y, MinMaxEpsilon.y, P0, P1, AxisYName, 1); break;
                case 2: DrawAxis(AxesMin.z, AxesMax.z, MinMaxEpsilon.z, P0, P1, AxisZName, 1); break;
                }

                EdgeVisibleFlag[AxisEdge] = 0;
            }   
        }

        if (AxesStyle == PLOT3D_AXES_BOXED)
        {
            glMatrixMode(GL_MODELVIEW);
            TMatrix glModelViewMatrix = ModelViewMatrix;
            glModelViewMatrix.Transpose();
            glLoadMatrixd((GLdouble *) glModelViewMatrix.m);
        
            for (i = 0; i < 12; i ++)
            {
                if (EdgeVisibleFlag[i]) 
                {
                    int v0 = BoxEdge[i][0];
                    int v1 = BoxEdge[i][1];
                    glBegin(GL_LINES);
                        glVertex3d(BoxVertex[v0].x, BoxVertex[v0].y, BoxVertex[v0].z);
                        glVertex3d(BoxVertex[v1].x, BoxVertex[v1].y, BoxVertex[v1].z);
                    glEnd();
                }
            }
        }
    }
}

void TPlot3D::CreateModelViewMatrix()
{
    TMatrix RotateMatrix;
    RotateMatrix.Rotate(-Orientation.Alpha, -Orientation.Gamma, -Orientation.Beta);

    for (int i = 0; i < 3; i ++) 
    {
        TDouble t = RotateMatrix.m[1][i];
        RotateMatrix.m[1][i] = RotateMatrix.m[2][i];
        RotateMatrix.m[2][i] = -t;
    }

    TMatrix Matrix;
    Matrix.Identity();
    Matrix.Translate(TPoint3D(0.0, 0.0, ModelDist = 1.0f / Perspective));
    ModelViewMatrix = Matrix * RotateMatrix;
    glModelViewMatrix = ModelViewMatrix;
    glModelViewMatrix.Transpose();
}

void TPlot3D::InitLight()
{   
    glMatrixMode(GL_MODELVIEW);

    if (LightScheme % 2)
    {
        glLoadIdentity();
    }
    else
    {
        TMatrix RotateMatrix;
        RotateMatrix.Rotate(Orientation.Alpha, Orientation.Beta, Orientation.Gamma);        
        RotateMatrix.Transpose();
        // glLoadMatrixd((GLdouble *) RotateMatrix.m);
        glLoadMatrixd((GLdouble *) glModelViewMatrix.m);
    }

    // glTranslated(0.0, 0.0, -ModelDist);

    if (LightScheme < 3)
    {
        float Position0[4] = {0.0f, 0.0f, -2.0f, 0.0f};
        float Ambient0[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float Diffuse0[4] = {0.3f, 1.0f, 0.0f, 1.0f};
        float Specular0[4] = {1.0f, 1.0f, 1.0f, 1.0f};   

        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, Position0);
        glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, Specular0);

        float Position1[4] = {0.0f, 0.0f, 2.0f, 0.0f};
        float Ambient1[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float Diffuse1[4] = {1.0f, 0.3f, 0.0f, 1.0f};
        float Specular1[4] = {1.0f, 1.0f, 1.0f, 1.0f};   

        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, Position1);
        glLightfv(GL_LIGHT1, GL_AMBIENT, Ambient1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, Diffuse1);
        glLightfv(GL_LIGHT1, GL_SPECULAR, Specular1);
    }
    else
    {
        float Position0[4] = {0.0f, 0.0f, -2.0f, 0.0f};
        float Ambient0[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float Diffuse0[4] = {0.4f, 0.2f, 0.9f, 1.0f};
        float Specular0[4] = {1.0f, 1.0f, 1.0f, 1.0f};   

        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, Position0);
        glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, Specular0);
#if 0
        float Position1[4] = {1.75f, -2.0f, 2.0f, 0.0f};
        float Ambient1[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float Diffuse1[4] = {0.7f, 0.6f, 0.2f, 1.0f};
        float Specular1[4] = {0.3f, 1.0f, 0.3f, 1.0f};   

        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, Position1);
        glLightfv(GL_LIGHT1, GL_AMBIENT, Ambient1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, Diffuse1);
        glLightfv(GL_LIGHT1, GL_SPECULAR, Specular1);
#endif
        float Position2[4] = {0.0f, -2.0f, 2.0f, 0.0f};
        float Ambient2[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float Diffuse2[4] = {0.7f, 0.6f, 0.2f, 1.0f};
        float Specular2[4] = {1.0f, 1.0f, 1.0f, 1.0f};   

        glEnable(GL_LIGHT2);
        glLightfv(GL_LIGHT2, GL_POSITION, Position2);
        glLightfv(GL_LIGHT2, GL_AMBIENT, Ambient2);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, Diffuse2);
        glLightfv(GL_LIGHT2, GL_SPECULAR, Specular2);

        float Position3[4] = {0.0f, 0.0f, 3.0f, 0.0f};
        float Ambient3[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float Diffuse3[4] = {0.5f, 0.3f, 0.9f, 1.0f};
        float Specular3[4] = {1.0f, 1.0f, 1.0f, 1.0f};   

        glEnable(GL_LIGHT3);
        glLightfv(GL_LIGHT3, GL_POSITION, Position3);
        glLightfv(GL_LIGHT3, GL_AMBIENT, Ambient3);
        glLightfv(GL_LIGHT3, GL_DIFFUSE, Diffuse3);
        glLightfv(GL_LIGHT3, GL_SPECULAR, Specular3);
    }

    /*
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();    
    glColor3f(0.5, 0.5, 0.5);
    for (int Light = GL_LIGHT0; Light <= GL_LIGHT7; Light ++)
    {
        if (glIsEnabled(Light))
        {
            TFloat LightPos[4];
            glGetLightfv(Light, GL_POSITION, LightPos);
            glBegin(GL_LINES);
                glVertex3fv(LightPos);
                glVertex3f(0.0, 0.0, 0.0);
            glEnd();
        }   
    }
    */
}

void TPlot3D::InitMaterial()
{
    TFloat Ambient[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    TFloat Diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    TFloat Specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};   
    TFloat ShininessTable[4] = {30.0f, 30.0f, 80.0f, 80.0f};
    TFloat Shininess = ShininessTable[LightScheme - 1];
    TFloat Shininess2 = 0.0f;
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
    glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
    glMaterialf(GL_BACK, GL_SHININESS, Shininess2);
}

void TPlot3D::DrawContour(int Coloring)
{
    int MeshContourLines;
    TFloat (*MeshContourLineList)[2][3];
    TBYTE (*MeshContourLineColorList)[2][3];

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
        for (int FuncIndex = 0; FuncIndex < Functions; FuncIndex ++)
        {
            MeshContourLineList = FunctionList[FuncIndex]->MeshContourLineList;
            MeshContourLines = FunctionList[FuncIndex]->MeshContourLines;
            MeshContourLineColorList = FunctionList[FuncIndex]->MeshContourLineColorList;
                                
            for (int i = 0; i < MeshContourLines; i ++)
            {
                if (Coloring) glColor3ubv((GLubyte *) &MeshContourLineColorList[i][0][0]);
                glVertex3fv((GLfloat *) &MeshContourLineList[i][0]);
                if (Coloring) glColor3ubv((GLubyte *) &MeshContourLineColorList[i][1][0]);
                glVertex3fv((GLfloat *) &MeshContourLineList[i][1]);
            }
        }
    glEnd();
}

void TPlot3D::DrawWireframe(int Coloring)
{
    GLfloat (*MeshVertexList)[3];
    TBYTE (*MeshVertexColorList)[3];
    int i, j;

    for (int FuncIndex = 0; FuncIndex < Functions; FuncIndex ++)
    {
        MeshVertexList = FunctionList[FuncIndex]->MeshVertexList;
        MeshVertexColorList = FunctionList[FuncIndex]->MeshVertexColorList;

        GLfloat (*Vertex)[3] = MeshVertexList;
        TBYTE (*VertexColor)[3] = MeshVertexColorList;
        glColor3f(0.0, 0.0, 0.0);
        for (i = 0; i <= GridSizeY; i ++)
        {
            glBegin(GL_LINE_STRIP);
                for (j = 0; j <= GridSizeX; j ++, Vertex ++, VertexColor ++)
                {
                    if (Coloring) glColor3ubv((GLubyte *) &VertexColor[0][0]);
                    glVertex3fv((GLfloat *) &Vertex[0][0]);
                }
            glEnd();
        }

        int n = GridSizeX + 1;
        GLfloat (*Vertex0)[3] = MeshVertexList;
        TBYTE (*VertexColor0)[3] = MeshVertexColorList;
        for (i = 0; i <= GridSizeX; i ++, Vertex0 ++, VertexColor0 ++)
        {
            GLfloat (*Vertex)[3] = Vertex0;
            TBYTE (*VertexColor)[3] = VertexColor0;
            glBegin(GL_LINE_STRIP);
                for (j = 0; j <= GridSizeY; j ++, Vertex += n, VertexColor += n)
                {
                    if (Coloring) glColor3ubv((GLubyte *) &VertexColor[0][0]);
                    glVertex3fv((GLfloat *) &Vertex[0][0]);
                }
            glEnd();
        }
    }
}

void TPlot3D::Render()
{
    GLfloat (*MeshVertexList)[3];
    int MeshVertices;
    TBYTE (*MeshVertexColorList)[3];
    int (*MeshFaceList)[4];
    TFloat (*MeshNormalList)[3];
    int i, FuncIndex;

    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    CreateModelViewMatrix();

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);

    if (LightScheme)
    {
        InitLight();
        InitMaterial();
        glEnable(GL_LIGHTING);
    }
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    SetPerspective(Perspective);
    TDouble FrustumNear = 0.2;
    if (PerspectiveType == 0)
    {
        glOrtho(Frustum.Left, -Frustum.Left,
                Frustum.Top, -Frustum.Top,
                FrustumNear, 10.0);        
    }
    else
    {
        glFrustum(Frustum.Left * FrustumNear, -Frustum.Left * FrustumNear, 
                  Frustum.Top  * FrustumNear, -Frustum.Top  * FrustumNear,
                  FrustumNear, 10.0);
    }

    glPushMatrix();

    TMatrix glModelViewMatrix = ModelViewMatrix;
    glMatrixMode(GL_MODELVIEW);
    glModelViewMatrix.Transpose();
    glLoadMatrixd((GLdouble *) glModelViewMatrix.m);

    if (Fog)
    {
        TFloat FogColor[4] = {1.0, 1.0, 1.0, 1.0};
     
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, (TFloat) ModelDist - 0.5f);
        glFogf(GL_FOG_END, (TFloat) ModelDist + 0.5f);
        glFogfv(GL_FOG_COLOR, FogColor);
    }

    TPoint3D Clip = AxesSize * 1.002;

    TDouble LeftPlane[4]   = {-1.0,  0.0,  0.0, Clip.x};
    TDouble RightPlane[4]  = { 1.0,  0.0,  0.0, Clip.x};
    TDouble TopPlane[4]    = { 0.0, -1.0,  0.0, Clip.y};
    TDouble BottomPlane[4] = { 0.0,  1.0,  0.0, Clip.y};
    TDouble FrontPlane[4]  = { 0.0,  0.0, -1.0, Clip.z};
    TDouble BackPlane[4]   = { 0.0,  0.0,  1.0, Clip.z};

    glEnable(GL_CLIP_PLANE0); glClipPlane(GL_CLIP_PLANE0, LeftPlane);
    glEnable(GL_CLIP_PLANE1); glClipPlane(GL_CLIP_PLANE1, RightPlane);
    glEnable(GL_CLIP_PLANE2); glClipPlane(GL_CLIP_PLANE2, TopPlane);
    glEnable(GL_CLIP_PLANE3); glClipPlane(GL_CLIP_PLANE3, BottomPlane);
    glEnable(GL_CLIP_PLANE4); glClipPlane(GL_CLIP_PLANE4, FrontPlane);
    glEnable(GL_CLIP_PLANE5); glClipPlane(GL_CLIP_PLANE5, BackPlane);

    glDepthFunc(GL_LESS);

    if (PlotStyle == PLOT3D_STYLE_POINT)
    {
        glBegin(GL_POINTS);
            for (FuncIndex = 0; FuncIndex < Functions; FuncIndex ++)
            {
                MeshVertexList = FunctionList[FuncIndex]->MeshVertexList;
                MeshVertices = FunctionList[FuncIndex]->MeshVertices;
                MeshVertexColorList = FunctionList[FuncIndex]->MeshVertexColorList;

                TFloat (*Vertex)[3] = MeshVertexList;
                TBYTE (*VertexColor)[3] = MeshVertexColorList;
                for (i = 0; i < MeshVertices; i ++, Vertex ++, VertexColor ++)
                {
                    glColor3ubv((GLubyte *) &VertexColor[0][0]);
                    glVertex3fv((GLfloat *) &Vertex[0][0]);
                }
            }
        glEnd();
    }

    if (PlotStyle == PLOT3D_STYLE_WIREFRAME)
        DrawWireframe(Coloring);          

    if (PlotStyle == PLOT3D_STYLE_CONTOUR)
        DrawContour(Coloring);
        
    if (PlotStyle == PLOT3D_STYLE_HIDDEN_LINE ||
        PlotStyle == PLOT3D_STYLE_PATCH ||
        PlotStyle == PLOT3D_STYLE_PATCH_AND_CONTOUR ||
        PlotStyle == PLOT3D_STYLE_PATCH_WITHOUT_GRID)
    {
        int UsePatchColor;
        int UseGrid = 1;
        int UseContour = 0;
        
        switch (PlotStyle)
        {
        case PLOT3D_STYLE_HIDDEN_LINE:        UsePatchColor = 0;                              break;
        case PLOT3D_STYLE_PATCH:              UsePatchColor = 1;                              break;
        case PLOT3D_STYLE_PATCH_WITHOUT_GRID: UsePatchColor = 1; UseGrid = 0;                 break;
        case PLOT3D_STYLE_PATCH_AND_CONTOUR:  UsePatchColor = 1; UseGrid = 0; UseContour = 1; break;
        }

        for (FuncIndex = 0; FuncIndex < Functions; FuncIndex ++)
        {
            MeshVertexList = FunctionList[FuncIndex]->MeshVertexList;
            MeshVertices = FunctionList[FuncIndex]->MeshVertices;
            MeshVertexColorList = FunctionList[FuncIndex]->MeshVertexColorList;
            MeshFaceList = FunctionList[FuncIndex]->MeshFaceList;
            MeshNormalList = FunctionList[FuncIndex]->MeshNormalList;

            if (UsePatchColor)
            {
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(3, GL_UNSIGNED_BYTE, 0, MeshVertexColorList);
            }
            else
            {
                glColor3f(1.0f, 1.0f, 1.0f);
            }

            if (LightScheme)
            {
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(GL_FLOAT, 0, MeshNormalList);
            }

            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, MeshVertexList);
            for (int Offset = 0; Offset < (GridSizeX) * GridSizeY; Offset += GridSizeX)
                glDrawElements(GL_TRIANGLE_STRIP, GridSizeX * 4, GL_UNSIGNED_INT, MeshFaceList + Offset);

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
        }

        glDisable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        if (PerspectiveType)
        {
            TDouble ProjectionMatrix[4][4];
            glGetDoublev(GL_PROJECTION_MATRIX, (GLdouble *) &ProjectionMatrix[0][0]);
            ProjectionMatrix[3][2] -= Plot3DWireframePersOffset;
            glLoadMatrixd((GLdouble *) &ProjectionMatrix[0][0]);
        }
        else
        {
            glTranslated(0.0, 0.0, Plot3DWireframeOffset);
        }

        glDepthFunc(GL_LEQUAL);
        
        if (UseGrid) 
            DrawWireframe(!UsePatchColor && Coloring);

        if (UseContour)
            DrawContour(0);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);

    glDisable(GL_FOG);

    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);

    DrawAxes();
}

void TPlot3D::AttachFunction(TDouble (*Func)(TDouble, TDouble), TPoint2D &Min, TPoint2D &Max)
{
    if (Functions == MaxFunctions) 
    {
        if (MaxFunctions == 0)
            MaxFunctions = 2;
        MaxFunctions *= 2;
        FunctionList = (TPlot3DFunction **) 
                       realloc(FunctionList, sizeof(TPlot3DFunction *) * MaxFunctions);
    }
    FunctionList[Functions] = new TPlot3DFunction(Func, Min, Max);
    Functions ++;
}

void TPlot3D::GetViewportSize(int &Width, int &Height)
{
    GLint i[4];
    glGetIntegerv(GL_VIEWPORT, i);
    Width = i[2];
    Height = i[3];
}

void TPlot3D::SetupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int SelectedPixelFormat;
    BOOL retVal;

    memset(&pfd, 0, sizeof(pfd));
    
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER |
                  PFD_SUPPORT_OPENGL |
                  PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cAccumBits = 0;
    pfd.cStencilBits = 0;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    SelectedPixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (!SelectedPixelFormat) 
    {
        MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed\n",
                   "Error", MB_ICONERROR | MB_OK);
        exit(1);
    }

    retVal = SetPixelFormat(hDC, SelectedPixelFormat, &pfd);
    if (retVal != TRUE) 
    {
        MessageBox(WindowFromDC(hDC), "SetPixelFormat failed",
                   "Error", MB_ICONERROR | MB_OK);
        exit(1);
    }
}

void TPlot3D::Display()
{
    if (!DisplayAllowed)
        return; 

    wglMakeCurrent(hDC, hGLRC);
    // if (RedrawAll)
    {
        RedrawAll = 0;

        glViewport(0, 0, winWidth, winHeight);
        glClearColor(Plot3DBackGroundColor[0], 
                     Plot3DBackGroundColor[1], 
                     Plot3DBackGroundColor[2], 
                     Plot3DBackGroundColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Render();
        glFlush();
    }
    SwapBuffers(hDC);
}

static LRESULT APIENTRY
PlotWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TPlotWindowClass *PlotClass = FindPlotClass(hWnd);

    if (PlotClass)
    {
        if (PlotClass->Type == 0)
        {
            TPlot2D *Plot2D = (TPlot2D *) PlotClass->Class;
            return Plot2D->WndProc(hWnd, message, wParam, lParam);
        }
        else
        {
            TPlot3D *Plot3D = (TPlot3D *) PlotClass->Class;
            return Plot3D->WndProc(hWnd, message, wParam, lParam);
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void TPlot3D::StoreState(TPlot3DState &State)
{
    State.AxesStyle = AxesStyle;
    State.ConstrainedProjection = ConstrainedProjection;
    State.Coloring = Coloring;
    State.FaceSide = FaceSide;
    State.Fog = Fog;
    State.LightScheme = LightScheme;
    State.PerspectiveType = PerspectiveType;
    State.PlotStyle = PlotStyle;
}

void TPlot3D::LoadState(TPlot3DState &State)
{
    AxesStyle = State.AxesStyle;
    ConstrainedProjection = State.ConstrainedProjection;
    Coloring = State.Coloring;
    FaceSide = State.FaceSide;
    Fog = State.Fog;
    LightScheme = State.LightScheme;
    PerspectiveType = State.PerspectiveType;
    PlotStyle = State.PlotStyle;
}

void TPlot3D::UpdateMenu(HMENU hMenu)
{
    static int MenuPlotStyle;
    static int MenuAxesStyle;
    static int MenuLightScheme;
    static int MenuConstrainedProjection;
    static int MenuPerspectiveType;
    static int MenuColoring;
        
    if (!MenuFirstCall)
    {
        CheckMenuItem(hMenu, ID_PLOT3D_STYLE_FIRST + MenuPlotStyle, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_PLOT3D_AXES_FIRST + MenuAxesStyle, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_PLOT3D_LIGHT_FIRST + MenuLightScheme, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_PLOT3D_PROJECTION_UNCONSTRAINED + MenuConstrainedProjection, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_PLOT3D_PROJECTION_PERSPECTIVE_FIRST + MenuPerspectiveType, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_PLOT3D_COLOR_FIRST + MenuColoring, MF_UNCHECKED);
    }
    else
    {
        MenuFirstCall = 0;
    }

    CheckMenuItem(hMenu, ID_PLOT3D_STYLE_FIRST + PlotStyle, MF_CHECKED);
    CheckMenuItem(hMenu, ID_PLOT3D_AXES_FIRST + AxesStyle, MF_CHECKED);
    CheckMenuItem(hMenu, ID_PLOT3D_LIGHT_FIRST + LightScheme, MF_CHECKED);
    CheckMenuItem(hMenu, ID_PLOT3D_PROJECTION_UNCONSTRAINED + ConstrainedProjection, MF_CHECKED);
    CheckMenuItem(hMenu, ID_PLOT3D_PROJECTION_PERSPECTIVE_FIRST + PerspectiveType, MF_CHECKED);
    CheckMenuItem(hMenu, ID_PLOT3D_COLOR_FIRST + Coloring, MF_CHECKED);
    CheckMenuItem(hMenu, ID_PLOT3D_COLOR_FOG, Fog ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hMenu, ID_PLOT3D_LIGHT_SIDE, FaceSide ? MF_CHECKED : MF_UNCHECKED);
    
    MenuPlotStyle = PlotStyle;
    MenuAxesStyle = AxesStyle;
    MenuLightScheme = LightScheme;
    MenuConstrainedProjection = ConstrainedProjection;
    MenuPerspectiveType = PerspectiveType;
    MenuColoring = Coloring;
}

void TPlot3D::SetPerspectiveType(int Type)
{
    static TFloat PerspectiveTypeTable[4] = {0.8f, 0.8f, 0.5f, 0.2f};
    PerspectiveType = Type;
    SetPerspective(PerspectiveTypeTable[Type]);
}

LRESULT TPlot3D::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
    static HMENU hMenu;
    static HMENU hMainMenu;
    static int DragX, DragY;
    static int Drag = 0;

    switch (message) 
    {
    case WM_CREATE:
        hDC = GetDC(hWnd);
        SetupPixelFormat(hDC);
        hGLRC = wglCreateContext(hDC);
        hMenu = LoadMenu(hInstance, (LPSTR) MAKEINTRESOURCE(IDR_PLOT3D_MENU));
        hMainMenu = GetSubMenu(hMenu, NULL);
        MenuFirstCall = 1;
        DisplayAllowed = 0;
        RedrawAll = 1;
        break;
            
    case WM_DESTROY:
        if (hGLRC) 
        {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hGLRC);
        }
        ReleaseDC(hWnd, hDC);
        break;
            
    case WM_SIZE:
        if (hGLRC) 
        {
            winWidth = (int) LOWORD(lParam);
            winHeight = (int) HIWORD(lParam);
            RedrawAll = 1;
        }
        break;

    case WM_PAINT:
        PAINTSTRUCT ps;          
        BeginPaint(hWnd, &ps);
        if (hGLRC) 
            Display();
        EndPaint(hWnd, &ps);     
        break;

    case WM_COMMAND:
        {
            int Update = 0;
            int Param = (int) wParam;

            if (Param >= ID_PLOT3D_STYLE_FIRST &&
                Param <= ID_PLOT3D_STYLE_LAST)
            {
                if (PlotStyle != Param - ID_PLOT3D_STYLE_FIRST)
                {
                    PlotStyle = Param - ID_PLOT3D_STYLE_FIRST, Update = 1;
                    if (PlotStyle != PLOT3D_STYLE_PATCH &&
                        PlotStyle != PLOT3D_STYLE_PATCH_WITHOUT_GRID &&
                        PlotStyle != PLOT3D_STYLE_PATCH_AND_CONTOUR)
                            LightScheme = 0;
                }
            } 
            else
            if (Param >= ID_PLOT3D_AXES_FIRST &&
                Param <= ID_PLOT3D_AXES_LAST)
            {
                if (AxesStyle != Param - ID_PLOT3D_AXES_FIRST) 
                    AxesStyle = Param - ID_PLOT3D_AXES_FIRST, Update = 1;
            }
            else
            if (Param == ID_PLOT3D_LIGHT_SIDE) 
            {
                FaceSide ^= 1;
                Update = 1;
            }
            else
            if (Param >= ID_PLOT3D_LIGHT_FIRST &&
                Param <= ID_PLOT3D_LIGHT_LAST)
            {
                if (LightScheme != Param - ID_PLOT3D_LIGHT_FIRST) 
                {
                    LightScheme = Param - ID_PLOT3D_LIGHT_FIRST, Update = 1;
                    if (LightScheme)
                    {
                        if (PlotStyle != PLOT3D_STYLE_PATCH &&
                            PlotStyle != PLOT3D_STYLE_PATCH_WITHOUT_GRID &&
                            PlotStyle != PLOT3D_STYLE_PATCH_AND_CONTOUR)
                                PlotStyle = PLOT3D_STYLE_PATCH_WITHOUT_GRID;
                                // Coloring = 0;
                    }
                }
            }
            else
            if (Param == ID_PLOT3D_PROJECTION_CONSTRAINED)
            {
                if (!ConstrainedProjection)
                    ConstrainedProjection = 1, Update = 1;  
            }
            else
            if (Param == ID_PLOT3D_PROJECTION_UNCONSTRAINED)
            {
                if (ConstrainedProjection)
                    ConstrainedProjection = 0, Update = 1;  
            }
            else
            if (Param >= ID_PLOT3D_COLOR_FIRST &&
                Param <= ID_PLOT3D_COLOR_LAST)
            {
                if (Coloring != Param - ID_PLOT3D_COLOR_FIRST)
                {
                    Coloring = Param - ID_PLOT3D_COLOR_FIRST, Update = 1;                   
                    if (Param != ID_PLOT3D_COLOR_NO)
                        LightScheme = 0;
                }
            }
            else
            if (Param == ID_PLOT3D_COLOR_FOG)
            {
                Fog ^= 1;
                Update = 1;
            }
            else
            if (Param >= ID_PLOT3D_PROJECTION_PERSPECTIVE_FIRST &&
                Param <= ID_PLOT3D_PROJECTION_PERSPECTIVE_LAST)
            {
                if (PerspectiveType != Param - ID_PLOT3D_PROJECTION_PERSPECTIVE_FIRST) 
                {
                    PerspectiveType = Param - ID_PLOT3D_PROJECTION_PERSPECTIVE_FIRST;
                    SetPerspectiveType(PerspectiveType);
                    Update = 1;
                }
            }
            
            if (Update)
            {
                RedrawAll = 1;
                RebuildMesh();
                Display();
            }
        }
        break;
        
    case WM_CONTEXTMENU:
        {
            POINT MenuPoint;
            MenuPoint.x = (short) LOWORD(lParam);
            MenuPoint.y = (short) HIWORD(lParam);
            UpdateMenu(hMainMenu);
            TrackPopupMenu(hMainMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
                           MenuPoint.x, MenuPoint.y, 0, hWnd, NULL);
            break;
        }

    case WM_LBUTTONDOWN:
        Drag = 1;
        DragX = (short) LOWORD(lParam);
        DragY = (short) HIWORD(lParam);
        SetCapture(hWnd);
        break;

    case WM_LBUTTONUP:
        if (Drag)
        {
            Drag = 0;
            ReleaseCapture();
        }
        break;

    case WM_MOUSEMOVE:
        if (Drag)
        {
            int x = (short) LOWORD(lParam);
            int y = (short) HIWORD(lParam);
            Orientation.Alpha -= (y - DragY) * Plot3DDragSpeed;
            Orientation.Beta  -= (x - DragX) * Plot3DDragSpeed;
            DragX = x; DragY = y;
            RedrawAll = 1;
            Display();
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void TPlot3D::RegisterPlot3DClass()
{
    if (Plot3DFirstInstance)
    {
        Plot3DFirstInstance = 0;

        WNDCLASS wndClass;
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = PlotWndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = NULL;
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = Plot3DClassName;
    
        RegisterClass(&wndClass);
    }
}

void TPlot3D::CreatePlot3DWindow(int winX, int winY, 
                                 int _winWidth, int _winHeight,
                                 char *winName)
{
    winWidth = _winWidth;
    winHeight = _winHeight;
    
    hWnd = CreateWindowEx(WS_EX_TRANSPARENT, Plot3DClassName,
                        winName,
                        WS_OVERLAPPED
						/*|
                        WS_CHILD | 
                        WS_CLIPCHILDREN |
                        WS_CLIPSIBLINGS*/,
                        winX, winY,
                        winWidth, winHeight,
                        ParentWindow,   
                        NULL,           
                        hInstance,
                        NULL);
    
    RegisterPlotClass(hWnd, this, 1);
    WndProc(hWnd, WM_CREATE, 0, 0);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
}

TPlot3D::TPlot3D(HINSTANCE _hInstance, int winX, int winY, 
                 int winWidth, int winHeight, char *winName, HWND _ParentWindow)
{
    hInstance = _hInstance;
    ParentWindow = _ParentWindow;
        
    hGLRC = NULL;

    Functions = 0;
    MaxFunctions = 0;
    FunctionList = NULL;
    MeshFlushed = 0;
    
    GridSizeX = 20;
    GridSizeY = 20;

    AxesStyle = PLOT3D_AXES_NORMAL;
    ConstrainedProjection = 0;
    Coloring = PLOT3D_COLOR_Z_HUE;
    FaceSide = 0;
    Fog = 0;
    LightScheme = 0;
    PlotStyle = PLOT3D_STYLE_HIDDEN_LINE;
    
    SetOrientation(-PI * 0.25, -PI * 0.25, 0.0);
    SetPerspectiveType(PLOT3D_PERSPECTIVE_NO);
    SetAxesNames("x", "y", "z");

    InvalidMinMax = 1;
    
    Font.Load(PlotFixedFont);

    RegisterPlot3DClass();
    CreatePlot3DWindow(winX, winY, winWidth, winHeight, winName);
}

TPlot3D::~TPlot3D()
{
    DestroyWindow(hWnd);
    UnRegisterPlotClass(hWnd);
    hWnd = NULL;

    for (int i = 0; i < Functions; i ++)
        delete FunctionList[i];
    delete FunctionList;
}


void TPlot2D::RegisterPlot2DClass()
{             
    if (Plot2DFirstInstance)
    {
        Plot2DFirstInstance = 0;

        WNDCLASS wndClass;
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = PlotWndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = NULL;
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = Plot2DClassName;
    
        RegisterClass(&wndClass);
    }
}

void TPlot2D::CreatePlot2DWindow(int winX, int winY, 
                                 int _winWidth, int _winHeight,
                                 char *winName)
{
    winWidth = _winWidth;
    winHeight = _winHeight;
    
    hWnd = CreateWindow(Plot2DClassName,
                        winName,
                        // WS_OVERLAPPEDWINDOW |
                        WS_CHILD | 
                        WS_CLIPCHILDREN |
                        WS_CLIPSIBLINGS,
                        winX, winY,
                        winWidth, winHeight,
                        ParentWindow,   
                        NULL,           
                        hInstance,
                        NULL);
    
    RegisterPlotClass(hWnd, this, 0);
    WndProc(hWnd, WM_CREATE, 0, 0);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
}

void TPlot2D::SetupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int SelectedPixelFormat;
    BOOL retVal;

    memset(&pfd, 0, sizeof(pfd));
    
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER |
                  PFD_SUPPORT_OPENGL |
                  PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 0;
    pfd.cAccumBits = 0;
    pfd.cStencilBits = 0;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    SelectedPixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (!SelectedPixelFormat) 
    {
        MessageBox(NULL, "ChoosePixelFormat failed",
                   "Error", MB_ICONERROR | MB_OK);
        exit(1);
    }

    retVal = SetPixelFormat(hDC, SelectedPixelFormat, &pfd);
    if (retVal != TRUE) 
    {
        MessageBox(NULL, "SetPixelFormat failed",
                   "Error", MB_ICONERROR | MB_OK);
        exit(1);
    }
}

void TPlot2D::CorrectSelection(int &Left, int &Right)
{
    if (abs(Left - Right) >= SelectionMinPixelSize)
        return;

    if (Left <= Right)
    {
        int Delta = SelectionMinPixelSize - (Right - Left);
        if (PlotRect.Right >= Right + Delta)
            Right += Delta;                     
        else 
        {
            Right = PlotRect.Right;
            int Delta = SelectionMinPixelSize - (Right - Left);
            Left -= Delta;
            if (Left < PlotRect.Left)
                Left = PlotRect.Left;
        }
    }
    else
    {
        int Delta = SelectionMinPixelSize - (Left - Right);
        if (PlotRect.Left <= Right - Delta)     
            Right -= Delta;
        else 
        {
            Right = PlotRect.Left;
            int Delta = SelectionMinPixelSize - (Left - Right);
            Left += Delta;
            if (Left > PlotRect.Right)
                Left = PlotRect.Right;
        }
    }
}

void TPlot2D::DrawSelection(int Left, int Right, int Erase)
{
    static int PrevLeft = -1;
    static int PrevRight = -1;
    
    if (Erase)
    {
        if (PrevLeft < 0 && PrevRight < 0)
            return;

        Left = -1;
        Right = -1;
    }
    else
    {
        if (Left >= 0 && !(Left >= PlotRect.Left && Left <= PlotRect.Right))
            Left = -1;
        
        if (Right >= 0 && !(Right >= PlotRect.Left && Right <= PlotRect.Right))
            Right = -1;

        if (Left == PrevLeft && Right == PrevRight)
            return;
    }

    HDC hDC = GetDC(hWnd);
    HPEN hPen = CreatePen(PS_DOT, 1, Plot2DSelectionColor);
    HPEN hPrevPen = (HPEN) SelectObject(hDC, hPen);
    int PrevROP = SetROP2(hDC, R2_XORPEN);
    SetBkMode(hDC, TRANSPARENT);

    if (PrevLeft != Left)
    {
        if (Left >= 0)
        {
            MoveToEx(hDC, Left, PlotRect.Top, NULL);    
            LineTo(hDC, Left, PlotRect.Bottom);
        }

        if (PrevLeft >= 0)
        {
            MoveToEx(hDC, PrevLeft, PlotRect.Top, NULL);
            LineTo(hDC, PrevLeft, PlotRect.Bottom);
        }
    }

    if (PrevRight != Right)
    {
        if (Right >= 0)
        {
            MoveToEx(hDC, Right, PlotRect.Top, NULL);   
            LineTo(hDC, Right, PlotRect.Bottom);
        }

        if (PrevRight >= 0)
        {
            MoveToEx(hDC, PrevRight, PlotRect.Top, NULL);
            LineTo(hDC, PrevRight, PlotRect.Bottom);
        }
    }

    PrevLeft = Left;
    PrevRight = Right;

    SetROP2(hDC, PrevROP);
    SelectObject(hDC, hPrevPen);
    ReleaseDC(hWnd, hDC);
    DeleteObject(hPen);
}

void TPlot2D::EnableZoom(TDouble MinSize)
{
    ZoomEnabled = 1;
    SelectionMinSize = MinSize;
}

LRESULT TPlot2D::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
    static HCURSOR hCursor, hSizeCursor;
    static int Selection = 0;
    static int SelectionLeft;
    static int SelectionRight;
    int MouseX; 
    
    switch (message) 
    {   
    case WM_CREATE:
        hDC = GetDC(hWnd);
        SetupPixelFormat(hDC);
        hGLRC = wglCreateContext(hDC);
        hSizeCursor = LoadCursor(0, IDC_SIZEWE);
        DisplayAllowed = 0;
        RedrawAll = 1;
        break;
            
    case WM_DESTROY:
        if (hGLRC) 
        {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hGLRC);
        }
        ReleaseDC(hWnd, hDC);
        break;
            
    case WM_SIZE:
        if (hGLRC) 
        {
            winWidth = (short) LOWORD(lParam);
            winHeight = (short) HIWORD(lParam);
            RedrawAll = 1;
        }
        break;

    case WM_PAINT:
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        if (hGLRC) 
            Display();
        EndPaint(hWnd, &ps);
        break;

    case WM_LBUTTONDOWN:
        if (ZoomEnabled && !MaxZoom)
        {
            POINT CursorPos;
            GetCursorPos(&CursorPos);
            ScreenToClient(hWnd, &CursorPos);
            if (!Selection && CursorPos.x >= PlotRect.Left && CursorPos.x <= PlotRect.Right)
            {
                Selection = 1;
                SelectionLeft = CursorPos.x;
                SelectionRight = SelectionLeft;
                CorrectSelection(SelectionLeft, SelectionRight);
                DrawSelection(SelectionLeft, SelectionRight, 0);

                hCursor = SetCursor(hSizeCursor);
                SetCapture(hWnd);
            }
        }
        break;
    
    case WM_MOUSEMOVE:
        if (Selection)
        {
            POINT CursorPos;
            GetCursorPos(&CursorPos);
            ScreenToClient(hWnd, &CursorPos);
            int NewSelectionRight = CursorPos.x;
            
            if (SelectionRight != NewSelectionRight) 
            {
                SelectionRight = NewSelectionRight;
                if (SelectionRight >= 0 && SelectionRight < PlotRect.Left)
                    SelectionRight = PlotRect.Left;
                if (SelectionRight >= winWidth)
                    SelectionRight = -1;
                if (SelectionRight > PlotRect.Right)
                    SelectionRight = PlotRect.Right;

                if (SelectionRight >= 0)
                    CorrectSelection(SelectionLeft, SelectionRight);
                DrawSelection(SelectionLeft, SelectionRight, 0);
            }
        }
        break;

    case WM_LBUTTONUP:
        if (Selection)
        {
            ReleaseCapture();
            SetCursor(hCursor);
            DrawSelection(SelectionLeft, SelectionRight, 1);
            Selection = 0;

            if (SelectionRight < 0)
                break;

            if (SelectionLeft > SelectionRight)
            {
                int Tmp = SelectionLeft;
                SelectionLeft = SelectionRight;
                SelectionRight = Tmp;
            }

            if (!(SelectionLeft == PlotRect.Left && SelectionRight == PlotRect.Right))
            {
                ListExpand(ZoomHistoryList, TZoomHistory);
                TZoomHistory &ZoomHistory = ZoomHistoryList.ItemList[ZoomLevel];
                ZoomHistory.MinX = Min.x;
                ZoomHistory.MaxX = Max.x;

                ZoomLevel ++;
                if (ZoomHistoryList.Size < ZoomLevel)
                    ZoomHistoryList.Size = ZoomLevel;
                                    
                TDouble Scale = (Max.x - Min.x) / (PlotRect.Right - PlotRect.Left);

                Max.x = (SelectionRight == PlotRect.Right) ? Max.x : 
                        (SelectionRight - PlotRect.Left) * Scale + Min.x;                   
                Min.x = (SelectionLeft - PlotRect.Left) * Scale + Min.x;

                RebuildMesh();
                Display();
            }
        }
        break;

    case WM_RBUTTONUP:
        if (ZoomLevel)
        {   
            ZoomLevel --;
            TZoomHistory &ZoomHistory = ZoomHistoryList.ItemList[ZoomLevel];
            Min.x = ZoomHistory.MinX;
            Max.x = ZoomHistory.MaxX;

            MaxZoom = 0;

            RebuildMesh();
            Display();
        }
        break;
     
    case WM_COMMAND:
            break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

int TPlot2D::ComputeAxisPos(TDouble Min, TDouble Max, TDouble AxisSize, int Expand,
                            TDouble &Pos, TDouble &AxisMin, TDouble &AxisMax, 
                            TDouble &cAxisSize)
{
    int DrawZero;

    if (Min * Max > 0)
    {
        DrawZero = 0;
        Pos = -AxisSize;
    }   
    else
    {
        DrawZero = 1;
        Pos = Min / (Min - Max) * AxisSize * 2.0 - AxisSize;
    }
        
    if (Expand) 
    {
        TDouble Delta = (Max - Min) * Plot2DAxisShift * 0.5;
        AxisMin = Min - Delta;
        AxisMax = Max + Delta;
        cAxisSize = AxisSize * (1.0 + Plot2DAxisShift);
        if (DrawZero)
            Pos = AxisMin / (AxisMin - AxisMax) * cAxisSize * 2.0 - cAxisSize;
    } 
    else
    {
        AxisMin = Min;
        AxisMax = Max;
        cAxisSize = AxisSize;
    }

    return DrawZero;
}

void TPlot2D::DrawAxis(TDouble Left, TDouble Middle, TDouble Right, TDouble Epsilon,
                       TDouble Up, TDouble Down,
                       TPoint2D Point0, TPoint2D Point1, 
                       char *AxisName, int DrawZero, 
                       int Direction, int NumbersDist,
                       int AxisStepE, int AxisStepM, TDouble AxisScale)
{
    int Line;
    TDouble Value;

    int ViewportWidth;
    int ViewportHeight;
    GetViewportSize(ViewportWidth, ViewportHeight);

    TPoint2D Shift;
    Shift.x = Point1.y - Point0.y;
    Shift.y = Point0.x - Point1.x;

    TDouble Norm = fabs(Shift.x) + fabs(Shift.y);
    Norm = 1.0 / Norm;
    Shift.x *= Norm / PixelZoomX;
    Shift.y *= Norm / PixelZoomY;

    // TDouble PixelSize = fabs((Point0.x - Point1.x) * PixelZoomX) + 
    //                     fabs((Point0.y - Point1.y) * PixelZoomY);
    TDouble AxisScale5 = AxisScale / 5.0;
    int Lines0_5 = (int) ceil((Left - Epsilon) / AxisScale5);
    int Lines1_5 = (int) floor((Right + Epsilon) / AxisScale5);
    int AxisM = (int) ceil((Left - Epsilon) / AxisScale) * AxisStepM;

    TDouble RndLeft = Lines0_5 * AxisScale5;

    glBegin(GL_LINES);
        glVertex2d(Point0.x, Point0.y);
        glVertex2d(Point1.x, Point1.y);
    glEnd();    

    int Side = (-Up < Down) ? -1 : 1;
    if (!Direction) Side = -Side;

    int TextJustifyWidth;
    int TextJustifyHeight;
    if (Direction == 0)
    {
        TextJustifyWidth = TextJustifyCenter;
        TextJustifyHeight = (Side == 1) ? TextJustifyTop : TextJustifyBottom;
    }
    else
    {       
        TextJustifyWidth = (Side == 1) ? TextJustifyLeft : TextJustifyRight;
        TextJustifyHeight =  TextJustifyCenter;
    }   

    for (Line = Lines0_5, Value = RndLeft;
         Line <= Lines1_5; Line ++, Value += AxisScale5)
    {
        TDouble LineScale = (Line % 5) ? Plot2DAxisSmallLineSize : Plot2DAxisNormalLineSize;

        TPoint2D Point = (Point1 - Point0) *
                         ((Value - Left) / (Right - Left)) + Point0;

        TPoint2D SPoint = Point + Shift * LineScale * Side;
        TPoint2D TPoint = Point + Shift * Plot2DAxisTextShift * Side;

        glBegin(GL_LINES);
            glVertex2d(Point.x, Point.y);
            glVertex2d(SPoint.x, SPoint.y);
        glEnd();

        if ((Line % 5) == 0)
        {
            if (DrawZero || AxisM)
            {
                char StrValue[32];
                Float2Str(StrValue, AxisStepE, AxisM);
                glRasterPos2d(TPoint.x, TPoint.y);
                Font.SetJustify(TextJustifyWidth, TextJustifyHeight);
                Font.Print(StrValue);
            }   

            AxisM += AxisStepM;
        }
    }

    if (AxisName) 
    {
        TPoint2D Middle;
        if (Left * Right > 0) 
            Middle = Point0 + Point1;
        else 
        {
            TPoint2D ZeroPoint = Point0 + (Point1 - Point0) * (-Left / (-Left + Right));
            if (-Left < Right)
                    Middle = ZeroPoint + Point1;
            else    Middle = Point0 + ZeroPoint;
        }

        TDouble AxisNameShift = Direction ? Plot2DAxisYNameShift : Plot2DAxisXNameShift;
        Middle = Middle * 0.5 + Shift * AxisNameShift * Side;
        glRasterPos2d(Middle.x, Middle.y);
        Font.SetJustify(TextJustifyWidth, TextJustifyHeight);
        Font.Print(AxisName);
    }
}

void TPlot2D::ComputeAxesSize()
{
    if (ConstrainedProjection) 
    {
        TPoint2D Delta = Max - Min;
        TDouble DeltaNorm = Delta.x;
        if (Delta.y > DeltaNorm)
            DeltaNorm = Delta.y;
        DeltaNorm *= 2.0;
        Delta.x /= DeltaNorm;
        Delta.y /= DeltaNorm;
        AxesSize = Delta;
    }
    else
    {
        AxesSize = TPoint2D(0.5, 0.5);
    }
}

void TPlot2D::ComputeAxesPos()
{
    DrawZeroX = ComputeAxisPos(Min.x, Max.x, AxesSize.x, 0, AxesCenter.x, 
                               AxesMin.x, AxesMax.x, AxesLineSize.x);
    DrawZeroY = ComputeAxisPos(Min.y, Max.y, AxesSize.y, 1, AxesCenter.y, 
                               AxesMin.y, AxesMax.y, AxesLineSize.y);
}

void TPlot2D::DrawAxes()
{
    if (DrawZeroX && DrawZeroY)
        DrawZeroX = 0;

    TPoint2D OX0 = AxesCenter, OX1 = AxesCenter; OX0.x = -AxesLineSize.x; OX1.x = AxesLineSize.x;
    TPoint2D OY0 = AxesCenter, OY1 = AxesCenter; OY0.y = -AxesLineSize.y; OY1.y = AxesLineSize.y;

    glColor3f(0.0, 0.0, 0.0);
    DrawAxis(AxesMin.x, 0.0, AxesMax.x, MinMaxEpsilon.x, AxesMin.y, AxesMax.y, OX0, OX1, 
             AxisXName, DrawZeroX, 0, AxisXNumbersDist, 
             AxisXStepE, AxisXStepM, AxisXScale);
    DrawAxis(AxesMin.y, 0.0, AxesMax.y, MinMaxEpsilon.y, AxesMin.x, AxesMax.x, OY0, OY1, 
             AxisYName, DrawZeroY, 1, AxisYNumbersDist, 
             AxisYStepE, AxisYStepM, AxisYScale);
}

void TPlot2D::PrepareFrustum()
{
    int ViewportWidth;
    int ViewportHeight;
    GetViewportSize(ViewportWidth, ViewportHeight);

    Frustum.Left = -0.5;
    Frustum.Right = 0.5;
    Frustum.Top = -0.5;
    Frustum.Bottom = 0.5;

    Frustum.Width = Frustum.Right - Frustum.Left;
    Frustum.Height = Frustum.Bottom - Frustum.Top;
        
    ComputeAxesPos();
  
    TDouble BorderTop = 0.0;
    TDouble BorderBottom = 0.0;

    TDouble BorderHeightShift = AxisXName ?
                                (Font.GetMaxHeight() + Plot2DAxisXNameShift) :
                                (Font.GetMaxHeight() + Plot2DAxisTextShift);

    if (AxesCenter.y <= 0.0) 
    {
        BorderTop = ViewportHeight - (ViewportHeight - BorderHeightShift) / (0.5 - AxesCenter.y);
        if (BorderTop < 0.0) 
            BorderTop = 0.0;
    }
    else
    {
        BorderBottom = ViewportHeight - (ViewportHeight - BorderHeightShift) / (0.5 + AxesCenter.y);
        if (BorderBottom < 0.0)
            BorderBottom = 0.0;
    }

    BorderTop += Plot2DBorderHeight;
    BorderBottom += Plot2DBorderHeight;
    
    Frustum.Top -= BorderTop / (ViewportHeight - BorderTop);
    Frustum.Bottom += BorderBottom / (ViewportHeight - BorderBottom);
    Frustum.Height = Frustum.Bottom - Frustum.Top;

    PixelZoomX = ViewportWidth / Frustum.Width;
    PixelZoomY = ViewportHeight / Frustum.Height;

    TDouble PixelSizeX = 2.0 * AxesSize.x * PixelZoomX;

    int TextWidth;
    int TextHeight;
    AxisXNumbersDist = ComputeNumbersDist(Font, AxesMin.x, AxesMax.x, MinMaxEpsilon.x, PixelSizeX, TextWidth);
    AxisYNumbersDist = Plot2DAxisYNumbersDist;

    char StrValue[16];
    FindScale(AxesMin.y, AxesMax.y, (ViewportHeight - BorderTop - BorderBottom) * AxesLineSize.y * 2.0, 
              AxisYNumbersDist, AxisYStepE, AxisYStepM, AxisYScale);

    Float2Str(StrValue, AxisYStepE, AxisYStepM);
    Font.GetExtentPoint(StrValue, TextWidth, TextHeight);

    int TextWidth2;
    int LeftM = PlotComputeLeftM(AxesMin.y, MinMaxEpsilon.y, AxisYScale, AxisYStepM);
    Float2Str(StrValue, AxisYStepE, LeftM);
    Font.GetExtentPoint(StrValue, TextWidth2, TextHeight);
    if (TextWidth < TextWidth2)
        TextWidth = TextWidth2;
        
    int RightM = PlotComputeRightM(AxesMax.y, MinMaxEpsilon.y, AxisYScale, AxisYStepM);
    Float2Str(StrValue, AxisYStepE, RightM);
    Font.GetExtentPoint(StrValue, TextWidth2, TextHeight);
    if (TextWidth < TextWidth2)
        TextWidth = TextWidth2;

    TDouble BorderLeft = 0.0;
    TDouble BorderRight = 0.0;

    int AxisYNameWidth;
    int AxisYNameHeight;
    Font.GetExtentPoint(AxisYName, AxisYNameWidth, AxisYNameHeight);

    Plot2DAxisYNameShift = TextWidth + Plot2DAxisXNameShift - Font.GetMaxHeight();
    TDouble BorderWidthShift = AxisYName ? 
                               (Plot2DAxisYNameShift + AxisYNameWidth) :
                               (TextWidth + Plot2DAxisTextShift);

    if (AxesCenter.x <= 0.0) 
    {
        BorderLeft = ViewportWidth - (ViewportWidth - BorderWidthShift) / (0.5 - AxesCenter.x);
        if (BorderLeft < 0.0) 
            BorderLeft = 0.0;
    }
    else
    {
        BorderRight = ViewportWidth - (ViewportWidth - BorderWidthShift) / (AxesCenter.x + 0.5);
        if (BorderRight < 0.0)
            BorderRight = 0.0;
    }

    BorderLeft += Plot2DBorderWidth;
    BorderRight += Plot2DBorderWidth;

    Frustum.Left -= BorderLeft / (ViewportWidth - BorderLeft);
    Frustum.Right += BorderRight / (ViewportWidth - BorderRight);
    Frustum.Width = Frustum.Right - Frustum.Left;

    PlotRect.Left = (int) floor((-Frustum.Left - 0.5) / Frustum.Width * ViewportWidth + 0.001);
    PlotRect.Right = (int) floor((1.0 - (Frustum.Right - 0.5) / Frustum.Width) * ViewportWidth + 0.001);
    PlotRect.Top = (int) floor((Frustum.Bottom - 0.5) / Frustum.Height * ViewportHeight + 0.001);
    PlotRect.Bottom = (int) floor((1.0 - (-Frustum.Top - 0.5) / Frustum.Height) * ViewportHeight + 0.001);   

    TDouble SelectionMinPixelSizef = (PlotRect.Right - PlotRect.Left + 1) * SelectionMinSize / (Max.x - Min.x);
    if (SelectionMinPixelSizef > PlotRect.Right - PlotRect.Left)
            SelectionMinPixelSize = PlotRect.Right - PlotRect.Left;
    else    SelectionMinPixelSize = (int) (SelectionMinPixelSizef + 2.0);
    
    if (SelectionMinPixelSize < Plot2DSelectionMinSize)
        SelectionMinPixelSize = Plot2DSelectionMinSize;
        
    if (Max.x - Min.x < SelectionMinSize * Plot2DMaxZoomThreshold)
            MaxZoom = 1;

    PixelZoomX = ViewportWidth / Frustum.Width;
    PixelZoomY = ViewportHeight / Frustum.Height;

    FindScale(AxesMin.x, AxesMax.x, (ViewportWidth - BorderLeft - BorderRight) * AxesLineSize.x * 2.0, 
              AxisXNumbersDist, AxisXStepE, AxisXStepM, AxisXScale);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(Frustum.Left, Frustum.Right, 
            Frustum.Top, Frustum.Bottom,
            0.5, 10000.0);

    /*
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -1.0);
    
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2d(-0.5, -0.5);
        glVertex2d( 0.5, -0.5);
        glVertex2d( 0.5,  0.5);
        glVertex2d(-0.5,  0.5);
    glEnd();
    */
}

void TPlot2D::Render()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -1.0);
    
    MinMaxEpsilon = (Max - Min) * PlotMinMaxEpsilon;

    ComputeAxesSize();
    PrepareFrustum();
#ifdef PLOT2D_DRAW_MESH_ON_TOP
    DrawAxes();
    DrawMesh();
#else
    DrawMesh();
    DrawAxes();
#endif /* PLOT2D_DRAW_MESH_ON_TOP */
}

void TPlot2D::Display()
{
    if (!DisplayAllowed)
        return;

    wglMakeCurrent(hDC, hGLRC);

    // if (RedrawAll)
    {
        RedrawAll = 0;
        glViewport(0, 0, winWidth, winHeight);
        glClearColor(Plot2DBackGroundColor[0], 
                     Plot2DBackGroundColor[1], 
                     Plot2DBackGroundColor[2], 
                     Plot2DBackGroundColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        Render();
        glFlush();
    }   

    SwapBuffers(hDC);
}

void TPlot2D::GetViewportSize(int &Width, int &Height)
{
    GLint i[4];
    glGetIntegerv(GL_VIEWPORT, i);
    Width = i[2];
    Height = i[3];
}

void TPlot2D::ComputeFunctions()
{
    int Functions = PlotFunctionList.Size;
    if (Functions == 0)
        return;

    TPlot2DFunctionListItem *Function = PlotFunctionList.ItemList;

    do 
    {
        TPlot2DLineStripListItem &LineStrip = PlotLineStripList.ItemList[Function->LineStripListIndex];

        TDouble Left = Function->Left;
        TDouble Right = Function->Right;
        
        if (ZoomLevel)
        {
            if (Left < Min.x)
                Left = Min.x;
            if (Right > Max.x)
                Right = Max.x;
        }
                
        TPoint2D *PointList;
        if (LineStrip.PointList)
                PointList = LineStrip.PointList;
        else    PointList = LineStrip.PointList = new TPoint2D[Function->Points]; 

        TPoint2D *Point = PointList;
        int Count = Function->Points;
        TDouble FuncMin, FuncMax;
        TDouble x = Left;
        TDouble Step = (Right - Left) / (Count - 1);

        do 
        {
            TDouble y = Function->Func(x);
            Point->x = x;
            Point->y = y;

            if (Count == Function->Points)
            {
                FuncMin = y;
                FuncMax = y;
            }
            else
            if (FuncMin > y) 
                FuncMin = y;
            else
            if (FuncMax < y) 
                FuncMax = y;

            x += Step;
            Point ++;               
        } while (--Count);

        UnionMinMax(TPoint2D(Left, FuncMin), TPoint2D(Right, FuncMax));
    
        LineStrip.Color = Function->Color;
        LineStrip.Points = Function->Points;
        LineStrip.PointList = PointList;

        Function ++;
    } while (--Functions);
}

void TPlot2D::RebuildMesh()
{
    InvalidMinMax = 1;
    ComputeFunctions(); 
}

void TPlot2D::AttachFunction(TDouble Left, TDouble Right, TDouble (*Func)(TDouble), TRGBColor &Color, int Points)
{
    ListExpand(PlotFunctionList, TPlot2DFunctionListItem);
    ListExpand(PlotLineStripList, TPlot2DLineStripListItem);
        
    TPlot2DFunctionListItem &Function = PlotFunctionList.ItemList[PlotFunctionList.Size ++];

    Function.Left = Left;
    Function.Right = Right;
    Function.Func = Func;
    Function.Points = Points;
    Function.Color = Color;
    Function.LineStripListIndex = PlotLineStripList.Size;

    TPlot2DLineStripListItem &LineStrip = PlotLineStripList.ItemList[PlotLineStripList.Size ++];

    LineStrip.PointList = NULL;
}

void TPlot2D::AttachListStrip(TPoint2D *PointList, int Points, TRGBColor &Color)
{
    ListExpand(PlotLineStripList, TPlot2DLineStripListItem);
    TPlot2DLineStripListItem &ListItem = PlotLineStripList.ItemList[PlotLineStripList.Size ++];
    ListItem.Color = Color;
    ListItem.Points = Points;
    ListItem.PointList = PointList;
}

void TPlot2D::AttachPoints(TPoint2D *PointList, int Points, TRGBColor &Color)
{
    ListExpand(PlotPointList, TPlot2DPointListItem);
    TPlot2DPointListItem &ListItem = PlotPointList.ItemList[PlotPointList.Size ++];
    ListItem.Color = Color;
    ListItem.Points = Points;
    ListItem.PointList = PointList;
}

void TPlot2D::AttachKeyPoint(TPoint2D &Point, char *Text, int TextJustifyWidth, 
                             int TextJustifyHeight, TRGBColor &TextColor)
{
    ListExpand(PlotKeyPointList, TPlot2DKeyPointListItem);
    TPlot2DKeyPointListItem &ListItem = PlotKeyPointList.ItemList[PlotKeyPointList.Size ++];
    ListItem.TextColor = TextColor;
    ListItem.Point = Point;
    ListItem.Text = Text;
    ListItem.TextJustifyWidth = TextJustifyWidth;
    ListItem.TextJustifyHeight = TextJustifyHeight;
}

void TPlot2D::DrawMesh()
{
    TPoint2D Shift = (Min + Max) * -0.5;

    TPoint2D Scale;
    Scale.x = 2.0 * AxesSize.x / (Max.x - Min.x);
    Scale.y = 2.0 * AxesSize.y / (Max.y - Min.y);

    glPushMatrix();
    glScaled(Scale.x, Scale.y, 1.0);
    glTranslated(Shift.x, Shift.y, 0.0);

    TDouble LeftPlane[4] =   { 1.0,  0.0, 0.0, -AxesMin.x};
    TDouble RightPlane[4] =  {-1.0,  0.0, 0.0,  AxesMax.x};
    TDouble TopPlane[4] =    { 0.0, -1.0, 0.0,  AxesMax.y};
    TDouble BottomPlane[4] = { 0.0,  1.0, 0.0, -AxesMin.y};

    glEnable(GL_CLIP_PLANE0); glClipPlane(GL_CLIP_PLANE0, LeftPlane);
    glEnable(GL_CLIP_PLANE1); glClipPlane(GL_CLIP_PLANE1, RightPlane);
    glEnable(GL_CLIP_PLANE2); glClipPlane(GL_CLIP_PLANE2, TopPlane);
    glEnable(GL_CLIP_PLANE3); glClipPlane(GL_CLIP_PLANE3, BottomPlane);

    int Item;
    TPlot2DLineStripListItem *LineStrip;
    for (LineStrip = PlotLineStripList.ItemList, Item = 0;
         Item < PlotLineStripList.Size; LineStrip ++, Item ++)
    {
        TPoint2D *Point = LineStrip->PointList;
        if (Point)
        {
            int Points = LineStrip->Points;
            TRGBColor &Color = LineStrip->Color;
            glColor3f(Color.r, Color.g, Color.b);
            glBegin(GL_LINE_STRIP);
                while (Points --)
                {   
                    glVertex2d(Point->x, Point->y); 
                    Point ++; 
                }
            glEnd();
        }
    }

    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);

    /*
    glBegin(GL_LINES);
        glVertex2dv(&Min.x);
        glVertex2dv(&Max.x);
    glEnd();
    */

    glPopMatrix();
}

void TPlot2D::UnionMinMax(TPoint2D &_Min, TPoint2D &_Max)
{
    if (!InvalidMinMax) 
    {
        TBox2D BBox2(_Min, _Max);
        TBox2D BBox(Min, Max);
        BBox.Union(BBox, BBox2);
        Min = BBox.Min;
        Max = BBox.Max;
    }   
    else 
    {
        Min = _Min;
        Max = _Max;
        InvalidMinMax = 0;
    }   
}

void TPlot2D::GetMinMax(TPoint2D &_Min, TPoint2D &_Max)
{
    assert(InvalidMinMax == 0);
    _Min = Min;
    _Max = Max;
} 

void TPlot2D::SetMinMax(TPoint2D &_Min, TPoint2D &_Max)
{
    Min = _Min;
    Max = _Max;
    InvalidMinMax = 0;
}

void TPlot2D::StoreState(TPlot2DState &State)
{
    (void) State;
}

void TPlot2D::LoadState(TPlot2DState &State)
{
    (void) State;
}

void TPlot2D::Flush()
{
    ComputeFunctions();

    DisplayAllowed = 1;
}

TPlot2D::TPlot2D(HINSTANCE _hInstance, int winX, int winY, 
                 int winWidth, int winHeight, char *winName, HWND _ParentWindow)
{
    hInstance = _hInstance;
    ParentWindow = _ParentWindow;

    hGLRC = NULL;

    InvalidMinMax = 1;      
    
    Font.Load(PlotFixedFont);

    RegisterPlot2DClass();
    CreatePlot2DWindow(winX, winY, winWidth, winHeight, winName);

    AxisXName = "x";
    AxisYName = "y";

    ConstrainedProjection = 0;

    MaxZoom = 0;
    ZoomEnabled = 0;
    ZoomLevel = 0;

    ListInit(PlotPointList);
    ListInit(PlotKeyPointList);
    ListInit(PlotLineStripList);
    ListInit(PlotFunctionList);

    ListInit(ZoomHistoryList);
}

TPlot2D::~TPlot2D()
{
    DestroyWindow(hWnd);
    UnRegisterPlotClass(hWnd);
    hWnd = NULL;

    ListDone(ZoomHistoryList);

    ListDone(PlotPointList);
    ListDone(PlotKeyPointList);
    ListDone(PlotFunctionList);

    for (int i = 0; i < PlotLineStripList.Size; i ++)
        delete[] PlotLineStripList.ItemList[i].PointList;
    ListDone(PlotLineStripList);        
}
