#include "CreateConstructionPointVisitor.h"
#include "ComputePlacementVisitor.h"
#include "ProfilDef.h"

CreateConstructionPointVisitor::CreateConstructionPointVisitor() :
    ifc2x3::InheritVisitor()
{

}

bool CreateConstructionPointVisitor::visitIfcProduct(ifc2x3::IfcProduct* value)
{
    if(value->testRepresentation())
    {
        if (!_exploringRelVoid)
            _object = CreateRef<ObjectToConstruct>();

        return value->getRepresentation()->acceptVisitor(this);
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRelVoidsElement(
    ifc2x3::IfcRelVoidsElement* value)
{
    _exploringRelVoid = true;
    _object = CreateRef<ObjectVoid>();
    if (value->testRelatedOpeningElement())
    {
        value->getRelatedOpeningElement()->acceptVisitor(this);
    }
    if (value->testRelatingBuildingElement())
    {
        _object->Key = value->getRelatingBuildingElement()->getKey();
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
        nameItems.push_back(item->getType().getName());

        if (value->getRepresentationType() == "Curve2D")
        {
            ElementToConstruct el;
            el.Key = value->getKey();
            el.Type = value->type();

            if (!_exploringRelVoid)
                ((ObjectToConstruct*)_object.get())->ElementsToConstruct.push_back(el);
            else
                ((ObjectVoid*)_object.get())->ElementsToConstruct.push_back(el);
        }

        if(item->acceptVisitor(this))
        {
            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcBooleanClippingResult(
    ifc2x3::IfcBooleanClippingResult* value)
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
        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].AgreementHalf.push_back(value->getAgreementFlag());
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].AgreementHalf.push_back(value->getAgreementFlag());
    }

    if (!_exploringRelVoid)
        ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].EntitiesHalf.push_back(value->getClassType().getName());
    else
        ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].EntitiesHalf.push_back(value->getClassType().getName());

    return true;
}

bool CreateConstructionPointVisitor::visitIfcPolygonalBoundedHalfSpace(
    ifc2x3::IfcPolygonalBoundedHalfSpace* value)
{
    if (value->testPolygonalBoundary())
    {
        ElementToConstruct el;
        el.Key = value->getKey();
        el.Type = value->type();

        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct.push_back(el);
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct.push_back(el);

        if (value->getPolygonalBoundary()->acceptVisitor(this))
        {
            if (value->testPosition())
            {
                auto transform = ComputePlacementVisitor::getTransformation(value->getPosition());
                transformPoints(transform);

                if (!_exploringRelVoid)
                    ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].LocationsPolygonal.push_back(transform);
                else
                    ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].LocationsPolygonal.push_back(transform);
            }

            if (value->testAgreementFlag())
            {
                if (!_exploringRelVoid)
                    ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].AgreementPolygonal.push_back(value->getAgreementFlag());
                else
                    ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].AgreementPolygonal.push_back(value->getAgreementFlag());
            }

            if (value->testBaseSurface())
            {
                value->getBaseSurface()->acceptVisitor(this);
            }

            if (!_exploringRelVoid)
                ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].EntitiesPolygonal.push_back(value->getClassType().getName());
            else
                ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].EntitiesPolygonal.push_back(value->getClassType().getName());
            
            return true;
        }
    }
    return false;
}

