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
    double HauteurExtrusion;
    Matrix4 Transform;
    bool IsMappedItem;
    Matrix4 Transformation2D;
    Matrix4 TransformationOperator3D;

    virtual void createSolid3dProfil(Style styleDessin) = 0;
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

    void createSolid3dProfil(Style styleDessin) override;
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

    void createSolid3dProfil(Style styleDessin) override;
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

    void createSolid3dProfil(Style styleDessin) override;
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

    void createSolid3dProfil(Style styleDessin) override;
};

struct C_profilDef : public ProfilDef
{
    double Depth;
    double Width;
    double WallThickness;
    double Girth;
    double InternalFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Z_profilDef : public ProfilDef
{
    double Depth;
    double FlangeWidth;
    double WebThickness;
    double FlangeThickness;
    double FilletRadius;
    double EdgeRadius;

    void createSolid3dProfil(Style styleDessin) override;
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

    void createSolid3dProfil(Style styleDessin) override;
};

struct CircleHollow_profilDef : public ProfilDef
{
    double Radius;
    double WallThickness;

    void createSolid3dProfil(Style styleDessin) override;
};

struct RectangleHollow_profilDef : public ProfilDef
{
    double XDim;
    double YDim;
    double WallThickness;
    double InnerFilletRadius;
    double OuteerFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Rectangle_profilDef : public ProfilDef
{
    double XDim;
    double YDim;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Circle_profilDef : public ProfilDef
{
    double Radius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct MappedItem
{
    std::vector<std::string> nameItemsMap;
    std::string NameProfilDefMap;
    std::vector<int> keyItemsMap;
    std::list<Vec3> points1Map;
    std::string outerCurveNameMap;
    std::vector<int> ListNbArgMap;
    std::vector<int> listNbArgFaceMap;
    Vec3 VecteurExtrusionMap;
    float hauteurExtrusionMap;
    std::list<Matrix4> listPlanMap;
    std::list<Matrix4> listLocationPolygonalMap;
    std::vector<Step::Boolean> AgreementHalfMap;
    std::vector<Step::Boolean> AgreementPolygonalMap;
    std::vector<std::string> listEntityHalfMap;
    std::vector<std::string> listEntityPolygonalMap;
    CompositeCurveSegment _compositeCurveMap;
    int nbPolylineCompositeMap;
    Box boxMap;
    Matrix4 transform1Map;
    Matrix4 transformationMap;
    Matrix4 transformation2DMap;
    Matrix4 transformFaceMap;
    bool isMappedItemMap;
    Matrix4 transformationOperator3DMap;
    double scale;
};

struct IFCShapeRepresentation
{
    int Key;
    std::string EntityType;
    std::string RepresentationIdentifier;
    std::string RepresentationType;
    std::string ProfilDefName;
    std::string OuterCurveName;
    std::string EntityHalf;
    double Scale = 0.0;
    std::list<Vec3> Points;
    Matrix4 Transformation;
    Matrix4 Transformation2D;
    Matrix4 TransformationOperator3D;
    Matrix4 Plan;
    Step::Boolean AgreementHalf;
    Step::Boolean AgreementPolygonal;
    Step::Logical AgreementCompositeCurve;
    Matrix4 TransformBoolean;
    std::vector<IFCShapeRepresentation> IfcFaces;
    std::vector<CompositeCurveSegment> CompositeCurve;
    ProfilDef* ProfilDef;
};

struct IFCObject
{
    int Key;
    bool IsMappedItem;
    std::string Entity;
    Matrix4 LocalTransform;
    Vec3 ExtrusionVector;
    double ExtrusionHeight = 0.0;
    std::vector<int> KeyMappedItems;
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