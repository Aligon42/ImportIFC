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
    for (it = rpValue->getRepresentations().begin(); it != rpValue->getRepresentations().end(); it++) {
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
    }

    if (!shapeRepresentation.valid()) {
        shapeRepresentation = mDataSet->createIfcShapeRepresentation();
        rpValue->getRepresentations().push_back(shapeRepresentation.get());
    }

    result &= shapeRepresentation->acceptVisitor(this);

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcRepresentation(ifc2x3::IfcRepresentation* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcRepresentation > rpValue = value;

    Step::RefPtr< ifc2x3::IfcRepresentationItem > representationItem;

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
        if (isCoveringExp == true)
        {
            representationItem = (ifc2x3::IfcRepresentationItem*)mDataSet->createIfcFacetedBrep().get();
            isCoveringExp = false;
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

    rpValue->getItems().insert(representationItem.get());

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

    mFaceIndex = 0;
    mOffset = 0;

    for (int i = 0; i < mElements.size(); i++)
    {
        face = mDataSet->createIfcFace().get();
        result &= face->acceptVisitor(this);
        face_1_n.emplace(face);
        mFaceIndex++;
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
    else {
        rpValue->setOuterCurve(mPolyline.get());
        result &= true;
    }

    return result;
}

bool CreateGeometricRepresentationVisitor::visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value)
{
    bool result = true;
    Step::RefPtr<ifc2x3::IfcFaceOuterBound> rpValue = value;

    Step::RefPtr<ifc2x3::IfcPolyLoop> polyloop;
    Step::Boolean boolean = Step::Boolean::BTrue;

    polyloop = mDataSet->createIfcPolyLoop().get();
    rpValue->setBound(polyloop);
    rpValue->setOrientation(boolean);

    result &= polyloop->acceptVisitor(this);

    return result;

}

bool CreateGeometricRepresentationVisitor::visitIfcPolyline(ifc2x3::IfcPolyline* value)
{
    bool result = true;
    Step::RefPtr< ifc2x3::IfcPolyline > rpValue = value;

    for (unsigned int i = 0; i < m2DPolyline.size() / 2; i++) {
        Step::RefPtr< ifc2x3::IfcCartesianPoint > p = mDataSet->createIfcCartesianPoint();
        p->getCoordinates().push_back(m2DPolyline[2 * i]);
        p->getCoordinates().push_back(m2DPolyline[2 * i + 1]);
        rpValue->getPoints().push_back(p.get());
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

    int internalCount = 0;
    for (unsigned int i = mOffset * mElements[mFaceIndex]; i < mPolyloop.size() / 3; i++)
    {
        if (internalCount == mElements[mFaceIndex]) break;

        Step::RefPtr<ifc2x3::IfcCartesianPoint> p = mDataSet->createIfcCartesianPoint();
        p->getCoordinates().push_back(mPolyloop[3 * i]);
        p->getCoordinates().push_back(mPolyloop[3 * i + 1]);
        p->getCoordinates().push_back(mPolyloop[3 * i + 2]);
        rpValue->getPolygon().push_back(p.get());

        internalCount++;
    }

    mOffset++;

    if (mPolyloopMustBeClosed &&
        (*rpValue->getPolygon().begin())->getCoordinates() != (*rpValue->getPolygon().rbegin())->getCoordinates()) {
        rpValue->getPolygon().push_back(*rpValue->getPolygon().begin());
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

    rpValue->setDescription("bla bla bla");


    return result;
}

//bool CreateGeometricRepresentationVisitor::visitIfcProduct(ifc2x3::IfcProduct * value)
//{
//   bool result = true;
//   Step::RefPtr< ifc2x3::IfcProduct > rpValue = value;
//
//   return result;
//}

