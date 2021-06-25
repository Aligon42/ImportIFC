 #pragma once
#include "tchar.h"
#include "aced.h"
#include "rxregsvc.h"
#include "dbapserv.h"
#include "dbents.h"
#include "dbsol3d.h"
#include "dbregion.h"
#include "dbsymutl.h"
#include "dbplanesurf.h"
#include "AcApDMgr.h"
#include <Windows.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <string>
#include <ifc2x3/InheritVisitor.h>

#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

typedef mathfu::Vector<float, 3> Vec3;
typedef mathfu::Matrix<float, 4> Matrix4;

struct I_profilDef
{
    float OverallWidth;
    float OverallDepth;
    float webThickness;
    float flangeThickness;
    float filletRadius;
    float flangeEdgeRadius;
    float FlangeSlope;
    int nbArg;
};

struct L_profilDef
{
    float Depth;
    float Width;
    float Thickness;
    float FilletRadius;
    float EdgeRadius;
    float LegSlope;
    int nbArg;
};

struct T_profilDef
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
};

struct U_profilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;
    float FlangeSlope;
    int nbArg;
};

struct C_profilDef
{
    float Depth;
    float Width;
    float WallThickness;
    float Girth;
    float InternalFilletRadius;
};

struct Z_profilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;
};

struct AsymmetricI_profilDef
{
    float OverallWidth;
    float OverallDepth;
    float WebThickness;
    float FlangeThickness;
    float FlangeFilletRadius;
    float TopFlangeWidth;
    float TopFlangeThickness;
    float TopFlangeFilletRadius;
};

struct CircleHollow_profilDef
{
    float Radius;
    float WallThickness;
};

struct RectangleHollow_profilDef
{
    float XDim;
    float YDim;
    float WallThickness;
    float InnerFilletRadius;
    float OuteerFilletRadius;
};

struct Rectangle_profilDef
{
    float XDim;
    float YDim;
};

struct Circle_profilDef
{
    float Radius;
};

struct TrimmedCurve
{
    Vec3 centreCircle;
    float radius;
    int trim1;
    int trim2;
    bool senseArgreement;
};

struct CompositeCurveSegment
{
    std::vector<std::vector<Vec3>> listPolyligne;
    std::vector<TrimmedCurve> listTrimmedCurve;
    std::vector<std::string> listParentCurve;
};

struct Style
{
    int keyItem;
    double red;
    double green;
    double blue;
};

struct Box
{
    Vec3 Corner;
    int XDimBox;
    int YDimBox;
    int ZDimBox;
};

struct MappedItem
{
    std::vector<std::string> nameItemsMap;
    std::vector<int> keyItemsMap;
    std::list<Vec3> points1Map;
    std::string outerCurveNameMap;
    std::vector<int> ListNbArgMap;
    Vec3 VecteurExtrusionMap;
    float hauteurExtrusionMap;
    std::list<Matrix4> listPlanMap;
    std::list<Matrix4> listLocationPolygonalMap;
    std::vector<bool> AgreementHalfMap;
    std::vector<bool> AgreementPolygonalMap;
    std::vector<std::string> listEntityHalfMap;
    std::vector<std::string> listEntityPolygonalMap;
    CompositeCurveSegment _compositeCurveMap;
    int nbPolylineCompositeMap;
    Box boxMap;
    Matrix4 transform1Map;
    Matrix4 transformationMap;
    Matrix4 transformFaceMap;
    bool isMappedItemMap;
    Matrix4 transformationOperator3DMap;
};

//style
static Style style;
static Style styleDessin;
static std::vector<Style> listStyle;


class CreateConstructionPointVisitor : public ifc2x3::InheritVisitor
{
private:

    Matrix4 _transformation{ Matrix4::Identity() };
    Matrix4 transform;
    Matrix4 transformationOperator3D;
    float determinantMatrixOperator3D;

    std::list<Vec3> _points;
    Vec3 extrusionVector;
    float hauteurExtrusion;
    Matrix4 transformation;
    std::vector<std::string> nameItems;
    std::string outerCurveName;
    bool isCompositeCurve = false;
    bool isBoolean = false;
    bool isMappedItem = false;

