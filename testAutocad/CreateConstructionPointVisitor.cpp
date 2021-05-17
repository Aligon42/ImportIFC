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

bool CreateConstructionPointVisitor::visitIfcHalfSpaceSolid(
    ifc2x3::IfcHalfSpaceSolid* value)
{
    if (value->testBaseSurface())
    {
        value->getBaseSurface()->acceptVisitor(this);
    }
    if (value->testAgreementFlag())
    {
        AgreementHalf.push_back(value->getAgreementFlag());
    }

    entityHalf.push_back(value->getClassType().getName());

    return true;
}

bool CreateConstructionPointVisitor::visitIfcPolygonalBoundedHalfSpace(
    ifc2x3::IfcPolygonalBoundedHalfSpace* value)
{
    if (value->testPolygonalBoundary())
    {
        if (value->getPolygonalBoundary()->acceptVisitor(this))
        {
            if (value->testPosition())
            {
                transform = ComputePlacementVisitor::getTransformation(value->getPosition());
                transformPoints(transform);

                listLocationPolygonal.push_back(transform);
            }

            if (value->testAgreementFlag())
            {
                AgreementPolygonal.push_back(value->getAgreementFlag());
            }

            if (value->testBaseSurface())
            {
                value->getBaseSurface()->acceptVisitor(this);
            }

            entityPolygonal.push_back(value->getClassType().getName());

            
            return true;
        }
    }
    return false;
}

//bool CreateConstructionPointVisitor::visitIfcCompositeCurve(
//    ifc2x3::IfcCompositeCurve* value) 
//{
//    if (value->testSelfIntersect())
//    {
//        AgreementCompositeCurve.push_back(value->getSelfIntersect());
//    }
//
//    if (value->testSegments())
//    {
//        for (auto segment : value->getSegments())
//        {
//            if (segment->acceptVisitor(this))
//            {
//                return true;
//            }
//        }
//    }
//
//    return false;
//}
//
//bool CreateConstructionPointVisitor::visitIfcCompositeCurveSegment(
//    ifc2x3::IfcCompositeCurveSegment* value)
//{
//    if (value->testTransition())
//    {
//        transitionCompositeCurveSegment.push_back(value->getTransition());
//    }
//    if (value->testSameSense())
//    {
//        sameSenseCompositeCurveSegment.push_back(value->getSameSense());
//    }
//    if (value->testParentCurve())
//    {
//        value->getParentCurve()->acceptVisitor(this);
//        nameParentCurve = value->getParentCurve()->getClassType().getName();
//        return true;
//    }
//
//    return false;
//}
//
//bool CreateConstructionPointVisitor::visitIfcTrimmedCurve(
//    ifc2x3::IfcTrimmedCurve* value)
//{
//    if (value->testTrim1())
//    {
//       
//    }
//    if (value->testTrim2())
//    {
//
//    }
//    if (value->testSenseAgreement())
//    {
//        senseAgreementTrimmedCurve.push_back(value->getSenseAgreement());
//    }
//    if (value->testMasterRepresentation())
//    {
//
//    }
//    if (value->testBasisCurve())
//    {
//        value->getBasisCurve()->acceptVisitor(this);
//    }
//
//    return true;
//}

bool CreateConstructionPointVisitor::visitIfcCircle(
    ifc2x3::IfcCircle* value) 
{
    if (value->testRadius())
    {
        radiusCircle.push_back(value->getRadius());
    }
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }
    return true;
}

bool CreateConstructionPointVisitor::visitIfcPlane(
    ifc2x3::IfcPlane* value)
{
    if (value->testPosition())
    {
        auto axisPlacement = value->getPosition();

        Vec3 originePlan(0.0f, 0.0f, 0.0f);
        Vec3 direction1Plan(1.0f, 0.0f, 0.0f);
        Vec3 direction2Plan(0.0f, 0.0f, 1.0f);

        if (axisPlacement->testLocation())
        {
            SwitchIfcCartesianPointToVecteur3D(axisPlacement->getLocation(), originePlan);
        }

        if (axisPlacement->testAxis())
        {
            SwitchIfcDirectionToVecteur3D(axisPlacement->getAxis(), direction1Plan);
        }

        if (axisPlacement->testRefDirection())
        {
            SwitchIfcDirectionToVecteur3D(axisPlacement->getRefDirection(), direction2Plan);
        }

        Matrix4 plan(
            direction2Plan.x(), direction2Plan.y(), direction2Plan.z(), 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            direction1Plan.x(), direction1Plan.y(), direction1Plan.z(), 0.0f,
            originePlan.x(), originePlan.y(), originePlan.z(), 0.0f
            );

        listPlan.push_back(plan);

        return true;
    }
    
    return false;
}

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

