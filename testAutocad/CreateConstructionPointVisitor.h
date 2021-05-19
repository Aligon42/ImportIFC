#pragma once
#include <string>
#include <ifc2x3/InheritVisitor.h>
#include "ProfilDef.h"

#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

typedef mathfu::Vector<float, 3> Vec3;
typedef mathfu::Matrix<float, 4> Matrix4;

class CreateConstructionPointVisitor : public ifc2x3::InheritVisitor
{
private:

    Matrix4 _transformation{ Matrix4::Identity() };
    Matrix4 transform;

    std::list<Vec3> _points;
    Vec3 extrusionVector;
    Matrix4 transformation;

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
    std::string _nameProfilDef;
    BaseProfilDef* _profilDef;


public:
    //! Constructor
    CreateConstructionPointVisitor();
    ~CreateConstructionPointVisitor();

    bool visitIfcProduct(ifc2x3::IfcProduct* value) override;
    bool visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value) override;
    bool visitIfcProductDefinitionShape(ifc2x3::IfcProductDefinitionShape* value) override;
    bool visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value) override;
    bool visitIfcBooleanClippingResult(ifc2x3::IfcBooleanClippingResult* value) override;
    bool visitIfcMappedItem(ifc2x3::IfcMappedItem* value) override;
    bool visitIfcHalfSpaceSolid(ifc2x3::IfcHalfSpaceSolid* value) override;
    bool visitIfcPolygonalBoundedHalfSpace(ifc2x3::IfcPolygonalBoundedHalfSpace* value) override;
    bool visitIfcCompositeCurve(ifc2x3::IfcCompositeCurve* value) override;
    bool visitIfcCompositeCurveSegment(ifc2x3::IfcCompositeCurveSegment* value) override;
    bool visitIfcTrimmedCurve(ifc2x3::IfcTrimmedCurve* value) override;
    bool visitIfcCircle(ifc2x3::IfcCircle* value) override; 
    bool visitIfcRepresentationMap(ifc2x3::IfcRepresentationMap* value) override;
    bool visitIfcPlane(ifc2x3::IfcPlane* value) override;
    bool visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value) override;

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
    bool visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value) override;

    std::list<Vec3> getPoints() const;
    Vec3 getVectorDirection() const;
    Matrix4 getTransformation() const;

    //get opération boolean
    std::vector<bool> getAgreementHalfBool() const;
    std::vector<bool> getAgreementPolygonalBool() const;
    std::list<Matrix4> getPlanPolygonal();
    std::list<Matrix4> getLocationPolygonal() const;
    std::vector<int> getNbArgPolyline() const;
    std::vector<std::string> getListEntityHalf() const;
    std::vector<std::string> getListEntityPolygonal() const;

    //get profilDef
    BaseProfilDef* GetProfilDef() { return _profilDef; }
    std::string getNameProfildef() const;
    int getnbArgIProfilDef() const;


    void SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine);
    void SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur);
    void transformPoints(const Matrix4& transform);

};
