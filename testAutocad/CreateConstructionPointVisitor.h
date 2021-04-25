#pragma once

#include <ifc2x3/InheritVisitor.h>

#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

typedef mathfu::Vector<float, 3> Vec3;
typedef mathfu::Matrix<float, 4> Matrix4;

class CreateConstructionPointVisitor : public ifc2x3::InheritVisitor
{
    std::list<Vec3> _points;
    Vec3 VecteurExtrusion;

public:
    //! Constructor
    CreateConstructionPointVisitor();

    bool visitIfcProduct(ifc2x3::IfcProduct* value) override;
    bool visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value) override;
    bool visitIfcProductDefinitionShape(ifc2x3::IfcProductDefinitionShape* value) override;
    bool visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value) override;
    bool visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value) override;
    bool visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value) override;
    bool visitIfcPolyline(ifc2x3::IfcPolyline* value) override;
    bool visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value) override;
    bool visitIfcClosedShell(ifc2x3::IfcClosedShell* value) override;
    bool visitIfcFace(ifc2x3::IfcFace* value) override;
    bool visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value) override;
    bool visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value) override;

    std::list<Vec3> getPoints() const;
    Vec3 getVectorDirection() const;
    void transformPoints(const Matrix4& transform);
};