bool CreateConstructionPointVisitor::visitIfcCompositeCurve(
    ifc2x3::IfcCompositeCurve* value) 
{
    if (value->testSelfIntersect())
    {
        AgreementCompositeCurve.push_back(value->getSelfIntersect());
    }

    _compositeCurve = true;

    if (value->testSegments())
    {
        for (auto segment : value->getSegments())
        {
            if (!segment->acceptVisitor(this))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcCompositeCurveSegment(
    ifc2x3::IfcCompositeCurveSegment* value)
{
    if (value->testTransition())
    {
        transitionCompositeCurveSegment.push_back(value->getTransition());
    }
    if (value->testSameSense())
    {
        sameSenseCompositeCurveSegment.push_back(value->getSameSense());
    }
    if (value->testParentCurve())
    {
        value->getParentCurve()->acceptVisitor(this);
        nameParentCurve = value->getParentCurve()->getClassType().getName();
        _compositeCurveSegment.listParentCurve.push_back(value->getParentCurve()->getClassType().getName());

        return true;
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcTrimmedCurve(
    ifc2x3::IfcTrimmedCurve* value)
{
    if (value->testTrim1())
    {
        _trimmedCurve.trim1 = value->getTrim1().getLowerBound();
    }
    if (value->testTrim2())
    {
        _trimmedCurve.trim2 = value->getTrim2().getLowerBound();
    }
    if (value->testSenseAgreement())
    {
        _trimmedCurve.senseArgreement = value->getSenseAgreement();
    }
    if (value->testMasterRepresentation())
    {

    }
    if (value->testBasisCurve())
    {
        value->getBasisCurve()->acceptVisitor(this);
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircle(
    ifc2x3::IfcCircle* value) 
{
    if (value->testRadius())
    {
        radiusCircle.push_back(value->getRadius());
        _trimmedCurve.radius = value->getRadius();
    }
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }
    auto index = value->getKey();

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

        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Plans.push_back(plan);
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Plans.push_back(plan);

        return true;
    }
    
    return false;
}

bool CreateConstructionPointVisitor::visitIfcAxis2Placement2D(ifc2x3::IfcAxis2Placement2D* value)
{
    auto index = value->getKey();
    if (value->testLocation())
    {
        value->getLocation()->acceptVisitor(this);
        auto coordonnees = value->getLocation()->getCoordinates();
        _trimmedCurve.centreCircle.x() = coordonnees.at(0);
        _trimmedCurve.centreCircle.y() = coordonnees.at(1);
        _trimmedCurve.centreCircle.z() = 0.0;
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcExtrudedAreaSolid(
    ifc2x3::IfcExtrudedAreaSolid* value)
{
    if(value->testSweptArea())
    {
        ElementToConstruct el;
        el.Key = value->getKey();
        el.Type = value->type();

        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct.push_back(el);
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct.push_back(el);

        if(value->getSweptArea()->acceptVisitor(this))
        {
            auto transform = ComputePlacementVisitor::getTransformation(value->getPosition());
            transformPoints(transform);

            if (!_exploringRelVoid)
                ((ObjectToConstruct*)_object.get())->Transform = transform;
            else
                ((ObjectVoid*)_object.get())->Transform = transform;
            
            if (_exploringRelVoid)
                ((ObjectVoid*)_object.get())->NameProfilDef = value->getSweptArea()->getType().getName();
            else if (_profilDef != nullptr)
                _profilDef->Name = value->getSweptArea()->getType().getName();

            if(value->testExtrudedDirection())
            {
                auto extrusionVector = ComputePlacementVisitor::getDirection(value->getExtrudedDirection());
                extrusionVector.Normalize();
                extrusionVector *= value->getDepth();

                if (!_exploringRelVoid)
                    ((ObjectToConstruct*)_object.get())->VecteurExtrusion = extrusionVector;
                else
                    ((ObjectVoid*)_object.get())->VecteurExtrusion = extrusionVector;
            }

            return true;
        }
    }

    return false;
}

#pragma region ProfilsDef
bool CreateConstructionPointVisitor::visitIfcIShapeProfileDef(
    ifc2x3::IfcIShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = CreateRef<I_profilDef>();

    ((I_profilDef*)_profilDef.get())->OverallWidth = (float) value->getOverallWidth();
    ((I_profilDef*)_profilDef.get())->OverallDepth = (float)value->getOverallDepth();
    ((I_profilDef*)_profilDef.get())->webThickness = (float)value->getWebThickness();
    ((I_profilDef*)_profilDef.get())->flangeThickness = (float)value->getFlangeThickness();
    ((I_profilDef*)_profilDef.get())->filletRadius = (float)value->getFilletRadius();
    ((I_profilDef*)_profilDef.get())->nbArg = 5;

    return true;
}

bool CreateConstructionPointVisitor::visitIfcLShapeProfileDef(
    ifc2x3::IfcLShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = CreateRef<L_profilDef>();
   
    ((L_profilDef*)_profilDef.get())->Depth = (float)value->getDepth();
    ((L_profilDef*)_profilDef.get())->Width = (float)value->getWidth();
    ((L_profilDef*)_profilDef.get())->Thickness = (float)value->getThickness();
    ((L_profilDef*)_profilDef.get())->FilletRadius = (float)value->getFilletRadius();
    ((L_profilDef*)_profilDef.get())->EdgeRadius = (float)value->getEdgeRadius();
    ((L_profilDef*)_profilDef.get())->nbArg = 5;
    if (value->testLegSlope())
    {
        ((L_profilDef*)_profilDef.get())->LegSlope = (float)value->getLegSlope();
        ((L_profilDef*)_profilDef.get())->nbArg = 6;
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

    _profilDef = CreateRef<T_profilDef>();
    
    ((T_profilDef*)_profilDef.get())->Depth = (float)value->getDepth();
    ((T_profilDef*)_profilDef.get())->FlangeWidth = (float)value->getFlangeWidth();
    ((T_profilDef*)_profilDef.get())->WebThickness = (float)value->getWebThickness();
    ((T_profilDef*)_profilDef.get())->FlangeThickness = (float)value->getFlangeThickness();
    ((T_profilDef*)_profilDef.get())->FilletRadius = (float)value->getFilletRadius();
    ((T_profilDef*)_profilDef.get())->FlangeEdgeRadius = (float)value->getFlangeEdgeRadius();
    ((T_profilDef*)_profilDef.get())->WebEdgeRadius = (float)value->getWebEdgeRadius();
    ((T_profilDef*)_profilDef.get())->nbArg = 7;
    if (value->testWebSlope() && value->testFlangeSlope())
    {
        ((T_profilDef*)_profilDef.get())->WebSlope = (float)value->getWebSlope();
        ((T_profilDef*)_profilDef.get())->FlangeSlope = (float)value->getFlangeSlope();
        ((T_profilDef*)_profilDef.get())->nbArg = 9;
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

    _profilDef = CreateRef<U_profilDef>();
    
    ((U_profilDef*)_profilDef.get())->Depth = (float)value->getDepth();
    ((U_profilDef*)_profilDef.get())->FlangeWidth = (float)value->getFlangeWidth();
    ((U_profilDef*)_profilDef.get())->WebThickness = (float)value->getWebThickness();
    ((U_profilDef*)_profilDef.get())->FlangeThickness = (float)value->getFlangeThickness();
    ((U_profilDef*)_profilDef.get())->FilletRadius = (float)value->getFilletRadius();
    ((U_profilDef*)_profilDef.get())->nbArg = 5;
    if (value->testEdgeRadius() && value->testFlangeSlope())
    {
        ((U_profilDef*)_profilDef.get())->EdgeRadius = (float)value->getEdgeRadius();
        ((U_profilDef*)_profilDef.get())->FlangeSlope = (float)value->getFlangeSlope();
        ((U_profilDef*)_profilDef.get())->nbArg = 7;
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

    _profilDef = CreateRef<C_profilDef>();
    
    ((C_profilDef*)_profilDef.get())->Depth = (float)value->getDepth();
    ((C_profilDef*)_profilDef.get())->Width = (float)value->getWidth();
    ((C_profilDef*)_profilDef.get())->WallThickness = (float)value->getWallThickness();
    ((C_profilDef*)_profilDef.get())->Girth = (float)value->getGirth();
    ((C_profilDef*)_profilDef.get())->InternalFilletRadius = (float)value->getInternalFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcZShapeProfileDef(
    ifc2x3::IfcZShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = CreateRef<Z_profilDef>();
    
    ((Z_profilDef*)_profilDef.get())->Depth = (float)value->getDepth();
    ((Z_profilDef*)_profilDef.get())->FlangeWidth = (float)value->getFlangeWidth();
    ((Z_profilDef*)_profilDef.get())->WebThickness = (float)value->getWebThickness();
    ((Z_profilDef*)_profilDef.get())->FlangeThickness = (float)value->getFlangeThickness();
    ((Z_profilDef*)_profilDef.get())->FilletRadius = (float)value->getFilletRadius();
    ((Z_profilDef*)_profilDef.get())->EdgeRadius = (float)value->getEdgeRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcAsymmetricIShapeProfileDef(
    ifc2x3::IfcAsymmetricIShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = CreateRef<AsymmetricI_profilDef>();
    
    ((AsymmetricI_profilDef*)_profilDef.get())->OverallWidth = (float)value->getOverallWidth();
    ((AsymmetricI_profilDef*)_profilDef.get())->OverallDepth = (float)value->getOverallDepth();
    ((AsymmetricI_profilDef*)_profilDef.get())->WebThickness = (float)value->getWebThickness();
    ((AsymmetricI_profilDef*)_profilDef.get())->FlangeThickness = (float)value->getFlangeThickness();
    ((AsymmetricI_profilDef*)_profilDef.get())->FlangeFilletRadius = (float)value->getTopFlangeFilletRadius();
    ((AsymmetricI_profilDef*)_profilDef.get())->TopFlangeWidth = (float)value->getTopFlangeWidth();
    ((AsymmetricI_profilDef*)_profilDef.get())->TopFlangeThickness = (float)value->getTopFlangeThickness();
    ((AsymmetricI_profilDef*)_profilDef.get())->TopFlangeFilletRadius = (float)value->getTopFlangeFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircleHollowProfileDef(
    ifc2x3::IfcCircleHollowProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = CreateRef<CircleHollow_profilDef>();
    
    ((CircleHollow_profilDef*)_profilDef.get())->Radius = (float)value->getRadius();
    ((CircleHollow_profilDef*)_profilDef.get())->WallThickness = (float)value->getWallThickness();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleHollowProfileDef(
    ifc2x3::IfcRectangleHollowProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = CreateRef<RectangleHollow_profilDef>();
    
    ((RectangleHollow_profilDef*)_profilDef.get())->XDim = (float)value->getXDim();
    ((RectangleHollow_profilDef*)_profilDef.get())->YDim = (float)value->getYDim();
    ((RectangleHollow_profilDef*)_profilDef.get())->WallThickness = (float)value->getWallThickness();
    ((RectangleHollow_profilDef*)_profilDef.get())->InnerFilletRadius = (float)value->getInnerFilletRadius();
    ((RectangleHollow_profilDef*)_profilDef.get())->OuteerFilletRadius = (float)value->getOuterFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleProfileDef(
    ifc2x3::IfcRectangleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    if (_exploringRelVoid)
    {
        ((ObjectVoid*)_object.get())->XDim = (float)value->getXDim();
        ((ObjectVoid*)_object.get())->YDim = (float)value->getYDim();
    }
    else
    {
        _profilDef = CreateRef<Rectangle_profilDef>();

        ((Rectangle_profilDef*)_profilDef.get())->XDim = (float)value->getXDim();
        ((Rectangle_profilDef*)_profilDef.get())->YDim = (float)value->getYDim();
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircleProfileDef(
    ifc2x3::IfcCircleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    if (_exploringRelVoid)
        ((ObjectVoid*)_object.get())->Radius = (float)value->getRadius();
    else
    {
        _profilDef = CreateRef<Circle_profilDef>();

        ((Circle_profilDef*)_profilDef.get())->Radius = (float)value->getRadius();
    }

    return true;
}
#pragma endregion

bool CreateConstructionPointVisitor::visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value)
{
    if(value->testOuterCurve())
    {
        return value->getOuterCurve()->acceptVisitor(this);
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcPolyline(ifc2x3::IfcPolyline* value)
{
    int size = 0;

    if (!_exploringRelVoid)
        size = ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.size();
    else
        size = ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.size();

    for (auto point : value->getPoints())
    {
        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(ComputePlacementVisitor::getPoint(point.get()));
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(ComputePlacementVisitor::getPoint(point.get()));
    }

    if (!_exploringRelVoid)
        ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.pop_back();
    else
        ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.pop_back();

    if (!_compositeCurve)
    {
        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Args.push_back(((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.size() - size);
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Args.push_back(((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.size() - size);

        _nbArgToCompute++;
    }
    else
    {
        if (!_exploringRelVoid)
        {
            int elementIndex = ((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1;

            if (((ObjectToConstruct*)_object.get())->ElementsToConstruct[elementIndex].Args.size() == 0)
                ((ObjectToConstruct*)_object.get())->ElementsToConstruct[elementIndex].Args.push_back(((ObjectToConstruct*)_object.get())->ElementsToConstruct[elementIndex].Points.size());
            else
            {
                int argsIndex = ((ObjectToConstruct*)_object.get())->ElementsToConstruct[elementIndex].Args.size() - 1;

                ((ObjectToConstruct*)_object.get())->ElementsToConstruct[elementIndex].Args[argsIndex] = ((ObjectToConstruct*)_object.get())->ElementsToConstruct[elementIndex].Points.size();
            }
        }
        else
        {
            int elementIndex = ((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1;

            if (((ObjectVoid*)_object.get())->ElementsToConstruct[elementIndex].Args.size() == 0)
                ((ObjectVoid*)_object.get())->ElementsToConstruct[elementIndex].Args.push_back(((ObjectVoid*)_object.get())->ElementsToConstruct[elementIndex].Points.size());
            else
            {
                int argsIndex = ((ObjectVoid*)_object.get())->ElementsToConstruct[elementIndex].Args.size() - 1;

                ((ObjectVoid*)_object.get())->ElementsToConstruct[elementIndex].Args[argsIndex] = ((ObjectVoid*)_object.get())->ElementsToConstruct[elementIndex].Points.size();
            }
        }
    }

    if (!_exploringRelVoid)
        return ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.empty() == false;
    else
        return ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.empty() == false;
}

bool CreateConstructionPointVisitor::visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value)
{
    if (value->testOuter())
    {
        ElementToConstruct el;
        el.Key = value->getKey();
        el.Type = value->type();

        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct.push_back(el);
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct.push_back(el);

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

        if (!_exploringRelVoid)
            ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(v);
        else
            ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(v);
    }

    if (!_exploringRelVoid)
        return ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.empty() == false;
    else
        return ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.empty() == false;
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

void CreateConstructionPointVisitor::transformPoints(const Matrix4& transform)
{
    std::list<Vec3> tmpPoints;

    if (!_exploringRelVoid)
        tmpPoints = ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points;
    else
        tmpPoints = ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points;

    int size = -1;

    if (!_exploringRelVoid)
    {
        if (((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Args.size() > 2)
        {
            size = 0;
            for (int i = 0; i < ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Args.size() - _nbArgToCompute; i++)
            {
                size += ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Args[i];
            }
        }
    }
    else
    {
        if (((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Args.size() > 2)
        {
            size = 0;
            for (int i = 0; i < ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Args.size() - _nbArgToCompute; i++)
            {
                size += ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Args[i];
            }
        }
    }

    if (!_exploringRelVoid)
        ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.clear();
    else
        ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.clear();

    int count = 1;
    for (const auto& point : tmpPoints)
    {
        if (count > size)
        {
            if (!_exploringRelVoid)
                ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(transform * point);
            else
                ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(transform * point);
        }
        else
        {
            if (!_exploringRelVoid)
                ((ObjectToConstruct*)_object.get())->ElementsToConstruct[((ObjectToConstruct*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(point);
            else
                ((ObjectVoid*)_object.get())->ElementsToConstruct[((ObjectVoid*)_object.get())->ElementsToConstruct.size() - 1].Points.push_back(point);
        }

        count++;
    }

    _nbArgToCompute = 0;
}

std::vector<std::string> CreateConstructionPointVisitor::getNameItems() const
{
    return std::vector<std::string>();
}

int CreateConstructionPointVisitor::getkeyForVoid() const
{
    return keyForVoid;
}

TrimmedCurve CreateConstructionPointVisitor::getTrimmedCurve() const
{
    return TrimmedCurve();
}

CompositeCurveSegment CreateConstructionPointVisitor::getCompositeCurveSegment() const
{
    return CompositeCurveSegment();
}