    int nbPolylineCompositeCurve = 0;
    int nbSupport = 0;
    int scale = 0;

    int keyForVoid;
    std::vector<int> keyItems;

    //opération boolean
    std::vector<bool> AgreementHalf;
    std::vector<std::string> entityHalf;
    std::vector<bool> AgreementPolygonal;
    std::vector<std::string> entityPolygonal;
    std::vector<bool> AgreementCompositeCurve;
    std::vector<ifc2x3::IfcTransitionCode> transitionCompositeCurveSegment;
    std::vector<bool> sameSenseCompositeCurveSegment;
    std::vector<int> trim1TrimmedCurve;
    std::vector<int> trim2TrimmedCurve;
    std::vector<bool> senseAgreementTrimmedCurve;
    std::string nameParentCurve;
    std::vector<int> radiusCircle;
    std::list<Matrix4> listPlan;
    std::list<Matrix4> listLocationPolygonal;
    std::vector<int> listNbArgPolyline;
    
    //profilDef
    std::string NameProfilDef;
    I_profilDef IprofilDef;
    L_profilDef LprofilDef;
    T_profilDef TprofilDef;
    U_profilDef UprofilDef;
    C_profilDef CprofilDef;
    Z_profilDef ZprofilDef;
    AsymmetricI_profilDef AsymmetricIprofilDef; 
    CircleHollow_profilDef CircleHollowprofilDef;
    RectangleHollow_profilDef RectangleHollowprofilDef;
    Circle_profilDef CircleprofilDef;
    Rectangle_profilDef RectangleprofilDef;

    TrimmedCurve _trimmedCurve;
    CompositeCurveSegment _compositeCurveSegment;

    //face
    bool orientationFace;
    std::vector<int> nbArgFace;

    //Box
    Box box;

    //MappedItem
    MappedItem mappedItem;
    

public:
    //! Constructor
    CreateConstructionPointVisitor();

