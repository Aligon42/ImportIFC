// IFC SDK : IFC2X3 C++ Early Classes  
// Copyright (C) 2009 CSTB
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full license is in Licence.txt file included with this 
// distribution or is available at :
//     http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

#include "CreateGeometricRepresentationVisitor.h"
#include "Export.h"

#include <ifc2x3/IfcProduct.h>
#include <ifc2x3/DefinedTypes.h>
#include <ifc2x3/IfcAxis2Placement.h>

CompositeCurve::~CompositeCurve()
{
    CompositeCurveSegments.clear();
    std::vector<std::shared_ptr<CompositeCurveSegmentEx>>().swap(CompositeCurveSegments);
}

CreateGeometricRepresentationVisitor::CreateGeometricRepresentationVisitor(ifc2x3::ExpressDataSet* eds)
{
    mDataSet = eds;
    init();
}

void CreateGeometricRepresentationVisitor::init()
{
    mGeomType = UNDEF_GEOM;
    mLocationType = UNDEF_LOC;
    mPolyloopMustBeClosed = false;
    m2DPolyline.clear();
    m3DPolyline.clear();
    mPosition.clear();
    mPosition.push_back(0.0);
    mPosition.push_back(0.0);
    mPosition.push_back(0.0);
    mLocalPlacement.clear();
    mLocalPlacement.push_back(0.0);
    mLocalPlacement.push_back(0.0);
    mLocalPlacement.push_back(0.0);
    mExtrusionDirection.clear();
    mExtrusionDirection.push_back(0.0);
    mExtrusionDirection.push_back(0.0);
    mExtrusionDirection.push_back(1.0);
    mExtrusionDepth = 3.0;
    mUpdateGeometry = false;

    mItems.clear();
    std::vector<std::shared_ptr<RepresentationItem>>().swap(mItems);
}

