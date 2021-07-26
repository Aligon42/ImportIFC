#include "ObjectVisitor.h"
#include "ComputePlacementVisitor.h"

ObjectVisitor::ObjectVisitor() : ifc2x3::InheritVisitor() { }

bool ObjectVisitor::visitIfcProduct(ifc2x3::IfcProduct* value)
{
	m_Key = value->getKey();
	m_Entity = value->type();

	if (value->testRepresentation())
	{
		return value->getRepresentation()->acceptVisitor(this);
	}

	return true;
}

bool ObjectVisitor::visitIfcSite(ifc2x3::IfcSite* value)
{
	return false;
}

bool ObjectVisitor::visitIfcRelVoidsElement(ifc2x3::IfcRelVoidsElement* value)
{
	return false;
}

bool ObjectVisitor::visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value)
{
	return false;
}

bool ObjectVisitor::visitIfcProductDefinitionShape(ifc2x3::IfcProductDefinitionShape* value)
{
	for (auto& representation : value->getRepresentations())
	{
		ObjectVisitor visitor;
		representation->acceptVisitor(&visitor);

		m_ShapeRepresentations.push_back(visitor.getShapeRepresentation());
	}

	return true;
}

bool ObjectVisitor::visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value)
{
	m_RepresentationIdentifier = value->getRepresentationIdentifier().toUTF8();
	m_RepresentationType = value->getRepresentationType().toUTF8();

	for (auto& shape : value->getItems())
	{
		shape->acceptVisitor(this);
	}

	return false;
}

bool ObjectVisitor::visitIfcBooleanClippingResult(ifc2x3::IfcBooleanClippingResult* value)
{
	bool r1 = false, r2 = false;

	if (value->testFirstOperand())
	{
		ObjectVisitor visitor;

		auto op1 = value->getFirstOperand();

		switch (op1->currentType())
		{
		case ifc2x3::IfcBooleanOperand::IFCBOOLEANRESULT:
			r1 = op1->getIfcBooleanResult()->acceptVisitor(&visitor);
			break;
		case ifc2x3::IfcBooleanOperand::IFCCSGPRIMITIVE3D:
			r1 = op1->getIfcCsgPrimitive3D()->acceptVisitor(&visitor);
			break;
		case ifc2x3::IfcBooleanOperand::IFCHALFSPACESOLID:
			r1 = op1->getIfcHalfSpaceSolid()->acceptVisitor(&visitor);
			break;
		case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			r1 = op1->getIfcSolidModel()->acceptVisitor(&visitor);
			break;
		}


	}

	if (value->testSecondOperand())
	{
		ObjectVisitor visitor;

		auto op2 = value->getSecondOperand();

		switch (op2->currentType())
		{
		case ifc2x3::IfcBooleanOperand::IFCBOOLEANRESULT:
			r2 = op2->getIfcBooleanResult()->acceptVisitor(&visitor);
			break;
		case ifc2x3::IfcBooleanOperand::IFCCSGPRIMITIVE3D:
			r2 = op2->getIfcCsgPrimitive3D()->acceptVisitor(&visitor);
			break;
		case ifc2x3::IfcBooleanOperand::IFCHALFSPACESOLID:
			r2 = op2->getIfcHalfSpaceSolid()->acceptVisitor(&visitor);
			break;
		case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			r2 = op2->getIfcSolidModel()->acceptVisitor(&visitor);
			break;
		}
	}

	return r1 && r2;
}

bool ObjectVisitor::visitIfcRepresentationMap(ifc2x3::IfcRepresentationMap* value)
{
	return false;
}

bool ObjectVisitor::visitIfcFaceBasedSurfaceModel(ifc2x3::IfcFaceBasedSurfaceModel* value)
{
	return false;
}

bool ObjectVisitor::visitIfcConnectedFaceSet(ifc2x3::IfcConnectedFaceSet* value)
{
	return false;
}

bool ObjectVisitor::visitIfcShellBasedSurfaceModel(ifc2x3::IfcShellBasedSurfaceModel* value)
{
	return false;
}

bool ObjectVisitor::visitIfcOpenShell(ifc2x3::IfcOpenShell* value)
{
	return false;
}

bool ObjectVisitor::visitIfcMappedItem(ifc2x3::IfcMappedItem* value)
{
	return false;
}

bool ObjectVisitor::visitIfcCartesianTransformationOperator3D(ifc2x3::IfcCartesianTransformationOperator3D* value)
{
	return false;
}

bool ObjectVisitor::visitIfcHalfSpaceSolid(ifc2x3::IfcHalfSpaceSolid* value)
{
	return false;
}

bool ObjectVisitor::visitIfcPolygonalBoundedHalfSpace(ifc2x3::IfcPolygonalBoundedHalfSpace* value)
{
	return false;
}

bool ObjectVisitor::visitIfcCompositeCurve(ifc2x3::IfcCompositeCurve* value)
{
	return false;
}

bool ObjectVisitor::visitIfcCompositeCurveSegment(ifc2x3::IfcCompositeCurveSegment* value)
{
	return false;
}

bool ObjectVisitor::visitIfcTrimmedCurve(ifc2x3::IfcTrimmedCurve* value)
{
	return false;
}