    bool visitIfcProduct(ifc2x3::IfcProduct* value) override;
    bool visitIfcSite(ifc2x3::IfcSite* value) override;
    bool visitIfcRelVoidsElement(ifc2x3::IfcRelVoidsElement* value) override;
    bool visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value) override;
    bool visitIfcProductDefinitionShape(ifc2x3::IfcProductDefinitionShape* value) override;
    bool visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value) override;
    bool visitIfcBooleanClippingResult(ifc2x3::IfcBooleanClippingResult* value) override;
    bool visitIfcRepresentationMap(ifc2x3::IfcRepresentationMap* value) override;
    bool visitIfcFaceBasedSurfaceModel(ifc2x3::IfcFaceBasedSurfaceModel* value) override;
    bool visitIfcConnectedFaceSet(ifc2x3::IfcConnectedFaceSet* value) override;
    bool visitIfcShellBasedSurfaceModel(ifc2x3::IfcShellBasedSurfaceModel* value) override;
    bool visitIfcOpenShell(ifc2x3::IfcOpenShell* value) override;
    bool visitIfcMappedItem(ifc2x3::IfcMappedItem* value) override;
    bool visitIfcCartesianTransformationOperator3D(ifc2x3::IfcCartesianTransformationOperator3D* value) override;
    bool visitIfcHalfSpaceSolid(ifc2x3::IfcHalfSpaceSolid* value) override;
    bool visitIfcPolygonalBoundedHalfSpace(ifc2x3::IfcPolygonalBoundedHalfSpace* value) override;
    bool visitIfcCompositeCurve(ifc2x3::IfcCompositeCurve* value) override;
    bool visitIfcCompositeCurveSegment(ifc2x3::IfcCompositeCurveSegment* value) override;
    bool visitIfcTrimmedCurve(ifc2x3::IfcTrimmedCurve* value) override;
    bool visitIfcCircle(ifc2x3::IfcCircle* value) override; 
    bool visitIfcPlane(ifc2x3::IfcPlane* value) override;
    bool visitIfcAxis2Placement(ifc2x3::IfcAxis2Placement* value) override;
    bool visitIfcAxis2Placement2D(ifc2x3::IfcAxis2Placement2D* value) override;
    bool visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value) override;
    bool visitIfcBoundingBox(ifc2x3::IfcBoundingBox* value) override;

    //style
    bool visitIfcStyledItem(ifc2x3::IfcStyledItem* value) override;
    bool visitIfcPresentationStyleAssignment(ifc2x3::IfcPresentationStyleAssignment* value) override;
    bool visitIfcSurfaceStyle(ifc2x3::IfcSurfaceStyle* value) override;
    bool visitIfcSurfaceStyleRendering(ifc2x3::IfcSurfaceStyleRendering* value) override;
    bool visitIfcColourRgb(ifc2x3::IfcColourRgb* value) override;

    //profilDef
    bool visitIfcIShapeProfileDef(ifc2x3::IfcIShapeProfileDef* value) override;
    bool visitIfcLShapeProfileDef(ifc2x3::IfcLShapeProfileDef* value) override;
    bool visitIfcTShapeProfileDef(ifc2x3::IfcTShapeProfileDef* value) override;
    bool visitIfcUShapeProfileDef(ifc2x3::IfcUShapeProfileDef* value) override;
    bool visitIfcCShapeProfileDef(ifc2x3::IfcCShapeProfileDef* value) override;
    bool visitIfcZShapeProfileDef(ifc2x3::IfcZShapeProfileDef* value) override;
    bool visitIfcAsymmetricIShapeProfileDef(ifc2x3::IfcAsymmetricIShapeProfileDef* value) override;
    bool visitIfcCircleHollowProfileDef(ifc2x3::IfcCircleHollowProfileDef* value) override;
    bool visitIfcRectangleHollowProfileDef(ifc2x3::IfcRectangleHollowProfileDef* value) override;
    bool visitIfcRectangleProfileDef(ifc2x3::IfcRectangleProfileDef* value) override;
    bool visitIfcCircleProfileDef(ifc2x3::IfcCircleProfileDef* value) override;

    bool visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value) override;
    bool visitIfcPolyline(ifc2x3::IfcPolyline* value) override;
    bool visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value) override;
    bool visitIfcClosedShell(ifc2x3::IfcClosedShell* value) override;
    bool visitIfcFace(ifc2x3::IfcFace* value) override;
    bool visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value) override;
    bool visitIfcFaceBound(ifc2x3::IfcFaceBound* value) override;
    bool visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value) override;

    std::list<Vec3> getPoints() const;
    Vec3 getVectorDirection() const;
    float getHauteurExtrusion() const;
    Matrix4 getTransformation() const;
    Matrix4 getTransformationOperator3D() const;
    float getDetermiantOperator3D() const;
    bool getIsMappedItem() const;
    bool getScale() const;
    std::vector<std::string> getNameItems() const;
    std::string getOuterCurveName() const;

    //get opération boolean
    std::vector<bool> getAgreementHalfBool() const;
    std::vector<bool> getAgreementPolygonalBool() const;
    std::list<Matrix4> getPlanPolygonal();
    std::list<Matrix4> getLocationPolygonal() const;
    std::vector<int> getNbArgPolyline() const;
    std::vector<std::string> getListEntityHalf() const;
    std::vector<std::string> getListEntityPolygonal() const;

    //get profilDef
    I_profilDef getIprofilDef() const;
    L_profilDef getLprofilDef() const;
    T_profilDef getTprofilDef() const;
    U_profilDef getUprofilDef() const;
    C_profilDef getCprofilDef() const;
    Z_profilDef getZprofilDef() const;
    AsymmetricI_profilDef getAsymmetricIprofilDef() const;
    CircleHollow_profilDef getCircleHollowprofilDef() const;
    RectangleHollow_profilDef getRectangleHollowprofilDef() const;
    Circle_profilDef getCircleprofilDef() const;
    Rectangle_profilDef getRectangleprofilDef() const;
    std::string getNameProfildef() const;

    int getkeyForVoid() const;
    int getnbPolylineCompositeCurve() const;
    CompositeCurveSegment getCompositeCurveSegment() const;

    std::vector<int> getListNbArgFace() const;
    std::vector<int> getListKeyItem() const;
    bool getOrientatationFace() const;

    Box getBox() const;
    Style getStyle() const;

    Vec3 SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine);
    Vec3 SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur);
    void transformPoints(const Matrix4& transform);

};
