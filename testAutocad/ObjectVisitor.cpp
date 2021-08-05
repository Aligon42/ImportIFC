#include "ObjectVisitor.h"
#include "ComputePlacementVisitor.h"

ObjectVisitor::ObjectVisitor() 
	: ifc2x3::InheritVisitor() 
{
	m_IfcObject = new IFCObject();
}

ObjectVisitor::ObjectVisitor(IFCObject* obj)
	: m_IfcObject(obj), ifc2x3::InheritVisitor() { }

bool ObjectVisitor::visitIfcProduct(ifc2x3::IfcProduct* value)
{
	m_IfcObject->Key = value->getKey();
	m_IfcObject->Entity = value->type();

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
		ObjectVisitor visitor(m_IfcObject);
		representation->acceptVisitor(&visitor);

		if (visitor.m_ShapeRepresentations.size() > 0)
		{
			m_ShapeRepresentations.assign(visitor.m_ShapeRepresentations.begin(), visitor.m_ShapeRepresentations.end());
		}
		else
		{
			auto shape = visitor.getShapeRepresentation();

			if (shape.Key != 0)
				m_ShapeRepresentations.push_back(shape);
		}
	}

	m_IfcObject->ShapeRepresentations.assign(m_ShapeRepresentations.begin(), m_ShapeRepresentations.end());

	return true;
}

