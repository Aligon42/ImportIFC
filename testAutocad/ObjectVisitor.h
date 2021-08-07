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

#include "Object.h"

typedef mathfu::Vector<double, 3> Vec3;
typedef mathfu::Matrix<double, 4> Matrix4;

class ObjectVisitor : public ifc2x3::InheritVisitor
{
public:
    //! Constructor
    ObjectVisitor(int visitingDepth = 0);
    ObjectVisitor(IFCObject* obj, int visitingDepth = 0);
    ObjectVisitor(IFCObject* obj, IFCShapeRepresentation& shape, int visitingDepth = 0);

    bool visitIfcProduct(ifc2x3::IfcProduct* value) override;
    bool visitIfcSite(ifc2x3::IfcSite* value) override;
    bool visitIfcRelVoidsElement(ifc2x3::IfcRelVoidsElement* value) override;
    bool visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value) override;
    bool visitIfcProductDefinitionShape(ifc2x3::IfcProductDefinitionShape* value) override;
    bool visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value) override;
    bool visitIfcBooleanClippingResult(ifc2x3::IfcBooleanClippingResult* value) override;
    bool visitIfcBooleanResult(ifc2x3::IfcBooleanResult* value) override;
    bool visitIfcRepresentationMap(ifc2x3::IfcRepresentationMap* value) override;
    bool visitIfcFaceBasedSurfaceModel(ifc2x3::IfcFaceBasedSurfaceModel* value) override;
    bool visitIfcConnectedFaceSet(ifc2x3::IfcConnectedFaceSet* value) override;
    bool visitIfcShellBasedSurfaceModel(ifc2x3::IfcShellBasedSurfaceModel* value) override;
    bool visitIfcShell(ifc2x3::IfcShell* value) override;
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

    IFCObject* getIfcObject();
    inline Style getStyle() const
    {
        return m_Style;
    }

private:
    void transformPoints(const Matrix4& transform);
    void SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine);
    void SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur);

    IFCShapeRepresentation& getShapeRepresentation(); 
    std::vector<IFCShapeRepresentation>& getShapeRepresentations();

private:
    int m_VisitingDepth;
    bool m_VisitingCompositeCurve = false;
    bool m_VisitingFaces = false;

    IFCShapeRepresentation m_IfcShapeRepresentation;
    std::vector<IFCShapeRepresentation> m_ShapeRepresentations;
    IFCObject* m_IfcObject;
    Style m_Style;
    RGBA m_Color;
    Face m_Face;
};