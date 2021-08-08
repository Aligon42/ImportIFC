#pragma once

#include <string>
#include <vector>
#include <list>
#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

#include <ifc2x3/InheritVisitor.h>

typedef mathfu::Vector<double, 3> Vec3;
typedef mathfu::Matrix<double, 4> Matrix4;

struct ProfilDef;
struct IFCShapeRepresentation;

struct TrimmedCurve
{
    Vec3 centreCircle;
    double Radius;
    int Trim1;
    int Trim2;
    bool SenseAgreement;
};

struct CompositeCurveSegment
{
    ifc2x3::IfcTransitionCode Transition;
    Step::Boolean SameSense;
    std::vector<Vec3> PointsPolyligne;
    TrimmedCurve* TrimmedCurves = nullptr;
    std::string ParentCurve;
};

struct RGBA
{
    double Red = 1;
    double Green = 1;
    double Blue = 1;
    double Alpha = 0;
};

struct Style
{
    int keyItem;
    std::vector<RGBA> Styles;
};

struct Box
{
    Vec3 Corner;
    int XDimBox;
    int YDimBox;
    int ZDimBox;
};

struct Face
{
    Step::Boolean Orientation;
    std::string Type;
    std::string SupType;
    std::vector<Vec3> Points;
};

struct IFCShapeRepresentation
{
    int Key;
    bool BooleanResult = false;
    std::string EntityType;
    std::string RepresentationIdentifier;
    std::string RepresentationType;
    std::string ProfilDefName;
    std::string OuterCurveName;
    std::string EntityHalf;
    double Scale = 0.0;
    double DeterminantMatrixOperator3D = 0.0;
    std::list<Vec3> Points;
    Matrix4 Transformation;
    Matrix4 Transformation2D;
    Matrix4 TransformationOperator3D;
    Matrix4 Plan;
    Matrix4 LocationPolygonal;
    Step::Boolean AgreementHalf;
    Step::Boolean AgreementPolygonal;
    Step::Logical AgreementCompositeCurve;
    Matrix4 TransformBoolean;
    std::vector<Face> IfcFaces;
    std::vector<CompositeCurveSegment> CompositeCurve;
    ProfilDef* ProfilDef;
    std::vector<IFCShapeRepresentation> SubShapeRepresentations;
};

struct IFCObject
{
    int Key;
    int VoidKey;
    bool IsMappedItem;
    std::string Entity;
    Matrix4 LocalTransform;
    Vec3 ExtrusionVector;
    double ExtrusionHeight = 0.0;
    std::vector<IFCShapeRepresentation> ShapeRepresentations;
};

struct ProfilDef
{
    int Key;
    std::string Name;
    std::string Entity;
    Vec3 VecteurExtrusion;
    double HauteurExtrusion;
    Matrix4 Transform;
    bool IsMappedItem;
    Matrix4 Transformation2D;
    Matrix4 TransformationOperator3D;
    IFCObject* ParentObject;
    virtual void createSolid3dProfil() = 0;
};

struct I_profilDef : public ProfilDef
{
    double OverallWidth;
    double OverallDepth;
    double WebThickness;
    double FlangeThickness;
    double FilletRadius;
    double FlangeEdgeRadius;
    double FlangeSlope;
    int nbArg;

    void createSolid3dProfil() override;
};

struct L_profilDef : public ProfilDef
{
    double Depth;
    double Width;
    double Thickness;
    double FilletRadius;
    double EdgeRadius;
    double LegSlope;
    int nbArg;

    void createSolid3dProfil() override;
};

struct T_profilDef : public ProfilDef
{
    double Depth;
    double FlangeWidth;
    double WebThickness;
    double FlangeThickness;
    double FilletRadius;
    double FlangeEdgeRadius;
    double WebEdgeRadius;
    double WebSlope;
    double FlangeSlope;
    int nbArg;

    void createSolid3dProfil() override;
};

struct U_profilDef : public ProfilDef
{
    double Depth;
    double FlangeWidth;
    double WebThickness;
    double FlangeThickness;
    double FilletRadius;
    double EdgeRadius;
    double FlangeSlope;
    int nbArg;

    void createSolid3dProfil() override;
};

struct C_profilDef : public ProfilDef
{
    double Depth;
    double Width;
    double WallThickness;
    double Girth;
    double InternalFilletRadius;

    void createSolid3dProfil() override;
};

struct Z_profilDef : public ProfilDef
{
    double Depth;
    double FlangeWidth;
    double WebThickness;
    double FlangeThickness;
    double FilletRadius;
    double EdgeRadius;

    void createSolid3dProfil() override;
};

struct AsymmetricI_profilDef : public ProfilDef
{
    double OverallWidth;
    double OverallDepth;
    double WebThickness;
    double FlangeThickness;
    double FlangeFilletRadius;
    double TopFlangeWidth;
    double TopFlangeThickness;
    double TopFlangeFilletRadius;

    void createSolid3dProfil() override;
};

struct CircleHollow_profilDef : public ProfilDef
{
    double Radius;
    double WallThickness;

    void createSolid3dProfil() override;
};

struct RectangleHollow_profilDef : public ProfilDef
{
    double XDim;
    double YDim;
    double WallThickness;
    double InnerFilletRadius;
    double OuteerFilletRadius;

    void createSolid3dProfil() override;
};

struct Rectangle_profilDef : public ProfilDef
{
    double XDim;
    double YDim;

    void createSolid3dProfil() override;
};

struct Circle_profilDef : public ProfilDef
{
    double Radius;

    void createSolid3dProfil() override;
};
