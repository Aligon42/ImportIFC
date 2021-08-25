#include "ObjectVisitor.h"
#include "ComputePlacementVisitor.h"

ObjectVisitor::ObjectVisitor(int representationCount, int visitingDepth)
	: m_RepresentationCount(representationCount), m_VisitingDepth(visitingDepth), ifc2x3::InheritVisitor()
{
	m_IfcObject = std::make_shared<IFCObject>();
}

ObjectVisitor::ObjectVisitor(std::shared_ptr<IFCObject> obj, int representationCount, int visitingDepth)
	: m_IfcObject(obj), m_RepresentationCount(representationCount), m_VisitingDepth(visitingDepth), ifc2x3::InheritVisitor() { }

ObjectVisitor::ObjectVisitor(std::shared_ptr<IFCObject> obj, IFCShapeRepresentation& shape, int representationCount, int visitingDepth)
	: m_IfcObject(obj), m_IfcShapeRepresentation(shape), m_RepresentationCount(representationCount), m_VisitingDepth(visitingDepth), ifc2x3::InheritVisitor() { }

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
	if (value->testRepresentation())
	{
		value->getRepresentation()->acceptVisitor(this);
	}

	return true;
}

bool ObjectVisitor::visitIfcRelVoidsElement(ifc2x3::IfcRelVoidsElement* value)
{
	if (value->testRelatedOpeningElement())
	{
		value->getRelatedOpeningElement()->acceptVisitor(this);
	}
	if (value->testRelatingBuildingElement())
	{
		m_IfcObject->VoidKey = value->getRelatingBuildingElement()->getKey();
	}

	m_IfcObject->Key = value->getKey();
	m_IfcObject->Entity = value->type();

	return true;
}

bool ObjectVisitor::visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value)
{
	return false;
}

bool ObjectVisitor::visitIfcProductDefinitionShape(ifc2x3::IfcProductDefinitionShape* value)
{
	auto& representations = value->getRepresentations();
	m_RepresentationCount = representations.size();

	for (auto& representation : representations)
	{
		auto type = representation->getRepresentationType();
		auto identifier = representation->getRepresentationIdentifier();

		if (identifier == "Axis" && type == "Curve2D") continue;

		ObjectVisitor visitor(m_IfcObject, m_RepresentationCount, m_VisitingDepth + 1);
		representation->acceptVisitor(&visitor);

		if (visitor.m_ShapeRepresentations.size() > 0)
		{
			m_ShapeRepresentations.assign(visitor.m_ShapeRepresentations.begin(), visitor.m_ShapeRepresentations.end());
		}
		else
		{
			auto shape = visitor.getShapeRepresentation();

			if (shape.Key != 0 || m_IfcObject->IsMappedItem)
			{
				m_ShapeRepresentations.push_back(shape);
			}
		}
	}

	m_IfcObject->ShapeRepresentations.assign(m_ShapeRepresentations.begin(), m_ShapeRepresentations.end());

	return true;
}

bool ObjectVisitor::visitIfcShapeRepresentation(ifc2x3::IfcShapeRepresentation* value)
{
	m_IfcShapeRepresentation.RepresentationIdentifier = value->getRepresentationIdentifier().toUTF8();
	m_IfcShapeRepresentation.RepresentationType = value->getRepresentationType().toUTF8();

	auto& shapesRepresentation = value->getItems();

	for (auto& shape : shapesRepresentation)
	{
		ObjectVisitor visitor(m_IfcObject, m_RepresentationCount, m_VisitingDepth + 1);
		shape->acceptVisitor(&visitor);

		auto shapes = visitor.getShapeRepresentations();
		if (shapes.size() > 0)
			m_ShapeRepresentations.insert(m_ShapeRepresentations.end(), shapes.begin(), shapes.end());
		else
			m_ShapeRepresentations.push_back(visitor.getShapeRepresentation());
	}

	return true;
}

