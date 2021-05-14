#pragma once
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
    float I_OverallWidth;
    float OverallDepth;
    float webThickness;
    float flangeThickness;
    float filletRadius;
};

struct L_profilDef
{
    float Depth;
    float Width;
    float Thickness;
    float FilletRadius;
    float EdgeRadius;
    float LegSlope;
    float CentreOfGravityInx;
    float CentreOfGravityInY; 
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
    float CentreOfGravityInY;
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
    float CentreOfGravityInY;
};

class CreateConstructionPointVisitor : public ifc2x3::InheritVisitor
{
private:

    std::list<Vec3> _points;
    Vec3 extrusionVector;
    bool Agreement;
    Matrix4 transform;
    std::list<Matrix4> listPlan;
    std::list<Matrix4> listLocationPolygonal;
    Matrix4 _transformation{ Matrix4::Identity() };
    int nbArg;
    std::vector<int> listNbArgPolyline;

    //I_profilDef
    


public:
    //! Constructor
    CreateConstructionPointVisitor();

    bool visitIfcProduct(ifc2x3::IfcProduct* value) override;
    bool visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value) override;
    bool visitIfcProductDefinitionShape(ifc2x3::IfcProductDefinitionShape* value) override;
    bool visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value) override;
    bool visitIfcBooleanClippingResult(ifc2x3::IfcBooleanClippingResult* value) override;
    bool visitIfcMappedItem(ifc2x3::IfcMappedItem* value) override;
    bool visitIfcHalfSpaceSolid(ifc2x3::IfcHalfSpaceSolid* value) override;
    bool visitIfcPolygonalBoundedHalfSpace(ifc2x3::IfcPolygonalBoundedHalfSpace* value) override;
    bool visitIfcRepresentationMap(ifc2x3::IfcRepresentationMap* value) override;
    bool visitIfcPlane(ifc2x3::IfcPlane* value) override;
    bool visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value) override;
    //profilDef
    bool visitIfcIShapeProfileDef(ifc2x3::IfcIShapeProfileDef* value) override;
    bool visitIfcLShapeProfileDef(ifc2x3::IfcLShapeProfileDef* value) override;
    bool visitIfcTShapeProfileDef(ifc2x3::IfcTShapeProfileDef* value) override;

    bool visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value) override;
    bool visitIfcPolyline(ifc2x3::IfcPolyline* value) override;
    bool visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value) override;
    bool visitIfcClosedShell(ifc2x3::IfcClosedShell* value) override;
    bool visitIfcFace(ifc2x3::IfcFace* value) override;
    bool visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value) override;
    bool visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value) override;

    std::list<Vec3> getPoints() const;
    Vec3 getVectorDirection() const;
    bool getAgreementBool() const;
    std::list<Matrix4> getPlanPolygonal();
    std::list<Matrix4> getLocationPolygonal() const;
    std::vector<int> getNbArgPolyline() const;
    void SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine);
    void SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur);
    void transformPoints(const Matrix4& transform);
};
