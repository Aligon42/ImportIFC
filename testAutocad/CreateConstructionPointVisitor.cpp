#include "CreateConstructionPointVisitor.h"
#include "ComputePlacementVisitor.h"

CreateConstructionPointVisitor::CreateConstructionPointVisitor() :
    ifc2x3::InheritVisitor()
{

}

bool CreateConstructionPointVisitor::visitIfcProduct(
    ifc2x3::IfcProduct* value)
{
    if(value->testRepresentation())
    {
        return value->getRepresentation()->acceptVisitor(this);
    }

    return true;
}

//bool CreateConstructionPointVisitor::visitIfcSpatialStructureElement(
//    ifc2x3::IfcSpatialStructureElement* value)
//{
//    if (value->)
//    {
//
//    }
//}

bool CreateConstructionPointVisitor::visitIfcSite(
    ifc2x3::IfcSite* value)
{
    if (value->testRepresentation())
    {
        value->getRepresentation()->acceptVisitor(this);
    }

    return true; 
}

bool CreateConstructionPointVisitor::visitIfcRelVoidsElement(
    ifc2x3::IfcRelVoidsElement* value)
{
    if (value->testRelatedOpeningElement())
    {
        value->getRelatedOpeningElement()->acceptVisitor(this);
    }
    if (value->testRelatingBuildingElement())
    {
        keyForVoid = value->getRelatingBuildingElement()->getKey();
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcProductRepresentation(
    ifc2x3::IfcProductRepresentation* value)
{
    for(auto representation : value->getRepresentations())
    {
        if(!(representation->acceptVisitor(this)))
        {
            return false;
        }
    }

    return true;
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
        keyItems.push_back(item->getKey());
        
        if (!isMappedItem)
        {
            nameItems.push_back(item->getType().getName());
        }
        else
        {
            nameItemsMap.push_back(item->getType().getName());
            keyShapeMap.push_back(item->getKey());
            isMappedItem = false;
        }
        
        if(!item->acceptVisitor(this))
        {
            return false;
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcBooleanClippingResult(
    ifc2x3::IfcBooleanClippingResult* value)
{
    bool r1 = false, r2 = false;
    isBoolean = true;

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

                //transformPoints(transformation);
            }

            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcFaceBasedSurfaceModel(
    ifc2x3::IfcFaceBasedSurfaceModel* value)
{
    if (value->testFbsmFaces())
    {
        for (auto face : value->getFbsmFaces())
        {
            if (!(face->acceptVisitor(this)))
            {
                return false;
            }
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcConnectedFaceSet(
    ifc2x3::IfcConnectedFaceSet* value)
{
    if (value->testCfsFaces())
    {
        for (auto face : value->getCfsFaces())
        {
            if (!(face->acceptVisitor(this)))
            {
                return false;
            }
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcShellBasedSurfaceModel(
    ifc2x3::IfcShellBasedSurfaceModel* value)
{
    if (value->testSbsmBoundary())
    {
        for (auto face : value->getSbsmBoundary())
        {
            if (!(face->acceptVisitor(this)))
            {
                return false;
            }
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcShell(
    ifc2x3::IfcShell* value)
{
    switch (value->currentType())
    {
    case value->IFCCLOSEDSHELL:
        value->getIfcClosedShell()->acceptVisitor(this);
        break;
    case value->IFCOPENSHELL:
        value->getIfcOpenShell()->acceptVisitor(this);
        break;
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcOpenShell(
    ifc2x3::IfcOpenShell* value)
{
    if (value->testCfsFaces())
    {
        for (auto face : value->getCfsFaces())
        {
            if (!(face->acceptVisitor(this)))
            {
                return false;
            }
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcMappedItem(
    ifc2x3::IfcMappedItem* value)
{
    isMappedItem = true;
    isMappedItemMethode = true;

    keyMappedItem.push_back(value->getKey());

    if (value->testMappingSource())
    {
        if (value->getMappingSource()->acceptVisitor(this))
        {
            if (value->testMappingTarget())
            {
                value->getMappingTarget()->acceptVisitor(this);
            }

            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcCartesianTransformationOperator3D(
    ifc2x3::IfcCartesianTransformationOperator3D* value)
{
    Vec3 Axis1(1.0f, 0.0f, 0.0f);

    if (value->testAxis1())
    {
       Axis1 = SwitchIfcDirectionToVecteur3D(value->getAxis1(), Axis1);
    }

    Vec3 Axis2(0.0f, 1.0f, 0.0f);

    if (value->testAxis2())
    {
        Axis2 = SwitchIfcDirectionToVecteur3D(value->getAxis2(), Axis2);
    }

    Vec3 Axis3(0.0f, 0.0f, 1.0f);

    if (value->testAxis3())
    {
        Axis3 = SwitchIfcDirectionToVecteur3D(value->getAxis3(), Axis3);
    }

    Vec3 localOrigine(0.0f, 0.0f, 0.0f);

    if (value->testLocalOrigin())
    {
        localOrigine = SwitchIfcCartesianPointToVecteur3D(value->getLocalOrigin(), localOrigine);
    }

    Matrix4 operator3D(
        Axis3.x(), Axis1.x(), Axis2.x(), localOrigine.x(),
        Axis3.y(), Axis1.y(), Axis2.y(), localOrigine.y(),
        Axis3.z(), Axis1.z(), Axis2.z(), localOrigine.z(),
        0.0f, 0.0f, 0.0f, 1.0f
    );

    scale = value->getScale();

    transformationOperator3D = operator3D;

    determinantMatrixOperator3D = (Axis1.x()*Axis2.y()*Axis3.z()) - (Axis2.x()*Axis1.y()*Axis3.z()) + (Axis3.x()*Axis1.y()*Axis2.z()) - localOrigine.x();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcStyledItem(
    ifc2x3::IfcStyledItem* value)
{
    if (value->testItem())
    {
        style.keyItem = value->getItem()->getKey();
    }
    if (value->testStyles())
    {
        for (auto styles : value->getStyles())
        {
            styles->acceptVisitor(this);
        }
    }
    return true;
}

bool CreateConstructionPointVisitor::visitIfcPresentationStyleAssignment(
    ifc2x3::IfcPresentationStyleAssignment* value)
{
    if (value->testStyles())
    {
        for (auto styles : value->getStyles())
        {
            switch (styles->currentType())
            {
            case ifc2x3::IfcPresentationStyleSelect::IFCCURVESTYLE :
                styles->getIfcCurveStyle()->acceptVisitor(this);
                break;

            case ifc2x3::IfcPresentationStyleSelect::IFCFILLAREASTYLE:
                styles->getIfcFillAreaStyle()->acceptVisitor(this);
                break;

            case ifc2x3::IfcPresentationStyleSelect::IFCTEXTSTYLE:
                styles->getIfcTextStyle()->acceptVisitor(this);
                break;

            case ifc2x3::IfcPresentationStyleSelect::IFCSURFACESTYLE:
                styles->getIfcSurfaceStyle()->acceptVisitor(this);
                break;
            }
            
        }
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcSurfaceStyle(
    ifc2x3::IfcSurfaceStyle* value)
{
    if (value->testSide())
    {
        value->getSide();
    }
    if (value->testStyles())
    {
        for (auto styles : value->getStyles())
        {
            
            switch (styles->currentType())
            {
            case ifc2x3::IfcSurfaceStyleElementSelect::IFCSURFACESTYLELIGHTING:
                styles->getIfcSurfaceStyleLighting()->acceptVisitor(this);
                break;

            case ifc2x3::IfcSurfaceStyleElementSelect::IFCSURFACESTYLESHADING:
                styles->getIfcSurfaceStyleShading()->acceptVisitor(this);
                break;

            case ifc2x3::IfcSurfaceStyleElementSelect::IFCSURFACESTYLEWITHTEXTURES:
                styles->getIfcSurfaceStyleWithTextures()->acceptVisitor(this);
                break;

            case ifc2x3::IfcSurfaceStyleElementSelect::IFCEXTERNALLYDEFINEDSURFACESTYLE:
                styles->getIfcExternallyDefinedSurfaceStyle()->acceptVisitor(this);
                break;

            case ifc2x3::IfcSurfaceStyleElementSelect::IFCSURFACESTYLEREFRACTION:
                styles->getIfcSurfaceStyleRefraction()->acceptVisitor(this);
                break;
            }
        }
    }
    return true;
}

bool CreateConstructionPointVisitor::visitIfcSurfaceStyleRendering(
    ifc2x3::IfcSurfaceStyleRendering* value)
{
    if (value->testSurfaceColour())
    {
        value->getSurfaceColour()->acceptVisitor(this);
    }

    if (value->testTransparency())
    {
        style.transparence = value->getTransparency();
    }
    return true;
}

bool CreateConstructionPointVisitor::visitIfcColourRgb(
    ifc2x3::IfcColourRgb* value)
{
    if (value->testRed())
    {
        style.red = value->getRed();
    }
    if (value->testGreen())
    {
        style.green = value->getGreen();
    }
    if (value->testBlue())
    {
        style.blue = value->getBlue();
    }
    return true;
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
                //transformPoints(transform);

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

            entityHalf.push_back(value->getClassType().getName());

            
            return true;
        }
    }
    return false;
}

bool CreateConstructionPointVisitor::visitIfcCompositeCurve(
    ifc2x3::IfcCompositeCurve* value) 
{
    isCompositeCurve = true;
    nbCompositeCurve = 0;
    if (value->testSelfIntersect())
    {
        AgreementCompositeCurve.push_back(value->getSelfIntersect());
    }

    if (value->testSegments())
    {
        for (auto segment : value->getSegments())
        {
            if (!segment->acceptVisitor(this))
            {
                return false;
            }
        }
        
        int a = 0;
        for (int j = 0; j < listNbArgPolyline.size(); j++)
        {
            std::vector<Vec3> points;
            int nb = 0;

            auto point = _points.begin();

            for (int i = 0; i < a; i++)
                point++;

            for (size_t i = 0; i < listNbArgPolyline[j]; i++)
            {
                    points.push_back(*point++);
                    a++;               
            }

            if (isBoolean && j < 2)
            { }
            else
            {
                _compositeCurveSegment.listPolyligne.push_back(points);
                nbPolylineCompositeCurve++;
            }
            

            /*
            for (int x = 0; x < nb; x++)
            {
                _points.erase(_points.begin());
            }*/
        }
    }
    nbCompositeCurve++;
    return true;
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
        _trimmedCurve.trim1 = value->getTrim1().begin()->get()->getIfcParameterValue();
    }
    if (value->testTrim2())
    {
        _trimmedCurve.trim2 = value->getTrim2().begin()->get()->getIfcParameterValue();
    }
    if (value->testSenseAgreement())
    {
        _trimmedCurve.senseArgreement = value->getSenseAgreement() - 1;
    }
    if (value->testMasterRepresentation())
    {

    }
    if (value->testBasisCurve())
    {
        value->getBasisCurve()->acceptVisitor(this);
    }

    _compositeCurveSegment.listTrimmedCurve.push_back(_trimmedCurve);


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

bool CreateConstructionPointVisitor::visitIfcAxis2Placement(
    ifc2x3::IfcAxis2Placement* value)
{
    switch (value->currentType())
    {
        case value->IFCAXIS2PLACEMENT2D:
            value->getIfcAxis2Placement2D()->acceptVisitor(this);
            break;
        case value->IFCAXIS2PLACEMENT3D:
            value->getIfcAxis2Placement3D()->acceptVisitor(this);
            break;
    }

    return true;
}

bool CreateConstructionPointVisitor::visitIfcAxis2Placement2D(
    ifc2x3::IfcAxis2Placement2D* value)
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
            transformation = ComputePlacementVisitor::getTransformation(
                                         value->getPosition());

            //transformPoints(transformation);

            NameProfilDef = value->getSweptArea()->getType().getName();

            if(value->testExtrudedDirection())
            {
                extrusionVector = ComputePlacementVisitor::getDirection(
                                      value->getExtrudedDirection());
            }
            if (value->testDepth())
            {
                hauteurExtrusion = value->getDepth();
            }

            return true;
        }
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcBoundingBox(
    ifc2x3::IfcBoundingBox* value)
{
    if (value->testXDim())
    {
        box.XDimBox = value->getXDim();
    }
    if (value->testYDim())
    {
        box.YDimBox = value->getYDim();
    }
    if (value->testZDim())
    {
        box.ZDimBox = value->getZDim();
    }
    if (value->testCorner())
    {
        auto corner = value->getCorner();
        SwitchIfcCartesianPointToVecteur3D(corner, box.Corner);
    }
    
    return true;
}

//***** PROFILDEF *****

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

    
    CircleHollowprofilDef.Radius = (float)value->getRadius();
    CircleHollowprofilDef.WallThickness = (float)value->getWallThickness();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleHollowProfileDef(
    ifc2x3::IfcRectangleHollowProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    
    RectangleHollowprofilDef.XDim = (float)value->getXDim();
    RectangleHollowprofilDef.YDim = (float)value->getYDim();
    RectangleHollowprofilDef.WallThickness = (float)value->getWallThickness();
    RectangleHollowprofilDef.InnerFilletRadius = (float)value->getInnerFilletRadius();
    RectangleHollowprofilDef.OuteerFilletRadius = (float)value->getOuterFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleProfileDef(
    ifc2x3::IfcRectangleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    
    RectangleprofilDef.XDim = (float)value->getXDim();
    RectangleprofilDef.YDim = (float)value->getYDim();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircleProfileDef(
    ifc2x3::IfcCircleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    
    CircleprofilDef.Radius = (float)value->getRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value)
{
    if(value->testOuterCurve())
    {
        outerCurveName = value->getOuterCurve()->getType().getName();
        return value->getOuterCurve()->acceptVisitor(this);
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcPolyline(ifc2x3::IfcPolyline* value)
{
    int size = _points.size();
    int nb = nbPolylineCompositeCurve;
    

    for(auto point : value->getPoints())
    {
        _points.push_back(ComputePlacementVisitor::getPoint(point.get()));
    }

    if (nbSupport != 0)
    {
        if (nbSupport == nb)
        {
            isCompositeCurve = false;
        }
    }
    
    if (!isCompositeCurve)
    {
        _points.pop_back();
    }
    else if (isBoolean && !isCompositeCurve)
    {
        _points.pop_back();
    }
    else if (isBoolean)
    {
        nbSupport++;
        
    }

    
    

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
        orientationFace = value->getOrientation();
        return value->getBound()->acceptVisitor(this);
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcFaceBound(ifc2x3::IfcFaceBound* value)
{
    auto t1 = value->getBound();

    if (value->testBound())
    {
        orientationFace = value->getOrientation();
        return value->getBound()->acceptVisitor(this);
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value)
{
    nbArgFace.push_back(value->getPolygon().size());

    for (auto point : value->getPolygon())
    {
        _points.push_back(ComputePlacementVisitor::getPoint(point.get()));
        
    }

    return _points.empty() == false;
}



Vec3 CreateConstructionPointVisitor::SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine)
{
    auto listPoint = value->getCoordinates();

    outOrigine.x() = (float) listPoint.at(0);
    outOrigine.y() = (float) listPoint.at(1);
    outOrigine.z() = (float) listPoint.at(2);

    return outOrigine;
}

Vec3 CreateConstructionPointVisitor::SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur)
{
    auto listPoint = value->getDirectionRatios();

    outVecteur.x() = (float) listPoint.at(0);
    outVecteur.y() = (float) listPoint.at(1);
    outVecteur.z() = (float) listPoint.at(2);

    return outVecteur;
}

std::list<Vec3> CreateConstructionPointVisitor::getPoints() const
{
    return _points;
}

Vec3 CreateConstructionPointVisitor::getVectorDirection() const
{
    return extrusionVector;
}

float CreateConstructionPointVisitor::getHauteurExtrusion() const
{
    return hauteurExtrusion;
}

Matrix4 CreateConstructionPointVisitor::getTransformation() const
{
    return transformation;
}

Matrix4 CreateConstructionPointVisitor::getTransformationOperator3D() const
{
    return transformationOperator3D;
}

float CreateConstructionPointVisitor::getDetermiantOperator3D() const
{
    return determinantMatrixOperator3D;
}

bool CreateConstructionPointVisitor::getIsMappedItem() const
{
    return isMappedItem;
}

bool CreateConstructionPointVisitor::getIsMappedItemMethode() const
{
    return isMappedItemMethode;
}

double CreateConstructionPointVisitor::getScale() const
{
    return scale;
}

std::vector<std::string> CreateConstructionPointVisitor::getNameItems() const
{
    return nameItems;
}

std::string CreateConstructionPointVisitor::getOuterCurveName() const
{
    return outerCurveName;
}

//***** BOOLEAN *****

std::vector<Step::Boolean> CreateConstructionPointVisitor::getAgreementHalfBool() const
{
    return AgreementHalf;
}

std::vector<Step::Boolean> CreateConstructionPointVisitor::getAgreementPolygonalBool() const
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


//****** PROFILDEF ******

I_profilDef CreateConstructionPointVisitor::getIprofilDef() const
{
    
    return IprofilDef;
}

L_profilDef CreateConstructionPointVisitor::getLprofilDef() const
{

    return LprofilDef;
}

T_profilDef CreateConstructionPointVisitor::getTprofilDef() const
{

    return TprofilDef;
}

U_profilDef CreateConstructionPointVisitor::getUprofilDef() const
{

    return UprofilDef;
}

C_profilDef CreateConstructionPointVisitor::getCprofilDef() const
{

    return CprofilDef;
}

Z_profilDef CreateConstructionPointVisitor::getZprofilDef() const
{

    return ZprofilDef;
}

AsymmetricI_profilDef CreateConstructionPointVisitor::getAsymmetricIprofilDef() const
{

    return AsymmetricIprofilDef;
}

CircleHollow_profilDef CreateConstructionPointVisitor::getCircleHollowprofilDef() const
{

    return CircleHollowprofilDef;
}

RectangleHollow_profilDef CreateConstructionPointVisitor::getRectangleHollowprofilDef() const
{

    return RectangleHollowprofilDef;
}

Circle_profilDef CreateConstructionPointVisitor::getCircleprofilDef() const
{

    return CircleprofilDef;
}

Rectangle_profilDef CreateConstructionPointVisitor::getRectangleprofilDef() const
{

    return RectangleprofilDef;
}

std::string CreateConstructionPointVisitor::getNameProfildef() const
{
    return NameProfilDef;
}


int CreateConstructionPointVisitor::getkeyForVoid() const
{
    return keyForVoid;
}


int CreateConstructionPointVisitor::getnbPolylineCompositeCurve() const
{
    return nbPolylineCompositeCurve;
}

CompositeCurveSegment CreateConstructionPointVisitor::getCompositeCurveSegment() const
{
    return _compositeCurveSegment;
}

int CreateConstructionPointVisitor::getNbCompositeCurve() const
{
    return nbCompositeCurve;
}

std::vector<int> CreateConstructionPointVisitor::getKeyMappedItem() const
{
    return keyMappedItem;
}

std::vector<std::string> CreateConstructionPointVisitor::getNameItemsMap() const
{
    return nameItemsMap;
}

std::vector<int> CreateConstructionPointVisitor::getkeyShapeMap() const
{
    return keyShapeMap;
}


std::vector<int> CreateConstructionPointVisitor::getListNbArgFace() const
{
    return nbArgFace;
}

std::vector<int> CreateConstructionPointVisitor::getListKeyItem() const
{
    return keyItems;
}

Box CreateConstructionPointVisitor::getBox() const
{
    return box;
}

Style CreateConstructionPointVisitor::getStyle() const
{
    return style;
}

bool CreateConstructionPointVisitor::getOrientatationFace() const
{
    return orientationFace;
}

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