bool ObjectVisitor::visitIfcCircle(ifc2x3::IfcCircle* value)
{
	return false;
}

bool ObjectVisitor::visitIfcPlane(ifc2x3::IfcPlane* value)
{
	return false;
}

bool ObjectVisitor::visitIfcAxis2Placement(ifc2x3::IfcAxis2Placement* value)
{
	return false;
}

bool ObjectVisitor::visitIfcAxis2Placement2D(ifc2x3::IfcAxis2Placement2D* value)
{
	return false;
}

bool ObjectVisitor::visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value)
{
	m_Entity = value->type();
	m_Key = value->getKey();

	if (value->testSweptArea())
	{
		if (value->getSweptArea()->acceptVisitor(this))
		{
			m_Transformation = ComputePlacementVisitor::getTransformation(value->getPosition());

			transformPoints(m_Transformation);

			m_NameProfilDef = value->getSweptArea()->getType().getName();

			if (value->testExtrudedDirection())
			{
				m_ExtrusionVector = ComputePlacementVisitor::getDirection(value->getExtrudedDirection());
			}
			if (value->testDepth())
			{
				m_ExtrusionHeight = value->getDepth();
			}

			return true;
		}
	}

	return false;
}

bool ObjectVisitor::visitIfcBoundingBox(ifc2x3::IfcBoundingBox* value)
{
	return false;
}

bool ObjectVisitor::visitIfcStyledItem(ifc2x3::IfcStyledItem* value)
{
	return false;
}

bool ObjectVisitor::visitIfcPresentationStyleAssignment(ifc2x3::IfcPresentationStyleAssignment* value)
{
	return false;
}

bool ObjectVisitor::visitIfcSurfaceStyle(ifc2x3::IfcSurfaceStyle* value)
{
	return false;
}

bool ObjectVisitor::visitIfcSurfaceStyleRendering(ifc2x3::IfcSurfaceStyleRendering* value)
{
	return false;
}

bool ObjectVisitor::visitIfcColourRgb(ifc2x3::IfcColourRgb* value)
{
	return false;
}

bool ObjectVisitor::visitIfcIShapeProfileDef(ifc2x3::IfcIShapeProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcLShapeProfileDef(ifc2x3::IfcLShapeProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcTShapeProfileDef(ifc2x3::IfcTShapeProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcUShapeProfileDef(ifc2x3::IfcUShapeProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcCShapeProfileDef(ifc2x3::IfcCShapeProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcZShapeProfileDef(ifc2x3::IfcZShapeProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcAsymmetricIShapeProfileDef(ifc2x3::IfcAsymmetricIShapeProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcCircleHollowProfileDef(ifc2x3::IfcCircleHollowProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcRectangleHollowProfileDef(ifc2x3::IfcRectangleHollowProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcRectangleProfileDef(ifc2x3::IfcRectangleProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcCircleProfileDef(ifc2x3::IfcCircleProfileDef* value)
{
	return false;
}

bool ObjectVisitor::visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value)
{
	if (value->testOuterCurve())
	{
		return value->getOuterCurve()->acceptVisitor(this);
	}

	return false;
}

bool ObjectVisitor::visitIfcPolyline(ifc2x3::IfcPolyline* value)
{
	int size = m_Points.size();
	int firstKey = 0, lastKey = 0;

	auto& points = value->getPoints();

	for (int i = 0; i < points.size(); i++)
	{
		if (i == 0)
			firstKey = points[i]->getKey();
		else if (i == points.size() - 1)
			lastKey = points[i]->getKey();

		m_Points.push_back(ComputePlacementVisitor::getPoint(points[i].get()));
	}

	if (m_Points.size() > 0)
	{
		if (firstKey == lastKey)
			m_Points.pop_back();
	}

	return m_Points.empty() == false;
}

bool ObjectVisitor::visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value)
{
	return false;
}

bool ObjectVisitor::visitIfcClosedShell(ifc2x3::IfcClosedShell* value)
{
	return false;
}

bool ObjectVisitor::visitIfcFace(ifc2x3::IfcFace* value)
{
	return false;
}

bool ObjectVisitor::visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value)
{
	return false;
}

bool ObjectVisitor::visitIfcFaceBound(ifc2x3::IfcFaceBound* value)
{
	return false;
}

bool ObjectVisitor::visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value)
{
	return false;
}

IFCObjTemp ObjectVisitor::getIfcObject()
{
	return { m_Key, m_Entity, m_ShapeRepresentations };
}

void ObjectVisitor::transformPoints(const Matrix4& transform)
{
	std::list<Vec3> tmpPoints = m_Points;

	m_Points.clear();

	for (const auto& point : tmpPoints)
	{
		m_Points.push_back(transform * point);
	}
}

IFCShapeRepresentation* ObjectVisitor::getShapeRepresentation()
{
	IFCExtrudedAreaSolid* representation = nullptr;

	if (m_Entity == "IfcExtrudedAreaSolid")
	{
		representation = new IFCExtrudedAreaSolid(m_Key, m_Entity, m_RepresentationIdentifier, m_RepresentationType, m_NameProfilDef, m_Transformation, m_ExtrusionVector, m_ExtrusionHeight, m_Points);
	}

	if (m_Entity == "IfcBooleanClippingResult")
	{

	}

	return representation;
}