bool CreateConstructionPointVisitor::visitIfcIShapeProfileDef(
    ifc2x3::IfcIShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    
    IprofilDef.OverallWidth = (float) value->getOverallWidth();
    IprofilDef.OverallDepth = (float)value->getOverallDepth();
    IprofilDef.webThickness = (float)value->getWebThickness();
    IprofilDef.flangeThickness = (float)value->getFlangeThickness();
    IprofilDef.filletRadius = (float)value->getFilletRadius();
    IprofilDef.nbArg = 5;

    return true;
}

bool CreateConstructionPointVisitor::visitIfcLShapeProfileDef(
    ifc2x3::IfcLShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    L_profilDef LprofilDef;
    LprofilDef.Depth = (float)value->getDepth();
    LprofilDef.Width = (float)value->getWidth();
    LprofilDef.Thickness = (float)value->getThickness();
    LprofilDef.FilletRadius = (float)value->getFilletRadius();
    LprofilDef.EdgeRadius = (float)value->getEdgeRadius();
    LprofilDef.nbArg = 5;
    if (value->testLegSlope())
    {
        LprofilDef.LegSlope = (float)value->getLegSlope();
        LprofilDef.nbArg = 6;
    }
    

    return true;
}

bool CreateConstructionPointVisitor::visitIfcTShapeProfileDef(
    ifc2x3::IfcTShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    T_profilDef TprofilDef;
    TprofilDef.Depth = (float)value->getDepth();
    TprofilDef.FlangeWidth = (float)value->getFlangeWidth();
    TprofilDef.WebThickness = (float)value->getWebThickness();
    TprofilDef.FlangeThickness = (float)value->getFlangeThickness();
    TprofilDef.FilletRadius = (float)value->getFilletRadius();
    TprofilDef.FlangeEdgeRadius = (float)value->getFlangeEdgeRadius();
    TprofilDef.WebEdgeRadius = (float)value->getWebEdgeRadius();
    TprofilDef.nbArg = 7;
    if (value->testWebSlope() && value->testFlangeSlope())
    {
        TprofilDef.WebSlope = (float)value->getWebSlope();
        TprofilDef.FlangeSlope = (float)value->getFlangeSlope();
        TprofilDef.nbArg = 9;
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcUShapeProfileDef(
    ifc2x3::IfcUShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    U_profilDef UprofilDef;
    UprofilDef.Depth = (float)value->getDepth();
    UprofilDef.FlangeWidth = (float)value->getFlangeWidth();
    UprofilDef.WebThickness = (float)value->getWebThickness();
    UprofilDef.FlangeThickness = (float)value->getFlangeThickness();
    UprofilDef.FilletRadius = (float)value->getFilletRadius();
    UprofilDef.nbArg = 5;
    if (value->testEdgeRadius() && value->testFlangeSlope())
    {
        UprofilDef.EdgeRadius = (float)value->getEdgeRadius();
        UprofilDef.FlangeSlope = (float)value->getFlangeSlope();
        UprofilDef.nbArg = 7;
    }
    

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCShapeProfileDef(
    ifc2x3::IfcCShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    C_profilDef CprofilDef;
    CprofilDef.Depth = (float)value->getDepth();
    CprofilDef.Width = (float)value->getWidth();
    CprofilDef.WallThickness = (float)value->getWallThickness();
    CprofilDef.Girth = (float)value->getGirth();
    CprofilDef.InternalFilletRadius = (float)value->getInternalFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcZShapeProfileDef(
    ifc2x3::IfcZShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    Z_profilDef ZprofilDef;
    ZprofilDef.Depth = (float)value->getDepth();
    ZprofilDef.FlangeWidth = (float)value->getFlangeWidth();
    ZprofilDef.WebThickness = (float)value->getWebThickness();
    ZprofilDef.FlangeThickness = (float)value->getFlangeThickness();
    ZprofilDef.FilletRadius = (float)value->getFilletRadius();
    ZprofilDef.EdgeRadius = (float)value->getEdgeRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcAsymmetricIShapeProfileDef(
    ifc2x3::IfcAsymmetricIShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    AsymmetricI_profilDef AsymmetricIprofilDef;
    AsymmetricIprofilDef.OverallWidth = (float)value->getOverallWidth();
    AsymmetricIprofilDef.OverallDepth = (float)value->getOverallDepth();
    AsymmetricIprofilDef.WebThickness = (float)value->getWebThickness();
    AsymmetricIprofilDef.FlangeThickness = (float)value->getFlangeThickness();
    AsymmetricIprofilDef.FlangeFilletRadius = (float)value->getTopFlangeFilletRadius();
    AsymmetricIprofilDef.TopFlangeWidth = (float)value->getTopFlangeWidth();
    AsymmetricIprofilDef.TopFlangeThickness = (float)value->getTopFlangeThickness();
    AsymmetricIprofilDef.TopFlangeFilletRadius = (float)value->getTopFlangeFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircleHollowProfileDef(
    ifc2x3::IfcCircleHollowProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    CircleHollow_profilDef circleHollowProfilDef;
    circleHollowProfilDef.Radius = (float)value->getRadius();
    circleHollowProfilDef.WallThickness = (float)value->getWallThickness();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleHollowProfileDef(
    ifc2x3::IfcRectangleHollowProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    RectangleHollow_profilDef rectangleHollowProfilDef;
    rectangleHollowProfilDef.XDim = (float)value->getXDim();
    rectangleHollowProfilDef.YDim = (float)value->getYDim();
    rectangleHollowProfilDef.WallThickness = (float)value->getWallThickness();
    rectangleHollowProfilDef.InnerFilletRadius = (float)value->getInnerFilletRadius();
    rectangleHollowProfilDef.OuteerFilletRadius = (float)value->getOuterFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleProfileDef(
    ifc2x3::IfcRectangleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    Rectangle_profilDef rectangleProfilDef;
    rectangleProfilDef.XDim = (float)value->getXDim();
    rectangleProfilDef.YDim = (float)value->getYDim();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircleProfileDef(
    ifc2x3::IfcCircleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    Circle_profilDef circleProfilDef;
    circleProfilDef.Radius = (float)value->getRadius();

    return true;
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
    int size = _points.size();

    for(auto point : value->getPoints())
    {
        _points.push_back(ComputePlacementVisitor::getPoint(point.get()));
    }
    
    _points.pop_back();

    listNbArgPolyline.push_back(_points.size() - size);

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

void CreateConstructionPointVisitor::SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine)
{
    auto listPoint = value->getCoordinates();

    outOrigine.x() = (float) listPoint.at(0);
    outOrigine.y() = (float) listPoint.at(1);
    outOrigine.z() = (float) listPoint.at(2);
}

void CreateConstructionPointVisitor::SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur)
{
    auto listPoint = value->getDirectionRatios();

    outVecteur.x() = (float) listPoint.at(0);
    outVecteur.y() = (float) listPoint.at(1);
    outVecteur.z() = (float) listPoint.at(2);
}

std::list<Vec3> CreateConstructionPointVisitor::getPoints() const
{
    return _points;
}

Vec3 CreateConstructionPointVisitor::getVectorDirection() const
{
    return extrusionVector;
}

std::vector<bool> CreateConstructionPointVisitor::getAgreementHalfBool() const
{
    return AgreementHalf;
}

std::vector<bool> CreateConstructionPointVisitor::getAgreementPolygonalBool() const
{
    return AgreementPolygonal;
}

std::list<Matrix4> CreateConstructionPointVisitor::getPlanPolygonal()
{
    return listPlan;
}

std::list<Matrix4> CreateConstructionPointVisitor::getLocationPolygonal() const
{
    return listLocationPolygonal;
}

std::vector<int> CreateConstructionPointVisitor::getNbArgPolyline() const
{
    return listNbArgPolyline;
}

std::vector<std::string> CreateConstructionPointVisitor::getListEntityHalf() const
{
    return entityHalf;
}

std::vector<std::string> CreateConstructionPointVisitor::getListEntityPolygonal() const
{
    return entityPolygonal;
}

//I_profilDef CreateConstructionPointVisitor::getIprofilDef() const
//{
//    
//    return IprofilDef;
//}

void CreateConstructionPointVisitor::transformPoints(const Matrix4& transform)
{
    std::list<Vec3> tmpPoints = _points;

    auto size = -1;
    
    if (listNbArgPolyline.size() > 2)
    {
        size = 0;
        for (int i = 0; i < listNbArgPolyline.size() - 1; i++)
        {
            size += listNbArgPolyline[i];
        }
    }
    
    _points.clear();

    int count = 1;
    for(const auto& point : tmpPoints)
    {
        if (count > size)
            _points.push_back(transform * point);
        else
            _points.push_back(point);

        count++;
    }
}

