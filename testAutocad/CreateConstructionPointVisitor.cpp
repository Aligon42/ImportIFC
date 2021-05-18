#include "CreateConstructionPointVisitor.h"
#include "ComputePlacementVisitor.h"

CreateConstructionPointVisitor::CreateConstructionPointVisitor() :
    ifc2x3::InheritVisitor()
{

}

CreateConstructionPointVisitor::~CreateConstructionPointVisitor()
{
    delete _profilDef;
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

bool CreateConstructionPointVisitor::visitIfcCompositeCurve(
    ifc2x3::IfcCompositeCurve* value) 
{
    if (value->testSelfIntersect())
    {
        AgreementCompositeCurve.push_back(value->getSelfIntersect());
    }

    if (value->testSegments())
    {
        for (auto segment : value->getSegments())
        {
            if (segment->acceptVisitor(this))
            {
                return true;
            }
        }
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
        return true;
    }

    return false;
}

bool CreateConstructionPointVisitor::visitIfcTrimmedCurve(
    ifc2x3::IfcTrimmedCurve* value)
{
    if (value->testTrim1())
    {
       
    }
    if (value->testTrim2())
    {

    }
    if (value->testSenseAgreement())
    {
        senseAgreementTrimmedCurve.push_back(value->getSenseAgreement());
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
            transformation = ComputePlacementVisitor::getTransformation(
                                         value->getPosition());

            transformPoints(transformation);

            _nameProfilDef = value->getSweptArea()->getType().getName();
            const char* test = value->getSweptArea()->getType().getName().c_str();

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

//***** PROFILDEF *****

bool CreateConstructionPointVisitor::visitIfcIShapeProfileDef(
    ifc2x3::IfcIShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new I_profilDef();

    ((I_profilDef*)_profilDef)->OverallWidth = (float) value->getOverallWidth();
    ((I_profilDef*)_profilDef)->OverallDepth = (float)value->getOverallDepth();
    ((I_profilDef*)_profilDef)->webThickness = (float)value->getWebThickness();
    ((I_profilDef*)_profilDef)->flangeThickness = (float)value->getFlangeThickness();
    ((I_profilDef*)_profilDef)->filletRadius = (float)value->getFilletRadius();
    ((I_profilDef*)_profilDef)->nbArg = 5;

    return true;
}

bool CreateConstructionPointVisitor::visitIfcLShapeProfileDef(
    ifc2x3::IfcLShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new L_profilDef();
   
    ((L_profilDef*)_profilDef)->Depth = (float)value->getDepth();
    ((L_profilDef*)_profilDef)->Width = (float)value->getWidth();
    ((L_profilDef*)_profilDef)->Thickness = (float)value->getThickness();
    ((L_profilDef*)_profilDef)->FilletRadius = (float)value->getFilletRadius();
    ((L_profilDef*)_profilDef)->EdgeRadius = (float)value->getEdgeRadius();
    ((L_profilDef*)_profilDef)->nbArg = 5;
    if (value->testLegSlope())
    {
        ((L_profilDef*)_profilDef)->LegSlope = (float)value->getLegSlope();
        ((L_profilDef*)_profilDef)->nbArg = 6;
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

    _profilDef = new T_profilDef();
    
    ((T_profilDef*)_profilDef)->Depth = (float)value->getDepth();
    ((T_profilDef*)_profilDef)->FlangeWidth = (float)value->getFlangeWidth();
    ((T_profilDef*)_profilDef)->WebThickness = (float)value->getWebThickness();
    ((T_profilDef*)_profilDef)->FlangeThickness = (float)value->getFlangeThickness();
    ((T_profilDef*)_profilDef)->FilletRadius = (float)value->getFilletRadius();
    ((T_profilDef*)_profilDef)->FlangeEdgeRadius = (float)value->getFlangeEdgeRadius();
    ((T_profilDef*)_profilDef)->WebEdgeRadius = (float)value->getWebEdgeRadius();
    ((T_profilDef*)_profilDef)->nbArg = 7;
    if (value->testWebSlope() && value->testFlangeSlope())
    {
        ((T_profilDef*)_profilDef)->WebSlope = (float)value->getWebSlope();
        ((T_profilDef*)_profilDef)->FlangeSlope = (float)value->getFlangeSlope();
        ((T_profilDef*)_profilDef)->nbArg = 9;
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

    _profilDef = new U_profilDef();
    
    ((U_profilDef*)_profilDef)->Depth = (float)value->getDepth();
    ((U_profilDef*)_profilDef)->FlangeWidth = (float)value->getFlangeWidth();
    ((U_profilDef*)_profilDef)->WebThickness = (float)value->getWebThickness();
    ((U_profilDef*)_profilDef)->FlangeThickness = (float)value->getFlangeThickness();
    ((U_profilDef*)_profilDef)->FilletRadius = (float)value->getFilletRadius();
    ((U_profilDef*)_profilDef)->nbArg = 5;
    if (value->testEdgeRadius() && value->testFlangeSlope())
    {
        ((U_profilDef*)_profilDef)->EdgeRadius = (float)value->getEdgeRadius();
        ((U_profilDef*)_profilDef)->FlangeSlope = (float)value->getFlangeSlope();
        ((U_profilDef*)_profilDef)->nbArg = 7;
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

    _profilDef = new C_profilDef();
    
    ((C_profilDef*)_profilDef)->Depth = (float)value->getDepth();
    ((C_profilDef*)_profilDef)->Width = (float)value->getWidth();
    ((C_profilDef*)_profilDef)->WallThickness = (float)value->getWallThickness();
    ((C_profilDef*)_profilDef)->Girth = (float)value->getGirth();
    ((C_profilDef*)_profilDef)->InternalFilletRadius = (float)value->getInternalFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcZShapeProfileDef(
    ifc2x3::IfcZShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new Z_profilDef();
    
    ((Z_profilDef*)_profilDef)->Depth = (float)value->getDepth();
    ((Z_profilDef*)_profilDef)->FlangeWidth = (float)value->getFlangeWidth();
    ((Z_profilDef*)_profilDef)->WebThickness = (float)value->getWebThickness();
    ((Z_profilDef*)_profilDef)->FlangeThickness = (float)value->getFlangeThickness();
    ((Z_profilDef*)_profilDef)->FilletRadius = (float)value->getFilletRadius();
    ((Z_profilDef*)_profilDef)->EdgeRadius = (float)value->getEdgeRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcAsymmetricIShapeProfileDef(
    ifc2x3::IfcAsymmetricIShapeProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new AsymmetricI_profilDef();
    
    ((AsymmetricI_profilDef*)_profilDef)->OverallWidth = (float)value->getOverallWidth();
    ((AsymmetricI_profilDef*)_profilDef)->OverallDepth = (float)value->getOverallDepth();
    ((AsymmetricI_profilDef*)_profilDef)->WebThickness = (float)value->getWebThickness();
    ((AsymmetricI_profilDef*)_profilDef)->FlangeThickness = (float)value->getFlangeThickness();
    ((AsymmetricI_profilDef*)_profilDef)->FlangeFilletRadius = (float)value->getTopFlangeFilletRadius();
    ((AsymmetricI_profilDef*)_profilDef)->TopFlangeWidth = (float)value->getTopFlangeWidth();
    ((AsymmetricI_profilDef*)_profilDef)->TopFlangeThickness = (float)value->getTopFlangeThickness();
    ((AsymmetricI_profilDef*)_profilDef)->TopFlangeFilletRadius = (float)value->getTopFlangeFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircleHollowProfileDef(
    ifc2x3::IfcCircleHollowProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new CircleHollow_profilDef();
    
    ((CircleHollow_profilDef*)_profilDef)->Radius = (float)value->getRadius();
    ((CircleHollow_profilDef*)_profilDef)->WallThickness = (float)value->getWallThickness();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleHollowProfileDef(
    ifc2x3::IfcRectangleHollowProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new RectangleHollow_profilDef();
    
    ((RectangleHollow_profilDef*)_profilDef)->XDim = (float)value->getXDim();
    ((RectangleHollow_profilDef*)_profilDef)->YDim = (float)value->getYDim();
    ((RectangleHollow_profilDef*)_profilDef)->WallThickness = (float)value->getWallThickness();
    ((RectangleHollow_profilDef*)_profilDef)->InnerFilletRadius = (float)value->getInnerFilletRadius();
    ((RectangleHollow_profilDef*)_profilDef)->OuteerFilletRadius = (float)value->getOuterFilletRadius();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcRectangleProfileDef(
    ifc2x3::IfcRectangleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new Rectangle_profilDef();
    
    ((Rectangle_profilDef*)_profilDef)->XDim = (float)value->getXDim();
    ((Rectangle_profilDef*)_profilDef)->YDim = (float)value->getYDim();

    return true;
}

bool CreateConstructionPointVisitor::visitIfcCircleProfileDef(
    ifc2x3::IfcCircleProfileDef* value)
{
    if (value->testPosition())
    {
        value->getPosition()->acceptVisitor(this);
    }

    _profilDef = new Circle_profilDef();
    
    ((Circle_profilDef*)_profilDef)->Radius = (float)value->getRadius();

    return true;
}




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

Matrix4 CreateConstructionPointVisitor::getTransformation() const
{
    return transformation;
}


//***** BOOLEAN *****

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

std::string CreateConstructionPointVisitor::getNameProfildef() const
{
    return _nameProfilDef;
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

