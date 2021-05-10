#include "CreateConstructionPointVisitor.h"
#include "ComputePlacementVisitor.h"

CreateConstructionPointVisitor::CreateConstructionPointVisitor() :
    ifc2x3::InheritVisitor()
{

}

bool CreateConstructionPointVisitor::visitIfcProduct(ifc2x3::IfcProduct* value)
{
    if(value->testRepresentation())
    {
        return value->getRepresentation()->acceptVisitor(this);
    }

    return true;
}



bool CreateConstructionPointVisitor::visitIfcProductRepresentation(
    ifc2x3::IfcProductRepresentation* value)
{
    for(auto representation : value->getRepresentations())
    {
        if(representation->acceptVisitor(this))
        {
            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcProductDefinitionShape(
    ifc2x3::IfcProductDefinitionShape* value)
{
    for (auto representation : value->getRepresentations())
    {
        if (!representation->acceptVisitor(this)) {
            return false;
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcShapeRepresentation(
    ifc2x3::IfcShapeRepresentation* value)
{
    for(auto item : value->getItems())
    {
        if(item->acceptVisitor(this))
        {
            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcBooleanClippingResult(ifc2x3::IfcBooleanClippingResult* value)
{
    bool r1 = false, r2 = false;

    if (value->testFirstOperand())
    {
        auto op1 = value->getFirstOperand();

        switch (op1->currentType())
        {
        case ifc2x3::IfcBooleanOperand::IFCBOOLEANRESULT:
            r1 = op1->getIfcBooleanResult()->acceptVisitor(this);
            break;
        case ifc2x3::IfcBooleanOperand::IFCCSGPRIMITIVE3D:
            r1 = op1->getIfcCsgPrimitive3D()->acceptVisitor(this);
            break;
        case ifc2x3::IfcBooleanOperand::IFCHALFSPACESOLID:
            r1 = op1->getIfcHalfSpaceSolid()->acceptVisitor(this);
            break;
        case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
            r1 = op1->getIfcSolidModel()->acceptVisitor(this);
            break;
        }
    }

    if (value->testSecondOperand())
    {
        auto op2 = value->getSecondOperand();

        switch (op2->currentType())
        {
        case ifc2x3::IfcBooleanOperand::IFCBOOLEANRESULT:
            r2 = op2->getIfcBooleanResult()->acceptVisitor(this);
            break;
        case ifc2x3::IfcBooleanOperand::IFCCSGPRIMITIVE3D:
            r2 = op2->getIfcCsgPrimitive3D()->acceptVisitor(this);
            break;
        case ifc2x3::IfcBooleanOperand::IFCHALFSPACESOLID:
            r2 = op2->getIfcHalfSpaceSolid()->acceptVisitor(this);
            break;
        case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
            r2 = op2->getIfcSolidModel()->acceptVisitor(this);
            break;
        }
    }

    return r1 && r2;
}

bool CreateConstructionPointVisitor::visitIfcRepresentationMap(
    ifc2x3::IfcRepresentationMap* value)
{
    if (value->testMappedRepresentation())
    {
        if (value->getMappedRepresentation()->acceptVisitor(this))
        {
            if (value->testMappingOrigin()
                && value->getMappingOrigin()->currentType() ==
                ifc2x3::IfcAxis2Placement::IFCAXIS2PLACEMENT3D)
            {
                Matrix4 transformation = ComputePlacementVisitor::getTransformation(
                    value->getMappingOrigin()->getIfcAxis2Placement3D());

                transformPoints(transformation);
            }

            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcMappedItem(
    ifc2x3::IfcMappedItem* value)
{
    if (value->testMappingSource())
    {
        if (value->getMappingSource()->acceptVisitor(this))
        {
            if (value->testMappingTarget())
            {
                Matrix4 transform = ComputePlacementVisitor::getTransformation(
                    value->getMappingTarget());

                transformPoints(transform);
            }

            return true;
        }
    }

    return false;
}

std::string Agreement;

bool CreateConstructionPointVisitor::visitIfcPolygonalBoundedHalfSpace(
    ifc2x3::IfcPolygonalBoundedHalfSpace* value)
{
    if (value->testPolygonalBoundary())
    {
        if (value->getPolygonalBoundary()->acceptVisitor(this))
        {
            if (value->testPosition())
            {
                Matrix4 transform = ComputePlacementVisitor::getTransformation(value->getPosition());
                transformPoints(transform);
            }

            if (value->testAgreementFlag())
            {
                Agreement = value->getAgreementFlag();
            }
            
            return true;
        }
    }
    return false;
}

Vec3 originePlan(0.f, 0.f, 0.f);
Vec3 direction1Plan(0.f, 0.f, 0.f);
Vec3 direction2Plan(0.f, 0.f, 0.f);


bool CreateConstructionPointVisitor::visitIfcPlane(
    ifc2x3::IfcPlane* value)
{
    if (value->testPosition())
    {
        auto axisPlacement = value->getPosition();

        if (axisPlacement->testLocation())
        {
            originePlan = SwitchIfcCartesianPointToVecteur3D(axisPlacement->getLocation());
        }

        if (axisPlacement->testAxis())
        {
            direction1Plan = SwitchIfcDirectionToVecteur3D(axisPlacement->getAxis());
            direction1Plan.Normalize();
        }

        if (axisPlacement->testRefDirection())
        {
            direction2Plan = SwitchIfcDirectionToVecteur3D(axisPlacement->getRefDirection());
            direction2Plan.Normalize();
        }

        return true;
    }
    
    return false;
}

Vec3 extrusionVector(0., 0., 1.);

bool CreateConstructionPointVisitor::visitIfcExtrudedAreaSolid(
    ifc2x3::IfcExtrudedAreaSolid* value)
{
    if(value->testSweptArea())
    {
        if(value->getSweptArea()->acceptVisitor(this))
        {
            Matrix4 transformation = ComputePlacementVisitor::getTransformation(
                                         value->getPosition());

            transformPoints(transformation);

            if(value->testExtrudedDirection())
            {
                extrusionVector = ComputePlacementVisitor::getDirection(
                                      value->getExtrudedDirection());
                extrusionVector.Normalize();

                extrusionVector *= value->getDepth();
            }

            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcArbitraryClosedProfileDef(
    ifc2x3::IfcArbitraryClosedProfileDef* value)
{
    if(value->testOuterCurve())
    {
        return value->getOuterCurve()->acceptVisitor(this);
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcPolyline(ifc2x3::IfcPolyline*
                                                      value)
{
    for(auto point : value->getPoints())
    {
        _points.push_back(ComputePlacementVisitor::getPoint(point.get()));
    }

    _points.pop_back();

    return _points.empty() == false;
}

bool CreateConstructionPointVisitor::visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value)
{
    if (value->testOuter())
    {
        return value->getOuter()->acceptVisitor(this);
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcClosedShell(ifc2x3::IfcClosedShell* value)
{
    if (value->testCfsFaces())
    {
        for (auto face : value->getCfsFaces())
        {
            if (!face->acceptVisitor(this))
            {
                return false;
            }
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcFace(ifc2x3::IfcFace* value)
{
    if (value->testBounds())
    {
        for (auto bound : value->getBounds())
        {
            if (!bound->acceptVisitor(this))
            {
                return false;
            }
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value)
{
    auto t1 = value->getBound();

    if (value->testBound())
    {
        return value->getBound()->acceptVisitor(this);
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value)
{
    for (auto point : value->getPolygon())
    {
        auto v = ComputePlacementVisitor::getPoint(point.get());
        _points.push_back(v);
    }

    return _points.empty() == false;
}

Vec3 CreateConstructionPointVisitor::SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value)
{
    auto listPoint = value->getCoordinates();

    originePlan.x() = listPoint.at(0);
    originePlan.y() = listPoint.at(1);
    originePlan.z() = listPoint.at(2);

    return originePlan;
}

Vec3 CreateConstructionPointVisitor::SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value)
{
    auto listPoint = value->getDirectionRatios();

    originePlan.x() = listPoint.at(0);
    originePlan.y() = listPoint.at(1);
    originePlan.z() = listPoint.at(2);

    return originePlan;
}

std::list<Vec3> CreateConstructionPointVisitor::getPoints() const
{
    return _points;
}

Vec3 CreateConstructionPointVisitor::getVectorDirection() const
{
    return extrusionVector;
}

std::string CreateConstructionPointVisitor::getAgreementBool() const
{
    return Agreement;
}

Vec3 CreateConstructionPointVisitor::getOriginePlan() const
{
    return originePlan;
}

Vec3 CreateConstructionPointVisitor::getDirection1Plan() const
{
    return direction1Plan;
}

Vec3 CreateConstructionPointVisitor::getDirection2Plan() const
{
    return direction2Plan;
}

void CreateConstructionPointVisitor::transformPoints(const Matrix4& transform)
{
    std::list<Vec3> tmpPoints = _points;

    _points.clear();

    for(const auto& point : tmpPoints)
    {
        _points.push_back(transform * point);
    }
}

