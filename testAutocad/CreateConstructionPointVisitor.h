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
#include "ProfilDef.h"
#include "DataObject.h"

typedef mathfu::Vector<float, 3> Vec3;
typedef mathfu::Matrix<float, 4> Matrix4;

static TrimmedCurve _trimmedCurve;
static CompositeCurveSegment _compositeCurveSegment;

class CreateConstructionPointVisitor : public ifc2x3::InheritVisitor
{
private:
    std::vector<std::string> nameItems;

    //opération boolean
    std::vector<bool> AgreementCompositeCurve;
    std::vector<ifc2x3::IfcTransitionCode> transitionCompositeCurveSegment;
    std::vector<bool> sameSenseCompositeCurveSegment;
    std::vector<int> trim1TrimmedCurve;
    std::vector<int> trim2TrimmedCurve;
    std::vector<bool> senseAgreementTrimmedCurve;
    std::string nameParentCurve;
    std::vector<int> radiusCircle;
    
    //profilDef
    BaseProfilDef* _profilDef;
    BaseObject* _object;

    int keyForVoid;
    bool _exploringRelVoid = false;
    bool _compositeCurve = false;
    int _nbArgToCompute = 0;

public:
    //! Constructor
    CreateConstructionPointVisitor();
    ~CreateConstructionPointVisitor();

    bool visitIfcProduct(ifc2x3::IfcProduct* value) override;
    bool visitIfcRelVoidsElement(ifc2x3::IfcRelVoidsElement* value) override;
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
    bool visitIfcAxis2Placement2D(ifc2x3::IfcAxis2Placement2D* value) override;
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

    std::vector<std::string> getNameItems() const;

    //get profilDef
    inline BaseProfilDef* GetProfilDef() { return _profilDef; }

    int getkeyForVoid() const;
    TrimmedCurve getTrimmedCurve() const;
    CompositeCurveSegment getCompositeCurveSegment() const;

    void SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine);
    void SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur);
    void transformPoints(const Matrix4& transform);

    inline ObjectToConstruct* getObjectToConstructs() { return (ObjectToConstruct*)_object; }
    inline ObjectVoid getObjectVoid() { return *(ObjectVoid*)_object; }
};
