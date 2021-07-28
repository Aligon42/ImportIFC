#pragma once

#include <string>
#include <vector>
#include <list>
#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

#include "Object.h"

typedef mathfu::Vector<double, 3> Vec3;
typedef mathfu::Matrix<double, 4> Matrix4;

struct TrimmedCurve
{
    Vec3 centreCircle;
    float Radius;
    int Trim1;
    int Trim2;
    bool SenseAgreement;
};

struct CompositeCurveSegment
{
    std::vector<Vec3> PointsPolyligne;
    TrimmedCurve* TrimmedCurves;
    std::string ParentCurve;
};

struct Style
{
    int keyItem;
    double red = 1;
    double green = 1;
    double blue = 1;
    double transparence = 0;
};

struct Box
{
    Vec3 Corner;
    int XDimBox;
    int YDimBox;
    int ZDimBox;
};

struct ProfilDef
{
    std::string Name;
    std::string Entity;
    Vec3 VecteurExtrusion;
    float HauteurExtrusion;
    Matrix4 Transform;
    bool IsMappedItem;
    Matrix4 TransformationOperator3D;

    virtual void createSolid3dProfil(Style styleDessin) = 0;
};

struct I_profilDef : public ProfilDef
{
    float OverallWidth;
    float OverallDepth;
    float webThickness;
    float flangeThickness;
    float filletRadius;
    float flangeEdgeRadius;
    float FlangeSlope;
    int nbArg;

    void createSolid3dProfil(Style styleDessin) override;
};

struct L_profilDef : public ProfilDef
{
    float Depth;
    float Width;
    float Thickness;
    float FilletRadius;
    float EdgeRadius;
    float LegSlope;
    int nbArg;

    void createSolid3dProfil(Style styleDessin) override;
};

struct T_profilDef : public ProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float FlangeEdgeRadius;
    float WebEdgeRadius;
    float WebSlope;
    float FlangeSlope;
    int nbArg;

    void createSolid3dProfil(Style styleDessin) override;
};

struct U_profilDef : public ProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;
    float FlangeSlope;
    int nbArg;

    void createSolid3dProfil(Style styleDessin) override;
};

struct C_profilDef : public ProfilDef
{
    float Depth;
    float Width;
    float WallThickness;
    float Girth;
    float InternalFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Z_profilDef : public ProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct AsymmetricI_profilDef : public ProfilDef
{
    float OverallWidth;
    float OverallDepth;
    float WebThickness;
    float FlangeThickness;
    float FlangeFilletRadius;
    float TopFlangeWidth;
    float TopFlangeThickness;
    float TopFlangeFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct CircleHollow_profilDef : public ProfilDef
{
    float Radius;
    float WallThickness;

    void createSolid3dProfil(Style styleDessin) override;
};

struct RectangleHollow_profilDef : public ProfilDef
{
    float XDim;
    float YDim;
    float WallThickness;
    float InnerFilletRadius;
    float OuteerFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Rectangle_profilDef : public ProfilDef
{
    float XDim;
    float YDim;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Circle_profilDef : public ProfilDef
{
    float Radius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct IFCObject
{
    int Key;
    std::string Entity;
    std::string NameProfilDef;
    Vec3 VecteurExtrusion;
    float HauteurExtrusion;
    Matrix4 Transform;
    std::list<Vec3> Points;
    Matrix4 TransformFace;
    std::vector<std::string> NameItems;
    Style StyleDessin;
    bool IsMappedItem;
    bool Orientation;
    Matrix4 TransformationOperator3D;
    std::vector<int> KeyItems;
    std::string OuterCurveName;
    std::vector<int> ListNbArg;
    std::vector<int> ListNbArgFace;
    std::list<Matrix4> ListPlan;
    std::list<Matrix4> ListLocationPolygonal;
    std::vector<bool> AgreementHalf;
    std::vector<bool> AgreementPolygonal;
    std::vector<std::string> ListEntityHalf;
    std::vector<std::string> ListEntityPolygonal;
    CompositeCurveSegment CompositeCurveSegment;
    int NbPolylineComposite;
    Box Box;
    std::shared_ptr<ProfilDef> ProfilDef;
};

struct IFCShapeRepresentation
{
    int Key;
    std::string EntityType;
    std::string RepresentationIdentifier;
    std::string RepresentationType;
    std::string ProfilDefName;
    Vec3 ExtrusionVector;
    double ExtrusionHeight = 0.0;
    std::list<Vec3> Points;
    Matrix4 Plan;
    Step::Boolean AgreementHalf;
    Step::Boolean AgreementPolygonal;
    Matrix4 LocationPolygonal;
    Matrix4 TransformBoolean;
    std::vector<CompositeCurveSegment> CompositeCurve;
    ProfilDef* ProfilDef;
};

struct IFCObjTemp
{
    int Key;
    bool IsMappedItem;
    std::string Entity;
    std::string OuterCurveName;
    Matrix4 LocalTransform;
    Matrix4 Transformation;
    Matrix4 Transformation2D;
    Matrix4 TransformationOperator3D;
    std::vector<IFCShapeRepresentation> ShapeRepresentations;
};

struct ObjectVoid
{
    int KeyForVoid;
    std::string NameProfilDef;
    Vec3 VecteurExtrusion;
    double HauteurExtrusion;
    std::list<Vec3> Points;
    std::vector<int> Args;
    double XDim;
    double YDim;
    double Radius;
    std::list<Matrix4> Plans;
    std::list<Matrix4> LocationsPolygonal;
    std::vector<Step::Boolean> AgreementHalf;
    std::vector<Step::Boolean> AgreementPolygonal;
    std::vector<std::string> EntitiesHalf;
    std::vector<std::string> EntitiesPolygonal;
    Matrix4 Transform;
    Matrix4 Transform2D;
    std::vector<IFCShapeRepresentation> ShapeRepresentations;
};