bool CreateGeometricRepresentationVisitor::visitIfcBuildingElementPart(ifc2x3::IfcBuildingElementPart* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcBuildingElementPart > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcWallStandardCase(ifc2x3::IfcWallStandardCase* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcWallStandardCase > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcCovering(ifc2x3::IfcCovering* value)
{
    bool result = true;
    isCoveringExp = true;
    Step::RefPtr< ifc2x3::IfcCovering > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcPlate(ifc2x3::IfcPlate* value)
{
    bool result = true;
    isPlateExp = true;
    Step::RefPtr< ifc2x3::IfcPlate > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcSlab(ifc2x3::IfcSlab* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcSlab > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcBeam(ifc2x3::IfcBeam* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcBeam > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcSpace(ifc2x3::IfcSpace* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcSpace > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcOpeningElement(ifc2x3::IfcOpeningElement* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcOpeningElement > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcWindow(ifc2x3::IfcWindow* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcWindow > rpValue = value;

    mGeomType = BODY_SWEPTSOLID;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcSpatialStructureElement(ifc2x3::IfcSpatialStructureElement* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcSpatialStructureElement > rpValue = value;

    mGeomType = FOOTPRINT_CURVE2D;
    result &= visitIfcProduct(value);
    mGeomType = UNDEF_GEOM;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcProduct(ifc2x3::IfcProduct* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcProduct > rpValue = value;

    // Set ObjectPlacement: Placement of the product in space, the placement can either
    // be absolute (relative to the world coordinate system), relative (relative to the
    // object placement of another product), or constraint (e.g. relative to grid axes).
    // It is determined by the various subtypes of IfcObjectPlacement, which includes the
    // axis placement information to determine the transformation for the object coordinate system.
    Step::RefPtr< ifc2x3::IfcLocalPlacement > objetPlacement =
        (ifc2x3::IfcLocalPlacement*)rpValue->getObjectPlacement();

    if (!objetPlacement.valid()) {
        objetPlacement = mDataSet->createIfcLocalPlacement();
        result &= objetPlacement->acceptVisitor(this);
        rpValue->setObjectPlacement(objetPlacement.get());
    }

    // Set Representation: Reference to the representations of the product,
    // being either a representation (IfcProductRepresentation) or as a special
    // case a shape representations (IfcProductDefinitionShape).
    // The product definition shape provides for multiple geometric representations
    // of the shape property of the object within the same object coordinate system,
    // defined by the object placement.
    Step::RefPtr< ifc2x3::IfcProductRepresentation > productRepresentation =
        rpValue->getRepresentation();

    if (!productRepresentation.valid()) {
        productRepresentation = mDataSet->createIfcProductRepresentation();
        rpValue->setRepresentation(productRepresentation.get());
    }

    result &= productRepresentation->acceptVisitor(this);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcProductRepresentation > rpValue = value;

    Step::RefPtr< ifc2x3::IfcShapeRepresentation > shapeRepresentation;
    ifc2x3::List_IfcRepresentation_1_n::iterator it;
    
    /*for (it = rpValue->getRepresentations().begin(); it != rpValue->getRepresentations().end(); it++) 
    {
        switch (mGeomType)
        {
        case FOOTPRINT_CURVE2D:
            if ((*it)->getRepresentationIdentifier() == "FootPrint" && (*it)->getRepresentationType() == "Curve2D") {
                shapeRepresentation = (*it).get();
            }
            break;
        case BODY_SWEPTSOLID:
            if ((*it)->getRepresentationIdentifier() == "Body" && (*it)->getRepresentationType() == "SweptSolid") {
                shapeRepresentation = (*it).get();
            }
            break;
        default:
            return false;
            break;
        }
    }*/

    if (!shapeRepresentation.valid()) 
    {
        
            shapeRepresentation = mDataSet->createIfcShapeRepresentation();
            rpValue->getRepresentations().push_back(shapeRepresentation.get());

            result &= shapeRepresentation->acceptVisitor(this);
    }

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcRepresentation(ifc2x3::IfcRepresentation* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcRepresentation > rpValue = value;

    Step::RefPtr< ifc2x3::IfcRepresentationItem > representationItem;

    for (mCurrentItemIndex = 0; mCurrentItemIndex < mItems.size(); mCurrentItemIndex++)
    {
        switch (mGeomType)
        {
        case FOOTPRINT_CURVE2D:
            rpValue->setRepresentationIdentifier("FootPrint");
            rpValue->setRepresentationType("Curve2D");
            representationItem = (ifc2x3::IfcRepresentationItem*)mDataSet->createIfcPolyline().get();
            mPolyloopMustBeClosed = true;
            break;
        case BODY_SWEPTSOLID:
            rpValue->setRepresentationIdentifier("Body");
            rpValue->setRepresentationType("SweptSolid");
            if (mItems[mCurrentItemIndex]->TypeItem == "facetedBRep")
            {
                representationItem = (ifc2x3::IfcRepresentationItem*)mDataSet->createIfcFacetedBrep().get();
                isPlateExp = false;
                isCoveringExp = false;
            }
            else if (mItems[mCurrentItemIndex]->TypeItem == "compositeCurve")
            {
                mIsCompositeCurve = true;
                representationItem = (ifc2x3::IfcArbitraryClosedProfileDef*)mDataSet->createIfcArbitraryClosedProfileDef().get();
                isCoveringExp = false;
                isPlateExp = false;
            }
            else
            {
                representationItem = (ifc2x3::IfcRepresentationItem*)mDataSet->createIfcExtrudedAreaSolid().get();
            }

            break;
        default:
            return false;
            break;
        }

        result &= representationItem->acceptVisitor(this);
        mPolyloopMustBeClosed = false;
        mIsCompositeCurve = false;
        rpValue->getItems().insert(representationItem.get());
    }

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcFacetedBrep> rpValue = value;

    Step::RefPtr<ifc2x3::IfcClosedShell> closedShell;

    closedShell = mDataSet->createIfcClosedShell();
    rpValue->setOuter(closedShell.get());

    result &= closedShell->acceptVisitor(this);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcClosedShell(ifc2x3::IfcClosedShell* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcClosedShell> rpValue = value;

    Step::RefPtr<ifc2x3::IfcFace> face;
    ifc2x3::Set_IfcFace_1_n face_1_n;

    for (mCurrentFaceIndex = 0; mCurrentFaceIndex < ((FacetedRepresentation*)mItems[mCurrentItemIndex].get())->Faces.size(); mCurrentFaceIndex++)
    {
        face = mDataSet->createIfcFace().get();
        result &= face->acceptVisitor(this);
        face_1_n.emplace(face);
    }

    rpValue->setCfsFaces(face_1_n);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcFace(ifc2x3::IfcFace* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcFace> rpValue = value;

    Step::RefPtr<ifc2x3::IfcFaceOuterBound> faceOuterBound;
    ifc2x3::Set_IfcFaceBound_1_n faceBound_1_n;
    
    for (size_t i = 0; i < 1; i++)
    {
        faceOuterBound = mDataSet->createIfcFaceOuterBound();
        result &= faceOuterBound->acceptVisitor(this);
        faceBound_1_n.emplace(faceOuterBound);
    }

    rpValue->setBounds(faceBound_1_n);

    return result;
}


bool CreateGeometricRepresentationVisitor::visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcFaceOuterBound> rpValue = value;

    Step::RefPtr<ifc2x3::IfcPolyLoop> polyloop;
    Step::Boolean boolean = Step::Boolean::BTrue;

    Step::RefPtr<ifc2x3::IfcCompositeCurve> compositeCurve;
    Step::RefPtr<ifc2x3::IfcEdgeLoop> edgeLoop;

    if (((FacetedRepresentation*)mItems[mCurrentItemIndex].get())->Faces[mCurrentFaceIndex]->TypeFace == "polyline")
    {
        polyloop = mDataSet->createIfcPolyLoop().get();
        rpValue->setBound(polyloop);
        rpValue->setOrientation(boolean);
        result &= polyloop->acceptVisitor(this);
    }
    else if (((FacetedRepresentation*)mItems[mCurrentItemIndex].get())->Faces[mCurrentFaceIndex]->TypeFace == "compositeCurve")
    {
        compositeCurve = mDataSet->createIfcCompositeCurve().get();
        rpValue->setBound(compositeCurve);
        rpValue->setOrientation(boolean);
        result &= compositeCurve->acceptVisitor(this);
    }
    else if (((FacetedRepresentation*)mItems[mCurrentItemIndex].get())->Faces[mCurrentFaceIndex]->TypeFace == "edgeLoop")
    {
        edgeLoop = mDataSet->createIfcEdgeLoop().get();
        rpValue->setBound(edgeLoop);
        rpValue->setOrientation(boolean);
        result &= edgeLoop->acceptVisitor(this);
    }

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcSweptAreaSolid(ifc2x3::IfcSweptAreaSolid* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcSweptAreaSolid > rpValue = value;

    // Set Position: Position coordinate system for the swept area
    Step::RefPtr< ifc2x3::IfcAxis2Placement3D > position =
        mDataSet->createIfcAxis2Placement3D();

    mLocationType = POSITION;
    result &= position->acceptVisitor(this);
    mLocationType = UNDEF_LOC;

    rpValue->setPosition(position.get());

    // Set SweptArea: The surface defining the area to be swept.
    // It is given as a profile definition within the xy plane of the position coordinate system
    Step::RefPtr< ifc2x3::IfcArbitraryClosedProfileDef > arbitraryClosedProfileDef =
        mDataSet->createIfcArbitraryClosedProfileDef();
    result &= arbitraryClosedProfileDef->acceptVisitor(this);
    rpValue->setSweptArea(arbitraryClosedProfileDef.get());

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcExtrudedAreaSolid > rpValue = value;

    // Visit mother class IfcSweptAreaSolid
    result &= visitIfcSweptAreaSolid(value);

    // Set Depth: The distance the surface is to be swept
    rpValue->setDepth(mExtrusionDepth);

    // Set ExtrudedDirection: The direction in which the surface is to be swept
    Step::RefPtr< ifc2x3::IfcDirection > direction =
        mDataSet->createIfcDirection();
    direction->getDirectionRatios().push_back(mExtrusionDirection[0]);
    direction->getDirectionRatios().push_back(mExtrusionDirection[1]);
    direction->getDirectionRatios().push_back(mExtrusionDirection[2]);
    rpValue->setExtrudedDirection(direction);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcLocalPlacement(ifc2x3::IfcLocalPlacement* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcLocalPlacement > rpValue = value;

    // Set RelativePlacement: Geometric placement that defines the transformation from
    // the related coordinate system into the relating. The placement can be either 2D
    // or 3D, depending on the dimension count of the coordinate system.
    Step::RefPtr<ifc2x3::IfcAxis2Placement> placement = new ifc2x3::IfcAxis2Placement;

    Step::RefPtr< ifc2x3::IfcAxis2Placement3D > position =
        mDataSet->createIfcAxis2Placement3D();

    mLocationType = LOCAL_PLACEMENT;
    result &= position->acceptVisitor(this);
    mLocationType = UNDEF_LOC;

    placement->setIfcAxis2Placement3D(position.get());

    rpValue->setRelativePlacement(placement.get());

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcPlacement(ifc2x3::IfcPlacement* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcPlacement > rpValue = value;

    std::vector<double> placement;

    switch (mLocationType)
    {
    case POSITION:
        placement = mPosition;
        break;
    case LOCAL_PLACEMENT:
        placement = mLocalPlacement;
        break;
    default:
        return false;
    }

    Step::RefPtr< ifc2x3::IfcCartesianPoint > l = mDataSet->createIfcCartesianPoint();
    l->getCoordinates().push_back(placement[0]);
    l->getCoordinates().push_back(placement[1]);
    l->getCoordinates().push_back(placement[2]);

    rpValue->setLocation(l.get());

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcArbitraryClosedProfileDef > rpValue = value;
    Step::RefPtr<ifc2x3::IfcCompositeCurve> compositeCurve;

    // Set ProfileType: Defines the type of geometry into which this profile 
    // definition shall be resolved, either a curve or a surface area.
    // In case of curve the profile should be referenced by a swept surface,
    // in case of area the profile should be referenced by a swept area solid.
    rpValue->setProfileType(ifc2x3::IfcProfileTypeEnum_AREA);

    // Set OuterCurve: Bounded curve, defining the outer boundaries of the arbitrary profile
    if (mUpdateGeometry) {
        mPolyline = mDataSet->createIfcPolyline();
        mPolyloopMustBeClosed = true;
        result &= mPolyline->acceptVisitor(this);
        rpValue->setOuterCurve(mPolyline.get());
        mUpdateGeometry = false;
    }
    else if (mIsCompositeCurve)
    {
        compositeCurve = mDataSet->createIfcCompositeCurve();
        result &= compositeCurve->acceptVisitor(this);
        rpValue->setOuterCurve(compositeCurve);
        mIsCompositeCurve = false;
    }
    else {

        rpValue->setOuterCurve(mPolyline.get());
        result &= true;
    }


    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcCompositeCurve(ifc2x3::IfcCompositeCurve* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcCompositeCurve> rpValue = value;

    Step::RefPtr<ifc2x3::IfcCompositeCurveSegment> compositeCurveSegment;
    Step::Logical logical = Step::Logical::LFalse;

    CompositeCurve* compositeCurve = (CompositeCurve*)mItems[mCurrentItemIndex].get();

    //mIsCompositeCurve = true;

    for (mCurrentCompositeCurveIndex = 0; mCurrentCompositeCurveIndex < compositeCurve->CompositeCurveSegments.size(); mCurrentCompositeCurveIndex++)
    {
        compositeCurveSegment = mDataSet->createIfcCompositeCurveSegment();
        result &= compositeCurveSegment->acceptVisitor(this);
        rpValue->getSegments().push_back(compositeCurveSegment);
    }

    rpValue->setSelfIntersect(logical);

    mIsCompositeCurve = false;

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcCompositeCurveSegment(ifc2x3::IfcCompositeCurveSegment* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcCompositeCurveSegment> rpValue = value;
    auto transitionCode = ifc2x3::IfcTransitionCode_CONTINUOUS;
    Step::Boolean sameSense = Step::Boolean::BTrue;

    auto segment = ((CompositeCurve*)mItems[mCurrentItemIndex].get())->CompositeCurveSegments[mCurrentCompositeCurveIndex];

    if (segment->TypeSegment == "polyline")
    {
        Step::RefPtr<ifc2x3::IfcPolyline> polyline = mDataSet->createIfcPolyline();
        result &= polyline->acceptVisitor(this);
        rpValue->setParentCurve(polyline);
    }
    else if (segment->TypeSegment == "trimmedCurve")
    {
        Step::RefPtr<ifc2x3::IfcTrimmedCurve> trimmedCurve = mDataSet->createIfcTrimmedCurve();
        result &= trimmedCurve->acceptVisitor(this);
        rpValue->setParentCurve(trimmedCurve);
    }

    rpValue->setTransition(transitionCode);
    rpValue->setSameSense(sameSense);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcTrimmedCurve(ifc2x3::IfcTrimmedCurve* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcTrimmedCurve> rpValue = value;
    Step::RefPtr<ifc2x3::IfcCircle> circle = mDataSet->createIfcCircle();

    result &= circle->acceptVisitor(this);

    auto* trimmedCurve = (TrimmedCurveEx*)((CompositeCurve*)mItems[mCurrentItemIndex].get())->CompositeCurveSegments[mCurrentCompositeCurveIndex].get();

    Step::RefPtr<ifc2x3::IfcTrimmingSelect> trimming1 = new ifc2x3::IfcTrimmingSelect;
    Step::RefPtr<ifc2x3::IfcTrimmingSelect> trimming2 = new ifc2x3::IfcTrimmingSelect;
    ifc2x3::Set_IfcTrimmingSelect_1_2 trim1, trim2;

    trimming1->setIfcParameterValue(trimmedCurve->Trim1);
    trimming2->setIfcParameterValue(trimmedCurve->Trim2);

    trim1.emplace(trimming1);
    trim2.emplace(trimming2);

    rpValue->setTrim1(trim1);
    rpValue->setTrim2(trim2);

    rpValue->setBasisCurve(circle);

    rpValue->setSenseAgreement(trimmedCurve->SenseAgreement);
    rpValue->setMasterRepresentation(trimmedCurve->Preference);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcPolyline(ifc2x3::IfcPolyline* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcPolyline > rpValue = value;

    if (mIsCompositeCurve)
    {
        CompositeCurveSegmentPolyline* polyline = (CompositeCurveSegmentPolyline*)((CompositeCurve*)mItems[mCurrentItemIndex].get())->CompositeCurveSegments[mCurrentCompositeCurveIndex].get();

        for (auto& point : polyline->Points)
        {
            Step::RefPtr< ifc2x3::IfcCartesianPoint > p = mDataSet->createIfcCartesianPoint();
            p->getCoordinates().push_back(point.X);
            p->getCoordinates().push_back(point.Y);
            p->getCoordinates().push_back(point.Z);

            rpValue->getPoints().push_back(p.get());
        }
    }
    else
    {
        for (unsigned int i = 0; i < m2DPolyline.size() / 2; i++) {
            Step::RefPtr< ifc2x3::IfcCartesianPoint > p = mDataSet->createIfcCartesianPoint();
            p->getCoordinates().push_back(m2DPolyline[2 * i]);
            p->getCoordinates().push_back(m2DPolyline[2 * i + 1]);
            rpValue->getPoints().push_back(p.get());
        }
    }

    // Set Points: The points defining the polyline
    if (mPolyloopMustBeClosed &&
        (*rpValue->getPoints().begin())->getCoordinates() != (*rpValue->getPoints().rbegin())->getCoordinates()) {
        rpValue->getPoints().push_back(*rpValue->getPoints().begin());
    }

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcPolyLoop> rpValue = value;

    PolylineEx* polyline = ((PolylineEx*)((FacetedRepresentation*)mItems[mCurrentItemIndex].get())->Faces[mCurrentFaceIndex].get());

    for (auto& point : polyline->Points)
    {
        Step::RefPtr<ifc2x3::IfcCartesianPoint> p = mDataSet->createIfcCartesianPoint();
        p->getCoordinates().push_back(point.X);
        p->getCoordinates().push_back(point.Y);
        p->getCoordinates().push_back(point.Z);

        rpValue->getPolygon().push_back(p.get());
    }

    if (mPolyloopMustBeClosed &&
        (*rpValue->getPolygon().begin())->getCoordinates() != (*rpValue->getPolygon().rbegin())->getCoordinates()) {
        rpValue->getPolygon().push_back(*rpValue->getPolygon().begin());
    }

    return result;
}

//bool CreateGeometricRepresentationVisitor::visitIfcEdgeLoop(ifc2x3::IfcEdgeLoop* value)
//{
//    bool result = true;
//    Step::RefPtr<ifc2x3::IfcEdgeLoop> rpValue = value;
//
//    Step::RefPtr<ifc2x3::IfcEdge> edge;
//    Step::RefPtr<ifc2x3::IfcEdgeCurve> edgeCurve;
//    std::vector<Step::RefPtr<ifc2x3::IfcOrientedEdge>>::const_iterator orientedEdgeIT;
//    ifc2x3::List_IfcOrientedEdge_1_n orientedEdge_1_n;
//
//    auto edges = ((EdgeLoop*)mFaces[mCurrentFaceIndex].get())->EdgeList;
//
//    for (mCurrentEdge = 0; mCurrentEdge < edges.size(); mCurrentEdge++)
//    {
//        std::string type = edges[mCurrentEdge]->TypeEdge;
//        if (type == "edgeCurve")
//        {
//            mIsEdgeCurve = true;
//            Step::RefPtr<ifc2x3::IfcEdgeCurve> edgeCurve;
//            edgeCurve = mDataSet->createIfcEdgeCurve();
//            result &= edgeCurve->acceptVisitor(this);
//            orientedEdge_1_n.push_back(edgeCurve);
//        }
//        else if (type == "edge")
//        {
//            edge = mDataSet->createIfcEdge();
//            result &= edge->acceptVisitor(this);
//            orientedEdge_1_n.push_back(edge);
//        }
//
//        mIsEdgeCurve = false;
//    }
//
//    rpValue->setEdgeList(orientedEdge_1_n);
//
//    return result;
//}
//
//bool CreateGeometricRepresentationVisitor::visitIfcEdge(ifc2x3::IfcEdge* value)
//{
//    bool result = true;
//    Step::RefPtr<ifc2x3::IfcEdge> rpValue = value;
//
//    Step::RefPtr<ifc2x3::IfcVertexPoint> vertexE = mDataSet->createIfcVertexPoint();
//    Step::RefPtr<ifc2x3::IfcCartesianPoint> pointEnd = mDataSet->createIfcCartesianPoint();
//    auto edgeEnd = ((EdgeLoop*)mFaces[mCurrentFaceIndex].get())->EdgeList[mCurrentEdge]->EdgeEnd;
//    pointEnd->getCoordinates().push_back(edgeEnd.VertexGeometry.x);
//    pointEnd->getCoordinates().push_back(edgeEnd.VertexGeometry.y);
//    pointEnd->getCoordinates().push_back(edgeEnd.VertexGeometry.z);
//    vertexE->setVertexGeometry(pointEnd);
//
//    Step::RefPtr<ifc2x3::IfcVertexPoint> vertexS = mDataSet->createIfcVertexPoint();
//    Step::RefPtr<ifc2x3::IfcCartesianPoint> pointStart = mDataSet->createIfcCartesianPoint();
//    auto edgeStart = ((EdgeLoop*)mFaces[mCurrentFaceIndex].get())->EdgeList[mCurrentEdge]->EdgeStart;
//    pointStart->getCoordinates().push_back(edgeStart.VertexGeometry.x);
//    pointStart->getCoordinates().push_back(edgeStart.VertexGeometry.y);
//    pointStart->getCoordinates().push_back(edgeStart.VertexGeometry.z);
//    vertexS->setVertexGeometry(pointStart);
//
//    rpValue->setEdgeEnd(vertexE);
//    rpValue->setEdgeStart(vertexS);
//
//    return result;
//}
//
//bool CreateGeometricRepresentationVisitor::visitIfcOrientedEdge(ifc2x3::IfcOrientedEdge* value)
//{
//    bool result = true;
//    Step::RefPtr<ifc2x3::IfcOrientedEdge> rpValue = value;
//
//    Step::RefPtr<ifc2x3::IfcEdgeCurve> edgeCurve;
//
//    edgeCurve = mDataSet->createIfcEdgeCurve();
//    result &= edgeCurve->acceptVisitor(this);
//
//    rpValue->setEdgeElement(edgeCurve);
//
//    return result;
//}
//
//bool CreateGeometricRepresentationVisitor::visitIfcEdgeCurve(ifc2x3::IfcEdgeCurve* value)
//{
//    bool result = true;
//    Step::RefPtr<ifc2x3::IfcEdgeCurve> rpValue = value;
//
//    Step::RefPtr<ifc2x3::IfcVertexPoint> vertexE = mDataSet->createIfcVertexPoint();
//    Step::RefPtr<ifc2x3::IfcCartesianPoint> pointEnd = mDataSet->createIfcCartesianPoint();
//    auto edgeEnd = ((EdgeLoop*)mFaces[mCurrentFaceIndex].get())->EdgeList[mCurrentEdge]->EdgeEnd;
//    pointEnd->getCoordinates().push_back(edgeEnd.VertexGeometry.x);
//    pointEnd->getCoordinates().push_back(edgeEnd.VertexGeometry.y);
//    pointEnd->getCoordinates().push_back(edgeEnd.VertexGeometry.z);
//    vertexE->setVertexGeometry(pointEnd);
//
//    Step::RefPtr<ifc2x3::IfcVertexPoint> vertexS = mDataSet->createIfcVertexPoint();
//    Step::RefPtr<ifc2x3::IfcCartesianPoint> pointStart = mDataSet->createIfcCartesianPoint();
//    auto edgeStart = ((EdgeLoop*)mFaces[mCurrentFaceIndex].get())->EdgeList[mCurrentEdge]->EdgeStart;
//    pointStart->getCoordinates().push_back(edgeStart.VertexGeometry.x);
//    pointStart->getCoordinates().push_back(edgeStart.VertexGeometry.y);
//    pointStart->getCoordinates().push_back(edgeStart.VertexGeometry.z);
//    vertexS->setVertexGeometry(pointStart);
//
//    Step::RefPtr<ifc2x3::IfcCircle> circle = mDataSet->createIfcCircle();
//    result &= circle->acceptVisitor(this);
//
//    rpValue->setEdgeGeometry(circle);
//    rpValue->setEdgeEnd(vertexE);
//    rpValue->setEdgeStart(vertexS);
//    rpValue->setSameSense(((EdgeCurve*)((EdgeLoop*)mFaces[mCurrentFaceIndex].get())->EdgeList[mCurrentEdge].get())->SameSense);
//
//
//
//    return result;
//}
//
//bool CreateGeometricRepresentationVisitor::visitIfcVertexPoint(ifc2x3::IfcVertexPoint* value)
//{
//    bool result = true;
//    Step::RefPtr<ifc2x3::IfcVertexPoint> rpValue = value;
//
//    Step::RefPtr<ifc2x3::IfcCartesianPoint> p = mDataSet->createIfcCartesianPoint();
//    p->getCoordinates().push_back(0.0);//ajouter valeur
//    p->getCoordinates().push_back(0.0);//ajouter valeur
//    p->getCoordinates().push_back(0.0);//ajouter valeur
//    rpValue->setVertexGeometry(p.get());
//
//    rpValue->setVertexGeometry(p);
//
//    return result;
//}

bool CreateGeometricRepresentationVisitor::visitIfcCircle(ifc2x3::IfcCircle* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcCircle> rpValue = value;

    Step::RefPtr< ifc2x3::IfcAxis2Placement > axis2Placement = new ifc2x3::IfcAxis2Placement();
    Step::RefPtr<ifc2x3::IfcAxis2Placement3D> axis = mDataSet->createIfcAxis2Placement3D();

    auto& circle = ((TrimmedCurveEx*)((CompositeCurve*)mItems[mCurrentItemIndex].get())->CompositeCurveSegments[mCurrentCompositeCurveIndex].get())->Circle;

    mLocationType = LOCAL_PLACEMENT;
    result &= axis->acceptVisitor(this);
    mLocationType = UNDEF_LOC;

    axis2Placement->setIfcAxis2Placement3D(axis.get());
    rpValue->setPosition(axis2Placement);

    rpValue->setRadius(circle.Rayon);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcAxis2Placement3D(ifc2x3::IfcAxis2Placement3D* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcAxis2Placement3D> rpValue = value;

    Step::RefPtr<ifc2x3::IfcCartesianPoint> p = mDataSet->createIfcCartesianPoint();
    //Step::RefPtr<ifc2x3::IfcDirection> d = mDataSet->createIfcDirection();

    if (mIsEdgeCurve)
    {
        auto& circle = ((TrimmedCurveEx*)((CompositeCurve*)mItems[mCurrentItemIndex].get())->CompositeCurveSegments[mCurrentCompositeCurveIndex].get())->Circle;

        p->getCoordinates().push_back(circle.Centre.x);
        p->getCoordinates().push_back(circle.Centre.y);
        p->getCoordinates().push_back(circle.Centre.z);

        rpValue->setLocation(p.get());
    }
    else
    {
        p->getCoordinates().push_back(mPosition[0]);
        p->getCoordinates().push_back(mPosition[1]);
        p->getCoordinates().push_back(mPosition[2]);

        rpValue->setLocation(p.get());
    }

    return result;
}


bool CreateGeometricRepresentationVisitor::visitIfcCoveringType(ifc2x3::IfcCoveringType* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcCoveringType > rpValue = value;
    Step::RefPtr < ifc2x3::IfcPropertySet> propertySet;
    propertySet = mDataSet->createIfcPropertySet();

    result &= propertySet->acceptVisitor(this);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcPropertySet(ifc2x3::IfcPropertySet* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcPropertySet > rpValue = value;

    rpValue->setDescription("");

    return result;
}

//bool CreateGeometricRepresentationVisitor::visitIfcProduct(ifc2x3::IfcProduct * value)
//{
//   bool result = true;
//   Step::RefPtr< ifc2x3::IfcProduct > rpValue = value;
//
//   return result;
//}