bool ObjectVisitor::visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value)
{
	m_IfcShapeRepresentation.RepresentationIdentifier = value->getRepresentationIdentifier().toUTF8();
	m_IfcShapeRepresentation.RepresentationType = value->getRepresentationType().toUTF8();

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
		ObjectVisitor visitor(m_IfcObject);

		auto op1 = value->getFirstOperand();

		switch (op1->currentType())
		{
			case ifc2x3::IfcBooleanOperand::IFCBOOLEANRESULT:
				r1 = op1->getIfcBooleanResult()->acceptVisitor(this);
				break;
			case ifc2x3::IfcBooleanOperand::IFCCSGPRIMITIVE3D:
				r1 = op1->getIfcCsgPrimitive3D()->acceptVisitor(&visitor);
				break;
			case ifc2x3::IfcBooleanOperand::IFCHALFSPACESOLID:
			{
				r1 = op1->getIfcHalfSpaceSolid()->acceptVisitor(&visitor);

				auto shape = visitor.getShapeRepresentation();
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
			case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			{
				r1 = op1->getIfcSolidModel()->acceptVisitor(&visitor);

				auto shape = visitor.getShapeRepresentation();
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
		}
	}

	if (value->testSecondOperand())
	{
		ObjectVisitor visitor(m_IfcObject);

		auto op2 = value->getSecondOperand();

		switch (op2->currentType())
		{
			case ifc2x3::IfcBooleanOperand::IFCBOOLEANRESULT:
				r2 = op2->getIfcBooleanResult()->acceptVisitor(this);
				break;
			case ifc2x3::IfcBooleanOperand::IFCCSGPRIMITIVE3D:
				r2 = op2->getIfcCsgPrimitive3D()->acceptVisitor(&visitor);
				break;
			case ifc2x3::IfcBooleanOperand::IFCHALFSPACESOLID:
			{
				r2 = op2->getIfcHalfSpaceSolid()->acceptVisitor(&visitor);

				auto shape = visitor.getShapeRepresentation();
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
			case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			{

				r2 = op2->getIfcSolidModel()->acceptVisitor(&visitor);

				auto shape = visitor.getShapeRepresentation();
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
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
	m_IfcShapeRepresentation.EntityType = value->type();
	m_IfcShapeRepresentation.Key = value->getKey();

	if (value->testBaseSurface())
	{
		value->getBaseSurface()->acceptVisitor(this);
	}
	if (value->testAgreementFlag())
	{
		m_IfcShapeRepresentation.AgreementHalf = value->getAgreementFlag();
	}

	m_IfcShapeRepresentation.EntityHalf = value->getClassType().getName();

	return true;
}

bool ObjectVisitor::visitIfcPolygonalBoundedHalfSpace(ifc2x3::IfcPolygonalBoundedHalfSpace* value)
{
	m_IfcShapeRepresentation.EntityType = value->type();
	m_IfcShapeRepresentation.Key = value->getKey();

	if (value->testPolygonalBoundary())
	{
		if (value->getPolygonalBoundary()->acceptVisitor(this))
		{
			if (value->testPosition())
			{
				m_IfcShapeRepresentation.Transformation = ComputePlacementVisitor::getTransformation(value->getPosition());
			}

			if (value->testAgreementFlag())
			{
				m_IfcShapeRepresentation.AgreementPolygonal = value->getAgreementFlag();
			}

			if (value->testBaseSurface())
			{
				value->getBaseSurface()->acceptVisitor(this);
			}

			m_IfcShapeRepresentation.EntityHalf = value->getClassType().getName();

			return true;
		}
	}
	return false;
}

bool ObjectVisitor::visitIfcCompositeCurve(ifc2x3::IfcCompositeCurve* value)
{
	if (value->testSelfIntersect())
	{
		m_IfcShapeRepresentation.AgreementCompositeCurve = value->getSelfIntersect();
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
	}

	return true;
}

bool ObjectVisitor::visitIfcCompositeCurveSegment(ifc2x3::IfcCompositeCurveSegment* value)
{
	bool success = false;
	m_VisitingCompositeCurve = true;
	CompositeCurveSegment compositeCurve;

	if (value->testTransition())
	{
		compositeCurve.Transition = value->getTransition();
	}
	if (value->testSameSense())
	{
		compositeCurve.SameSense = value->getSameSense();
	}
	if (value->testParentCurve())
	{
		compositeCurve.ParentCurve = value->getParentCurve()->getClassType().getName();
		m_IfcShapeRepresentation.CompositeCurve.push_back(compositeCurve);
		success = value->getParentCurve()->acceptVisitor(this);
	}

	m_VisitingCompositeCurve = false;

	return success;
}

bool ObjectVisitor::visitIfcTrimmedCurve(ifc2x3::IfcTrimmedCurve* value)
{
	auto trimmedCurve = new TrimmedCurve();

	if (value->testTrim1())
	{
		trimmedCurve->Trim1 = value->getTrim1().begin()->get()->getIfcParameterValue();
	}
	if (value->testTrim2())
	{
		trimmedCurve->Trim2 = value->getTrim2().begin()->get()->getIfcParameterValue();
	}
	if (value->testSenseAgreement())
	{
		trimmedCurve->SenseAgreement = value->getSenseAgreement() - 1;
	}

	m_IfcShapeRepresentation.CompositeCurve[m_IfcShapeRepresentation.CompositeCurve.size() - 1].TrimmedCurves = trimmedCurve;

	if (value->testBasisCurve())
	{
		value->getBasisCurve()->acceptVisitor(this);
	}

	return true;
}

bool ObjectVisitor::visitIfcCircle(ifc2x3::IfcCircle* value)
{
	auto& compositeCurve = m_IfcShapeRepresentation.CompositeCurve[m_IfcShapeRepresentation.CompositeCurve.size() - 1];

	if (value->testRadius())
	{
		compositeCurve.TrimmedCurves->Radius = value->getRadius();
	}
	if (value->testPosition())
	{
		value->getPosition()->acceptVisitor(this);
	}

	return true;
}

bool ObjectVisitor::visitIfcPlane(ifc2x3::IfcPlane* value)
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

		m_IfcShapeRepresentation.Plan = plan;

		return true;
	}

	return false;
}

bool ObjectVisitor::visitIfcAxis2Placement(ifc2x3::IfcAxis2Placement* value)
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

bool ObjectVisitor::visitIfcAxis2Placement2D(ifc2x3::IfcAxis2Placement2D* value)
{
	if (value->testLocation())
	{
		auto coordonnees = value->getLocation()->getCoordinates();
		auto& compositeCurve = m_IfcShapeRepresentation.CompositeCurve[m_IfcShapeRepresentation.CompositeCurve.size() - 1];

		compositeCurve.TrimmedCurves->centreCircle = { coordonnees[0], coordonnees[1], 0.0 };
	}

	return true;
}

bool ObjectVisitor::visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value)
{
	m_IfcShapeRepresentation.EntityType = value->type();
	m_IfcShapeRepresentation.Key = value->getKey();

	if (value->testSweptArea())
	{
		if (value->getSweptArea()->acceptVisitor(this))
		{
			m_IfcShapeRepresentation.Transformation = ComputePlacementVisitor::getTransformation(value->getPosition());

			transformPoints(m_IfcShapeRepresentation.Transformation);

			m_IfcShapeRepresentation.ProfilDefName = value->getSweptArea()->getType().getName();

			if (value->testExtrudedDirection())
			{
				m_IfcObject->ExtrusionVector = ComputePlacementVisitor::getDirection(value->getExtrudedDirection());
			}
			if (value->testDepth())
			{
				m_IfcObject->ExtrusionHeight = value->getDepth();
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
	if (!m_VisitingCompositeCurve)
	{
		m_IfcShapeRepresentation.OuterCurveName = value->type();

		auto& shapePoints = m_IfcShapeRepresentation.Points;

		int size = shapePoints.size();
		int firstKey = 0, lastKey = 0;

		auto& points = value->getPoints();

		for (int i = 0; i < points.size(); i++)
		{
			if (i == 0)
				firstKey = points[i]->getKey();
			else if (i == points.size() - 1)
				lastKey = points[i]->getKey();

			shapePoints.push_back(ComputePlacementVisitor::getPoint(points[i].get()));
		}

		if (shapePoints.size() > 0)
		{
			if (firstKey == lastKey)
				shapePoints.pop_back();
		}

		return shapePoints.empty() == false;
	}
	else
	{
		auto& shapePoints = m_IfcShapeRepresentation.CompositeCurve[m_IfcShapeRepresentation.CompositeCurve.size() - 1].PointsPolyligne;

		int size = shapePoints.size();
		int firstKey = 0, lastKey = 0;

		auto& points = value->getPoints();

		for (int i = 0; i < points.size(); i++)
		{
			if (i == 0)
				firstKey = points[i]->getKey();
			else if (i == points.size() - 1)
				lastKey = points[i]->getKey();

			shapePoints.push_back(ComputePlacementVisitor::getPoint(points[i].get()));
		}

		if (shapePoints.size() > 0)
		{
			if (firstKey == lastKey)
				shapePoints.pop_back();
		}

		return shapePoints.empty() == false;
	}
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

IFCObject* ObjectVisitor::getIfcObject()
{
	m_IfcObject->LocalTransform = m_IfcShapeRepresentation.Transformation;

	return m_IfcObject;
}

void ObjectVisitor::transformPoints(const Matrix4& transform)
{
	std::list<Vec3> tmpPoints = m_IfcShapeRepresentation.Points;

	m_IfcShapeRepresentation.Points.clear();

	for (const auto& point : tmpPoints)
	{
		m_IfcShapeRepresentation.Points.push_back(transform * point);
	}
}

void ObjectVisitor::SwitchIfcCartesianPointToVecteur3D(ifc2x3::IfcCartesianPoint* value, Vec3& outOrigine)
{
	auto listPoint = value->getCoordinates();

	outOrigine.x() = listPoint.at(0);
	outOrigine.y() = listPoint.at(1);
	outOrigine.z() = listPoint.at(2);
}

void ObjectVisitor::SwitchIfcDirectionToVecteur3D(ifc2x3::IfcDirection* value, Vec3& outVecteur)
{
	auto listPoint = value->getDirectionRatios();

	outVecteur.x() = listPoint.at(0);
	outVecteur.y() = listPoint.at(1);
	outVecteur.z() = listPoint.at(2);
}

IFCShapeRepresentation ObjectVisitor::getShapeRepresentation()
{
	return m_IfcShapeRepresentation;
}