bool ObjectVisitor::visitIfcBooleanResult(ifc2x3::IfcBooleanResult* value)
{
	bool r1 = false, r2 = false;

	if (value->testFirstOperand())
	{
		ObjectVisitor visitor(m_IfcObject, m_VisitingDepth + 1);
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
			{
				r1 = op1->getIfcHalfSpaceSolid()->acceptVisitor(&visitor);
				auto shape = visitor.getShapeRepresentation();
				shape.BooleanResult = true;
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
			case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			{
				r1 = op1->getIfcSolidModel()->acceptVisitor(&visitor);
				auto shape = visitor.getShapeRepresentation();
				shape.BooleanResult = true;
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
		}
	}

	if (value->testSecondOperand())
	{
		ObjectVisitor visitor(m_IfcObject, m_VisitingDepth + 1);
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
			{
				r2 = op2->getIfcHalfSpaceSolid()->acceptVisitor(&visitor);
				auto shape = visitor.getShapeRepresentation();
				shape.BooleanResult = true;
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
			case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			{
				r2 = op2->getIfcSolidModel()->acceptVisitor(&visitor);
				auto shape = visitor.getShapeRepresentation();
				shape.BooleanResult = true;
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
		}
	}

	return r1 && r2;
}

bool ObjectVisitor::visitIfcBooleanClippingResult(ifc2x3::IfcBooleanClippingResult* value)
{
	bool r1 = false, r2 = false;

	if (value->testFirstOperand())
	{
		ObjectVisitor visitor(m_IfcObject, m_VisitingDepth + 1);

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
				shape.BooleanResult = true;
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
			case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			{
				r1 = op1->getIfcSolidModel()->acceptVisitor(&visitor);
				auto shape = visitor.getShapeRepresentation();
				shape.BooleanResult = true;
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
		}
	}

	if (value->testSecondOperand())
	{
		ObjectVisitor visitor(m_IfcObject, m_VisitingDepth + 1);

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
				shape.BooleanResult = true;
				if (shape.Key != 0)
					m_ShapeRepresentations.push_back(shape);

				break;
			}
			case ifc2x3::IfcBooleanOperand::IFCSOLIDMODEL:
			{
				r2 = op2->getIfcSolidModel()->acceptVisitor(&visitor);
				auto shape = visitor.getShapeRepresentation();
				shape.BooleanResult = true;
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
	if (value->testMappedRepresentation())
	{
		if (value->getMappedRepresentation()->acceptVisitor(this))
		{
			if (value->testMappingOrigin()
				&& value->getMappingOrigin()->currentType() ==
				ifc2x3::IfcAxis2Placement::IFCAXIS2PLACEMENT3D)
			{
				m_IfcShapeRepresentation.Transformation = ComputePlacementVisitor::getTransformation(
					value->getMappingOrigin()->getIfcAxis2Placement3D());
			}

			return true;
		}
	}

	return false;
}

bool ObjectVisitor::visitIfcFaceBasedSurfaceModel(ifc2x3::IfcFaceBasedSurfaceModel* value)
{
	m_IfcShapeRepresentation.EntityType = value->type();
	m_IfcShapeRepresentation.Key = value->getKey();

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

bool ObjectVisitor::visitIfcConnectedFaceSet(ifc2x3::IfcConnectedFaceSet* value)
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

bool ObjectVisitor::visitIfcShellBasedSurfaceModel(ifc2x3::IfcShellBasedSurfaceModel* value)
{
	m_IfcShapeRepresentation.EntityType = value->type();
	m_IfcShapeRepresentation.Key = value->getKey();

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

bool ObjectVisitor::visitIfcShell(ifc2x3::IfcShell* value)
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

bool ObjectVisitor::visitIfcOpenShell(ifc2x3::IfcOpenShell* value)
{
	m_Face.Key = value->getKey();

	if (value->testCfsFaces())
	{
		for (auto face : value->getCfsFaces())
		{
			m_Face.SupType = value->type();

			if (!(face->acceptVisitor(this)))
			{
				return false;
			}
		}
	}

	return true;
}

bool ObjectVisitor::visitIfcMappedItem(ifc2x3::IfcMappedItem* value)
{
	m_IfcObject->IsMappedItem = true;

	m_IfcShapeRepresentation.EntityType = value->type();
	m_IfcShapeRepresentation.Key = value->getKey();

	if (value->testMappingSource())
	{
		ObjectVisitor visitor(m_RepresentationCount, m_VisitingDepth + 1);

		if (value->getMappingSource()->acceptVisitor(&visitor))
		{
			if (value->testMappingTarget())
			{
				value->getMappingTarget()->acceptVisitor(this);
			}

			if (visitor.m_ShapeRepresentations.size() > 0)
			{
				m_IfcShapeRepresentation.SubShapeRepresentations.assign(visitor.m_ShapeRepresentations.begin(), visitor.m_ShapeRepresentations.end());
			}
			else
			{
				auto shape = visitor.getShapeRepresentation();

				if (shape.Key != 0 || m_IfcObject->IsMappedItem)
				{
					m_IfcShapeRepresentation.SubShapeRepresentations.push_back(shape);
				}
			}
			
			return true;
		}
	}

	return false;
}

bool ObjectVisitor::visitIfcCartesianTransformationOperator3D(ifc2x3::IfcCartesianTransformationOperator3D* value)
{
	Vec3 Axis1(1.0f, 0.0f, 0.0f);
	if (value->testAxis1())
	{
		SwitchIfcDirectionToVecteur3D(value->getAxis1(), Axis1);
		Axis1.Normalize();
	}

	Vec3 Axis2(0.0f, 1.0f, 0.0f);
	if (value->testAxis2())
	{
		SwitchIfcDirectionToVecteur3D(value->getAxis2(), Axis2);
		Axis2.Normalize();
	}

	Vec3 Axis3(0.0f, 0.0f, 1.0f);
	if (value->testAxis3())
	{
		SwitchIfcDirectionToVecteur3D(value->getAxis3(), Axis3);
		Axis3.Normalize();
	}

	Vec3 localOrigine(0.0f, 0.0f, 0.0f);
	if (value->testLocalOrigin())
	{
		SwitchIfcCartesianPointToVecteur3D(value->getLocalOrigin(), localOrigine);
	}

	Matrix4 operator3D(
		Axis3.x(), Axis1.x(), Axis2.x(), localOrigine.x(),
		Axis3.y(), Axis1.y(), Axis2.y(), localOrigine.y(),
		Axis3.z(), Axis1.z(), Axis2.z(), localOrigine.z(),
		0.0f, 0.0f, 0.0f, 1.0f
	);

	m_IfcShapeRepresentation.Scale = value->getScale();
	m_IfcShapeRepresentation.TransformationOperator3D = operator3D;
	m_IfcShapeRepresentation.DeterminantMatrixOperator3D = (Axis1.x() * Axis2.y() * Axis3.z()) - (Axis2.x() * Axis1.y() * Axis3.z()) + (Axis3.x() * Axis1.y() * Axis2.z()) - localOrigine.x();

	return true;
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

			if (m_IfcShapeRepresentation.ProfilDef != nullptr)
				m_IfcShapeRepresentation.ProfilDef->Transform = m_IfcShapeRepresentation.Transformation;

			m_IfcShapeRepresentation.ProfilDefName = value->getSweptArea()->getType().getName();

			if (value->testExtrudedDirection())
			{
				m_IfcShapeRepresentation.ExtrusionVector = ComputePlacementVisitor::getDirection(value->getExtrudedDirection());

				if (m_IfcShapeRepresentation.ProfilDef != nullptr)
					m_IfcShapeRepresentation.ProfilDef->VecteurExtrusion = m_IfcShapeRepresentation.ExtrusionVector;
			}
			if (value->testDepth())
			{
				m_IfcShapeRepresentation.ExtrusionHeight = value->getDepth();

				if (m_IfcShapeRepresentation.ProfilDef != nullptr)
					m_IfcShapeRepresentation.ProfilDef->HauteurExtrusion = m_IfcShapeRepresentation.ExtrusionHeight;
			}

			if (m_IfcShapeRepresentation.ProfilDef != nullptr)
				m_IfcShapeRepresentation.ProfilDef->ParentObject = m_IfcObject;

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
	if (value->testItem())
	{
		m_Style.keyItem = value->getItem()->getKey();
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

bool ObjectVisitor::visitIfcPresentationStyleAssignment(ifc2x3::IfcPresentationStyleAssignment* value)
{
	if (value->testStyles())
	{
		for (auto styles : value->getStyles())
		{
			switch (styles->currentType())
			{
			case ifc2x3::IfcPresentationStyleSelect::IFCCURVESTYLE:
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

bool ObjectVisitor::visitIfcSurfaceStyle(ifc2x3::IfcSurfaceStyle* value)
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

bool ObjectVisitor::visitIfcSurfaceStyleRendering(ifc2x3::IfcSurfaceStyleRendering* value)
{
	if (value->testSurfaceColour())
	{
		value->getSurfaceColour()->acceptVisitor(this);
	}

	if (value->testTransparency())
	{
		m_Color.Alpha = value->getTransparency();
	}

	m_Style.Styles.push_back(m_Color);

	return true;
}

bool ObjectVisitor::visitIfcColourRgb(ifc2x3::IfcColourRgb* value)
{
	if (value->testRed())
	{
		m_Color.Red = value->getRed();
	}
	if (value->testGreen())
	{
		m_Color.Green = value->getGreen();
	}
	if (value->testBlue())
	{
		m_Color.Blue = value->getBlue();
	}
	return true;
}

bool ObjectVisitor::visitIfcIShapeProfileDef(ifc2x3::IfcIShapeProfileDef* value)
{
	std::shared_ptr<I_profilDef> profilDef = std::make_shared<I_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->OverallWidth = (double)value->getOverallWidth();
	profilDef->OverallDepth = (double)value->getOverallDepth();
	profilDef->WebThickness = (double)value->getWebThickness();
	profilDef->FlangeThickness = (double)value->getFlangeThickness();
	profilDef->FilletRadius = (double)value->getFilletRadius();
	profilDef->nbArg = 5;

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcLShapeProfileDef(ifc2x3::IfcLShapeProfileDef* value)
{
	std::shared_ptr<L_profilDef> profilDef = std::make_shared<L_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->Depth = (double)value->getDepth();
	profilDef->Width = (double)value->getWidth();
	profilDef->Thickness = (double)value->getThickness();
	profilDef->FilletRadius = (double)value->getFilletRadius();
	profilDef->EdgeRadius = (double)value->getEdgeRadius();
	profilDef->nbArg = 5;

	if (value->testLegSlope())
	{
		profilDef->LegSlope = (double)value->getLegSlope();
		profilDef->nbArg = 6;
	}

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcTShapeProfileDef(ifc2x3::IfcTShapeProfileDef* value)
{
	std::shared_ptr<T_profilDef> profilDef = std::make_shared<T_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->Depth = (double)value->getDepth();
	profilDef->FlangeWidth = (double)value->getFlangeWidth();
	profilDef->WebThickness = (double)value->getWebThickness();
	profilDef->FlangeThickness = (double)value->getFlangeThickness();
	profilDef->FilletRadius = (double)value->getFilletRadius();
	profilDef->FlangeEdgeRadius = (double)value->getFlangeEdgeRadius();
	profilDef->WebEdgeRadius = (double)value->getWebEdgeRadius();
	profilDef->nbArg = 7;

	if (value->testWebSlope() && value->testFlangeSlope())
	{
		profilDef->WebSlope = (double)value->getWebSlope();
		profilDef->FlangeSlope = (double)value->getFlangeSlope();
		profilDef->nbArg = 9;
	}

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcUShapeProfileDef(ifc2x3::IfcUShapeProfileDef* value)
{
	std::shared_ptr<U_profilDef> profilDef = std::make_shared<U_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->Depth = (double)value->getDepth();
	profilDef->FlangeWidth = (double)value->getFlangeWidth();
	profilDef->WebThickness = (double)value->getWebThickness();
	profilDef->FlangeThickness = (double)value->getFlangeThickness();
	profilDef->FilletRadius = (double)value->getFilletRadius();
	profilDef->nbArg = 5;

	if (value->testEdgeRadius() && value->testFlangeSlope())
	{
		profilDef->EdgeRadius = (double)value->getEdgeRadius();
		profilDef->FlangeSlope = (double)value->getFlangeSlope();
		profilDef->nbArg = 7;
	}

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcCShapeProfileDef(ifc2x3::IfcCShapeProfileDef* value)
{
	std::shared_ptr<C_profilDef> profilDef = std::make_shared<C_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->Depth = (double)value->getDepth();
	profilDef->Width = (double)value->getWidth();
	profilDef->WallThickness = (double)value->getWallThickness();
	profilDef->Girth = (double)value->getGirth();
	profilDef->InternalFilletRadius = (double)value->getInternalFilletRadius();

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcZShapeProfileDef(ifc2x3::IfcZShapeProfileDef* value)
{
	std::shared_ptr<Z_profilDef> profilDef = std::make_shared<Z_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->Depth = (double)value->getDepth();
	profilDef->FlangeWidth = (double)value->getFlangeWidth();
	profilDef->WebThickness = (double)value->getWebThickness();
	profilDef->FlangeThickness = (double)value->getFlangeThickness();
	profilDef->FilletRadius = (double)value->getFilletRadius();
	profilDef->EdgeRadius = (double)value->getEdgeRadius();

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcAsymmetricIShapeProfileDef(ifc2x3::IfcAsymmetricIShapeProfileDef* value)
{
	std::shared_ptr<AsymmetricI_profilDef> profilDef = std::make_shared<AsymmetricI_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->OverallWidth = (double)value->getOverallWidth();
	profilDef->OverallDepth = (double)value->getOverallDepth();
	profilDef->WebThickness = (double)value->getWebThickness();
	profilDef->FlangeThickness = (double)value->getFlangeThickness();
	profilDef->FlangeFilletRadius = (double)value->getTopFlangeFilletRadius();
	profilDef->TopFlangeWidth = (double)value->getTopFlangeWidth();
	profilDef->TopFlangeThickness = (double)value->getTopFlangeThickness();
	profilDef->TopFlangeFilletRadius = (double)value->getTopFlangeFilletRadius();

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcCircleHollowProfileDef(ifc2x3::IfcCircleHollowProfileDef* value)
{
	std::shared_ptr<CircleHollow_profilDef> profilDef = std::make_shared<CircleHollow_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->Radius = (double)value->getRadius();
	profilDef->WallThickness = (double)value->getWallThickness();

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcRectangleHollowProfileDef(ifc2x3::IfcRectangleHollowProfileDef* value)
{
	std::shared_ptr<RectangleHollow_profilDef> profilDef = std::make_shared<RectangleHollow_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->XDim = (double)value->getXDim();
	profilDef->YDim = (double)value->getYDim();
	profilDef->WallThickness = (float)value->getWallThickness();
	profilDef->InnerFilletRadius = (float)value->getInnerFilletRadius();
	profilDef->OuteerFilletRadius = (float)value->getOuterFilletRadius();

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcRectangleProfileDef(ifc2x3::IfcRectangleProfileDef* value)
{
	std::shared_ptr<Rectangle_profilDef> profilDef = std::make_shared<Rectangle_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->XDim = (double)value->getXDim();
	profilDef->YDim = (double)value->getYDim();

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcCircleProfileDef(ifc2x3::IfcCircleProfileDef* value)
{
	std::shared_ptr<Circle_profilDef> profilDef = std::make_shared<Circle_profilDef>();
	profilDef->Entity = m_IfcObject->Entity;
	profilDef->Key = m_IfcShapeRepresentation.Key;

	if (value->testPosition())
	{
		profilDef->Transformation2D = ComputePlacementVisitor::getTransformation2D(value->getPosition());
	}

	profilDef->Radius = (float)value->getRadius();

	m_IfcShapeRepresentation.ProfilDef = profilDef;

	return true;
}

bool ObjectVisitor::visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value)
{
	m_IfcShapeRepresentation.OuterCurveName = value->getOuterCurve()->getType().getName();

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
	m_IfcShapeRepresentation.EntityType = value->type();
	m_IfcShapeRepresentation.Key = value->getKey();

	if (value->testOuter())
	{
		return value->getOuter()->acceptVisitor(this);
	}

	return false;
}

bool ObjectVisitor::visitIfcClosedShell(ifc2x3::IfcClosedShell* value)
{
	m_Face.Key = value->getKey();

	if (value->testCfsFaces())
	{
		for (auto face : value->getCfsFaces())
		{
			m_Face.SupType = value->type();

			if (!face->acceptVisitor(this))
			{
				return false;
			}
		}
	}

	return true;
}

bool ObjectVisitor::visitIfcFace(ifc2x3::IfcFace* value)
{
	m_Face.FaceKey = value->getKey();

	if (value->testBounds())
	{
		m_VisitingFaces = true;

		for (auto bound : value->getBounds())
		{
			if (!bound->acceptVisitor(this))
			{
				m_VisitingFaces = false;

				return false;
			}
		}
	}
	m_VisitingFaces = false;

	return true;
}

bool ObjectVisitor::visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value)
{
	m_Face.Type = value->type();
	m_Face.Points.clear();

	if (value->testBound())
	{
		m_Face.Orientation = value->getOrientation();
		value->getBound()->acceptVisitor(this);
	}

	m_IfcShapeRepresentation.IfcFaces.push_back(m_Face);

	return true;
}

bool ObjectVisitor::visitIfcFaceBound(ifc2x3::IfcFaceBound* value)
{
	m_Face.Type = value->type();
	m_Face.Points.clear();

	if (value->testBound())
	{
		m_Face.Orientation = value->getOrientation();
		value->getBound()->acceptVisitor(this);
	}

	m_IfcShapeRepresentation.IfcFaces.push_back(m_Face);

	return true;
}

bool ObjectVisitor::visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value)
{
	for (auto point : value->getPolygon())
	{
		if (!m_VisitingFaces)
			m_IfcShapeRepresentation.Points.push_back(ComputePlacementVisitor::getPoint(point.get()));
		else
			m_Face.Points.push_back(ComputePlacementVisitor::getPoint(point.get()));
	}

	return m_IfcShapeRepresentation.Points.empty() == false;
}

std::shared_ptr<IFCObject> ObjectVisitor::getIfcObject()
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

IFCShapeRepresentation& ObjectVisitor::getShapeRepresentation()
{
	return m_IfcShapeRepresentation;
}

std::vector<IFCShapeRepresentation>& ObjectVisitor::getShapeRepresentations()
{
	return m_ShapeRepresentations;
}