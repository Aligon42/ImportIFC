#include "Construction.h"

std::map<int, ObjectVoid> Construction::s_ObjectVoids;

Construction::Construction(IFCObjTemp* ifcObject)
	: m_IfcObject(ifcObject) 
{
	InitLayerTable();
	AcGeContext::gTol.setEqualPoint(0.001);
	AcGeContext::gTol.setEqualVector(0.001);
}

void Construction::Extrusion()
{
	const ACHAR* layerName = GetLayerName();
	AddTableRecord(layerName);
	DrawExtrusion(layerName);
}

Acad::ErrorStatus Construction::InitLayerTable()
{
	m_Database = acdbHostApplicationServices()->workingDatabase();
	return m_Database->getLayerTable(m_LayerTable, AcDb::kForRead);
}

Acad::ErrorStatus Construction::AddTableRecord(const ACHAR* layerName)
{
	Acad::ErrorStatus es;

	if (!m_LayerTable->has(layerName))
	{
		// Open the Layer table for write
		es = m_LayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		m_LayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		es = pLayerTableRecord->close();
	}

	es = m_LayerTable->close();

	return es;
}

const ACHAR* Construction::GetLayerName()
{
	const std::string& entity = m_IfcObject->Entity;
	std::string outLayerName = entity;

	if (entity == "IfcWallStandardCase")
	{
		outLayerName = "Mur";
	}
	if (entity == "IfcWall")
	{
		outLayerName = "Mur";
	}
	if (entity == "IfcSlab")
	{
		outLayerName = "Dalle";
	}
	if (entity == "IfcRoof")
	{
		outLayerName = "Toit";
	}
	if (entity == "IfcCovering")
	{
		outLayerName = "Revêtement";
	}
	if (entity == "IfcPlate")
	{
		outLayerName = "Plaque";
	}
	if (entity == "IfcFooting")
	{
		outLayerName = "Pied";
	}
	if (entity == "IfcMappedItem")
	{
		outLayerName = "Element";
	}

	return ConvertToWideChar(outLayerName.c_str());
}

void Construction::SetColor(AcDb3dSolid* solid)
{
	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(m_Style.red * 255, m_Style.green * 255, m_Style.blue * 255);
	solid->setColor(couleurRGB, false);

	double opa = abs((m_Style.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	solid->setTransparency(transparence);
}

void Construction::DrawExtrusion(const ACHAR* layerName)
{
	Acad::ErrorStatus es;
	AcDbRegion* pRegion = nullptr;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	IFCShapeRepresentation& shapeRepresentations = *m_IfcObject->ShapeRepresentations.begin();

	if (m_IfcObject->OuterCurveName == "IfcCompositeCurve")
	{
		pRegion = CreateCompositeCurve(shapeRepresentations.CompositeCurve, m_IfcObject->Transformation);
	}
	else if (m_IfcObject->OuterCurveName == "IfcPolyline")
	{
		ptArr.setLogicalLength(shapeRepresentations.Points.size());
		Vec3 pointOrigine = { m_IfcObject->Transformation[12], m_IfcObject->Transformation[13] , m_IfcObject->Transformation[14] };

		int i = 0;

		for (const auto& point : shapeRepresentations.Points)
		{
			ptArr[i].set(point.x(), point.y(), point.z());
			i++;
		}

		AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines);
		AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
		if (pRegion == nullptr) return;
	}

	AcGeVector3d vecExtru = GetExtrusionVector(shapeRepresentations.ExtrusionVector,shapeRepresentations.ExtrusionHeight);

	AcDbSweepOptions options;
	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	es = pSolid->createExtrudedSolid(pRegion, vecExtru, options);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	DeplacementObjet2D(pSolid, m_IfcObject->Transformation2D);
	DeplacementObjet3D(pSolid, m_IfcObject->Transformation);

	for (int a = 1; a < m_IfcObject->ShapeRepresentations.size(); a++)
	{
		CreationSection(pSolid, m_IfcObject->ShapeRepresentations[a]);
	}

	HandleDeplacements(pSolid);
	DrawVoids(pSolid);
	SetColor(pSolid);
	DrawElement(layerName, pSolid, es);
}

const wchar_t* Construction::ConvertToWideChar(const char* c, ...)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

AcDbRegion* Construction::CreateCompositeCurve(const std::vector<CompositeCurveSegment>& compositeCurve, const Matrix4& transform)
{
	AcDbVoidPtrArray lines;
	Acad::ErrorStatus es;
	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray regions;

	for (const auto& compositeCurveSegment : compositeCurve)
	{
		ptArr.setLogicalLength(compositeCurveSegment.PointsPolyligne.size());
		Vec3 pointOrigine = { transform[12], transform[13] , transform[14] };

		int i = 0;
		for (const auto& point : compositeCurveSegment.PointsPolyligne)
		{
			ptArr[i].set(point.x(), point.y(), point.z());
			i++;
		}

		AcDb2dPolyline* pNewPline = new AcDb2dPolyline(AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kFalse);
		pNewPline->setColorIndex(3);

		//get the boundary curves of the polyline
		AcDbEntity* pEntity = NULL;
		if (pNewPline == NULL)
		{
			pEntity->close();
			return nullptr;
		}

		lines.append((pNewPline));
		pNewPline->close();
	}

	//Arc
	AcDbArc* arc = new AcDbArc();
	for (const auto& compositeCurveSegment : compositeCurve)
	{
		float pointX = compositeCurveSegment.TrimmedCurves->centreCircle.x();
		float pointY = compositeCurveSegment.TrimmedCurves->centreCircle.y();
		float radius = compositeCurveSegment.TrimmedCurves->Radius;

		AcGePoint3d center = AcGePoint3d::AcGePoint3d(pointX, pointY, 0);
		arc->setCenter(center);
		arc->setRadius(compositeCurveSegment.TrimmedCurves->Radius);

		if (compositeCurveSegment.TrimmedCurves->SenseAgreement)
		{
			arc->setStartAngle((compositeCurveSegment.TrimmedCurves->Trim1 * PI) / 180);
			arc->setEndAngle((compositeCurveSegment.TrimmedCurves->Trim2 * PI) / 180);
		}
		else {
			arc->setStartAngle(-(compositeCurveSegment.TrimmedCurves->Trim1 * PI) / 180);
			arc->setEndAngle(-(compositeCurveSegment.TrimmedCurves->Trim2 * PI) / 180);
		}

		lines.append(arc);
		arc->close();
	}

	// Create a region from the set of lines.
	AcDbRegion* pRegion = CreateRegion(lines, regions);

	return pRegion;
}

void Construction::DrawVoids(AcDb3dSolid* solid)
{
	if (s_ObjectVoids.find(m_IfcObject->Key) != s_ObjectVoids.end())
	{
		if (s_ObjectVoids[m_IfcObject->Key].NameProfilDef == "IfcArbitraryClosedProfileDef")
		{
			CreationVoid(solid, s_ObjectVoids[m_IfcObject->Key]);
			//listVoid.erase(listVoid.begin() + v);
		}
		else if (s_ObjectVoids[m_IfcObject->Key].NameProfilDef == "IfcCircleProfileDef")
		{
			CreationVoidCircle(solid, s_ObjectVoids[m_IfcObject->Key]);
			//listVoid.erase(listVoid.begin() + v);
		}
		else if (s_ObjectVoids[m_IfcObject->Key].NameProfilDef == "IfcRectangleProfileDef")
		{
			CreationVoidRectangle(solid, s_ObjectVoids[m_IfcObject->Key]);
			//listVoid.erase(listVoid.begin() + v);
		}
	}
}

void Construction::HandleDeplacements(AcDb3dSolid* solid, bool move2D)
{
	if (m_IfcObject->IsMappedItem)
		DeplacementObjet3DMappedItem(solid, m_IfcObject->TransformationOperator3D);

	DeplacementObjet3D(solid, m_IfcObject->LocalTransform);

	if (move2D)
	DeplacementObjet2D(solid, m_IfcObject->Transformation2D);
}

void Construction::DrawElement(const ACHAR* layerName, AcDb3dSolid* pSolid, Acad::ErrorStatus es)
{
	pSolid->setLayer(layerName, Adesk::kFalse, false);
	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	if (es == Acad::eOk)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		pBlockTableRecord->appendAcDbEntity(pSolid);
		pBlockTableRecord->close();
		pSolid->close();
	}
	else
	{
		delete pSolid;
		acutPrintf(_T("Je ne fais rien du tout"));
		return;
	}
}

void Construction::CreationVoid(AcDb3dSolid* extrusion, ObjectVoid& objectVoid)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr; 
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	if (objectVoid.Args.size() > 1)
	{
		objectVoid.Args.erase(objectVoid.Args.begin());
		objectVoid.Points.pop_front();
	}

	ptArr.setLogicalLength(objectVoid.Args[0]);
	Vec3 pointOrigine = { objectVoid.Transform[12], objectVoid.Transform[13] , objectVoid.Transform[14] };

	int i = 0;

	for (const auto& point : objectVoid.Points)
	{
		if (i == objectVoid.Args[0]) break;

		ptArr[i].set(point.x(), point.y(), point.z());
		i++;
	}

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);

	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(objectVoid.VecteurExtrusion, objectVoid.HauteurExtrusion);
	AcDbSweepOptions options;

	// Extrude the region to create a solid.
	AcDb3dSolid* extrusion_void = new AcDb3dSolid();
	AcDb3dSolid* extrusion_void2 = new AcDb3dSolid();
	es = extrusion_void->createExtrudedSolid(pRegion, vecExtru, options);
	es = extrusion_void2->createExtrudedSolid(pRegion, -vecExtru, options);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	for (int a = 1; a < objectVoid.ShapeRepresentations.size(); a++)
	{
		for (size_t i = 0; i < objectVoid.Args[0]; i++)
		{
			objectVoid.Points.pop_front();
		}

		objectVoid.Args.erase(objectVoid.Args.begin());

		DeplacementObjet2D(extrusion_void, objectVoid.Transform2D);
		DeplacementObjet2D(extrusion_void2, objectVoid.Transform2D);

		if (objectVoid.Points.size() > 0 && objectVoid.Args.size() > 0)
		{
			CreationSection(extrusion_void, objectVoid.ShapeRepresentations[a]);
			CreationSection(extrusion_void2, objectVoid.ShapeRepresentations[a]);
		}
	}

	HandleDeplacements(extrusion_void);
	HandleDeplacements(extrusion_void2);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	AcDbDatabase* pDb = curDoc()->database();
	AcDbObjectId modelId;
	modelId = acdbSymUtil()->blockModelSpaceId(pDb);
	AcDbBlockTableRecord* pBlockTableRecord;
	acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
	pBlockTableRecord->appendAcDbEntity(extrusion_void);
	pBlockTableRecord->appendAcDbEntity(extrusion_void2);
	pBlockTableRecord->close();

	extrusion->booleanOper(AcDb::kBoolSubtract, extrusion_void);
	extrusion->booleanOper(AcDb::kBoolSubtract, extrusion_void2);
	extrusion_void->close();
	extrusion_void2->close();
}

void Construction::CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid)
{

}

void Construction::CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid)
{

}

void Construction::CreationSection(AcDb3dSolid* extrusion, IFCShapeRepresentation& shapeRepresentation)
{
	AcGeVector3d v1 = AcGeVector3d::AcGeVector3d(0, 0, 0);             // Vector 1 (x,y,z) & Vector 2 (x,y,z)
	AcGeVector3d v2 = AcGeVector3d::AcGeVector3d(0, 0, 0);
	AcGeVector3d normal = AcGeVector3d::AcGeVector3d(0, 0, 0);
	double p0x, p0y, p0z;
	AcGePoint3d p0 = AcGePoint3d::AcGePoint3d();
	double p1x, p1y,  p1z;
	AcGeVector3d V1 = AcGeVector3d::AcGeVector3d();
	AcGeVector3d V2 = AcGeVector3d::AcGeVector3d();
	AcGePoint3d p3 = AcGePoint3d::AcGePoint3d();
	double p2x, p2y, p2z;
	double p1xx, p1yy, p1zz, p2xx, p2yy, p2zz;
	AcGePoint3d p1 = AcGePoint3d::AcGePoint3d();
	AcGePoint3d p2 = AcGePoint3d::AcGePoint3d();
	AcGeVector3d v3d = AcGeVector3d::AcGeVector3d();
	AcGePoint3d p33 = AcGePoint3d::AcGePoint3d();
	AcGePoint3d p5 = AcGePoint3d::AcGePoint3d();

	if (shapeRepresentation.EntityType == "IfcHalfSpaceSolid" ||
		shapeRepresentation.EntityType == "IfcPolygonalBoundedHalfSpace")
	{
		//Coordonnées du repère du plan
		p0x = shapeRepresentation.Plan[12];  //x
		p0y = shapeRepresentation.Plan[13]; //y
		p0z = shapeRepresentation.Plan[14]; //z

		p0 = AcGePoint3d::AcGePoint3d(p0x, p0y, p0z);

		//Direction1 plan
		p1x = shapeRepresentation.Plan[8];  //x
		p1y = shapeRepresentation.Plan[9]; //y
		p1z = shapeRepresentation.Plan[10]; //z

		V1 = AcGeVector3d::AcGeVector3d(p1x, p1y, p1z);
		V2 = V1.normal();
		p3 = AcGePoint3d::AcGePoint3d(V2.x + p0x, V2.y + p0y, V2.z + p0z);

		//Direction2 plan
		p2x = shapeRepresentation.Plan[0];  //x
		p2y = shapeRepresentation.Plan[1]; //y
		p2z = shapeRepresentation.Plan[2]; //z

		p1xx = p0x + p1x;
		p1yy = p0x + p1x;
		p1zz = p0x + p1x;

		p1xx = (p1y * p2z) - (p1z * p2y);
		p1yy = (p1z * p2x) - (p1x * p2z);
		p1zz = (p1x * p2y) - (p1y * p2x);

		p1 = AcGePoint3d::AcGePoint3d(p1x + p0x, p1y + p0y, p1z + p0z);
		p2 = AcGePoint3d::AcGePoint3d(p2x + p0x, p2y + p0y, p2z + p0z);

		AcDbLine* line = new AcDbLine(p0, p1);
		v3d = line->normal();

		p33 = AcGePoint3d::AcGePoint3d(v3d.x, v3d.y, v3d.z);
		p5 = AcGePoint3d::AcGePoint3d(p1xx + p0x, p1yy + p0y, p1zz + p0z);
	}

	if (shapeRepresentation.EntityType == "IfcHalfSpaceSolid")
	{
		AcGePlane Poly_plane = AcGePlane::AcGePlane(p0, p5, p2);
		bool agreementHalf = StepBoolToBoolean(shapeRepresentation.AgreementHalf);
		if (agreementHalf == false)
		{
			Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal().negate());
		}
		else
		{
			Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal());
		}

		extrusion->getSlice(Poly_plane, agreementHalf, extrusion);
	}
	else if (shapeRepresentation.EntityType == "IfcPolygonalBoundedHalfSpace")
	{
		AcGePlane Poly_plane = AcGePlane::AcGePlane(p0, p5, p2);
		bool agreementPoly = !StepBoolToBoolean(shapeRepresentation.AgreementPolygonal);
		if (agreementPoly)
		{
			Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal().negate());
		}
		else
		{
			Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal());
		}

		Acad::ErrorStatus es;
		AcGePoint3dArray ptArr;
		AcDbVoidPtrArray lines;
		AcDbVoidPtrArray regions;
		ptArr.setLogicalLength(shapeRepresentation.Points.size());

		int i = 0;
		for (const auto& point : shapeRepresentation.Points)
		{
			ptArr[i].set(point.x(), point.y(), point.z());
			i++;
		}

		AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines);
		AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
		if (pRegion == nullptr) return;

		if (shapeRepresentation.CompositeCurve.size() > 0)
		{
			const CompositeCurveSegment& compositeCurveSegment = *shapeRepresentation.CompositeCurve.begin();

			if (compositeCurveSegment.PointsPolyligne.size() > 0 || compositeCurveSegment.TrimmedCurves != nullptr)
			{
				pRegion = CreateCompositeCurve(shapeRepresentation.CompositeCurve, m_IfcObject->Transformation);
			}
			else
				pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);
		}
		else
			pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);

		AcGeVector3d vecExtru = GetExtrusionVector(shapeRepresentation.ExtrusionVector, shapeRepresentation.ExtrusionHeight);
		AcDbSweepOptions options;
		// Extrude the region to create a solid.
		AcDb3dSolid* pSolid = new AcDb3dSolid();
		AcDb3dSolid* pSolid2 = new AcDb3dSolid();
		es = pSolid->createExtrudedSolid(pRegion, vecExtru, options);
		es = pSolid2->createExtrudedSolid(pRegion, -vecExtru, options);

		pSolid->booleanOper(AcDb::kBoolUnite, pSolid2);
		for (int i = 0; i < lines.length(); i++)
		{
			delete (AcRxObject*)lines[i];
		}
		for (int ii = 0; ii < regions.length(); ii++)
		{
			delete (AcRxObject*)regions[ii];
		}

		pSolid->close();

		if (m_IfcObject->IsMappedItem)
			DeplacementObjet3DMappedItem(pSolid, m_IfcObject->TransformationOperator3D);

		DeplacementObjet3D(pSolid, shapeRepresentation.LocationPolygonal);

		AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

		int nbPolylineComposite = 0;

		for (auto& curve : shapeRepresentation.CompositeCurve)
			nbPolylineComposite += curve.PointsPolyligne.size() > 0 ? 1 : 0;

		pSolid->getSlice(Poly_plane, nbPolylineComposite > 0 ? !agreementPoly : agreementPoly, pSolid);
		extrusion->booleanOper(AcDb::kBoolSubtract, pSolid);
	}
	else if (shapeRepresentation.EntityType == "IfcExtrudedAreaSolid")
	{
		if (shapeRepresentation.ProfilDefName == "IfcArbitraryClosedProfileDef")
		{
			Acad::ErrorStatus es;
			ads_name polyName;
			ads_point ptres;
			AcGePoint3dArray ptArr;
			AcDbVoidPtrArray lines;
			AcDbVoidPtrArray regions;

			ptArr.setLogicalLength(shapeRepresentation.Points.size());

			int i = 0;

			for (const auto& point : shapeRepresentation.Points)
			{
				ptArr[i].set(point.x(), point.y(), point.z());
				i++;
			}

			AcDb2dPolyline* pNewLine = CreatePolyline(ptArr, lines);
			AcDbRegion* pRegion = CreateRegion(pNewLine, lines, regions);

			AcGeVector3d vecExtru = GetExtrusionVector(shapeRepresentation.ExtrusionVector, shapeRepresentation.ExtrusionHeight);
			AcDbSweepOptions options;

			// Extrude the region to create a solid.
			AcDb3dSolid* extrusionBool = new AcDb3dSolid();
			es = extrusionBool->createExtrudedSolid(pRegion, vecExtru, options);

			for (int i = 0; i < lines.length(); i++)
			{
				delete (AcRxObject*)lines[i];
			}
			for (int ii = 0; ii < regions.length(); ii++)
			{
				delete (AcRxObject*)regions[ii];
			}

			DeplacementObjet3D(extrusionBool, shapeRepresentation.TransformBoolean);

			AcDbDatabase* pDb = curDoc()->database();
			AcDbObjectId modelId  = acdbSymUtil()->blockModelSpaceId(pDb);
			AcDbBlockTableRecord* pBlockTableRecord;
			acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
			pBlockTableRecord->appendAcDbEntity(extrusionBool);
			pBlockTableRecord->close();

			extrusion->booleanOper(AcDb::kBoolSubtract, extrusionBool);
			extrusionBool->close();
		}
		else if (shapeRepresentation.ProfilDefName == "IfcRectangleProfileDef")
		{
			Acad::ErrorStatus es;
			ads_name polyName;
			ads_point ptres;

			AcGePoint3dArray ptArr1;
			AcGePoint3dArray ptArr2;

			Rectangle_profilDef* rectProfilDef = dynamic_cast<Rectangle_profilDef*>(shapeRepresentation.ProfilDef);

			float XDim = rectProfilDef->XDim;
			float YDim = rectProfilDef->YDim;

			/// <summary>
			/// Première polyline
			/// </summary>
			ptArr1.setLogicalLength(4);

			ptArr1[0].set(0, 0, 0.0);
			ptArr1[1].set(XDim, 0, 0.0);
			ptArr1[2].set(XDim, YDim, 0.0);
			ptArr1[3].set(0, YDim, 0.0);

			AcDb2dPolyline* pNewPline1 = new AcDb2dPolyline(
				AcDb::k2dSimplePoly, ptArr1, 0.0, Adesk::kTrue);
			pNewPline1->setColorIndex(3);

			AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
			AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
			AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-XDim / 2, -YDim / 2, 0);
			AcGeVector3d acVec3d = pointDeplacement3D.asVector();
			pNewPline1->transformBy(matrix3d.translation(acVec3d));

			//get the boundary curves of the polyline
			AcDbEntity* pEntity1 = NULL;

			if (pNewPline1 == NULL)
			{
				pEntity1->close();
				return;
			}
			AcDbVoidPtrArray lines1;
			pNewPline1->explode(lines1);
			pNewPline1->close();

			// Create a region from the set of lines.
			AcDbVoidPtrArray regions1;
			AcDbRegion* pRegion1 = CreateRegion(pNewPline1, lines1, regions1);

			AcGeVector3d vecExtru = GetExtrusionVector(shapeRepresentation.ExtrusionVector, shapeRepresentation.ExtrusionHeight);
			AcDbSweepOptions options;
			// Extrude the region to create a solid.
			AcDb3dSolid* pSolid = new AcDb3dSolid();
			es = pSolid->createExtrudedSolid(pRegion1, vecExtru, options);

			for (int i = 0; i < lines1.length(); i++)
			{
				delete (AcRxObject*)lines1[i];
			}
			for (int ii = 0; ii < regions1.length(); ii++)
			{
				delete (AcRxObject*)regions1[ii];
			}

			extrusion->booleanOper(AcDb::kBoolSubtract, pSolid);
			pSolid->close();
		}
	}
}

AcDbRegion* Construction::CreateRegion(AcDb2dPolyline* pNewPline, AcDbVoidPtrArray lines, AcDbVoidPtrArray& regions)
{
	Acad::ErrorStatus es;

	es = AcDbRegion::createFromCurves(lines, regions);
	if (es != Acad::eOk)
	{
		pNewPline->close();
		acutPrintf(L"\nFailed to create region\n");
		return nullptr;
	}

	return AcDbRegion::cast((AcRxObject*)regions[0]);
}

AcDbRegion* Construction::CreateRegion(AcDbVoidPtrArray lines, AcDbVoidPtrArray& regions)
{
	Acad::ErrorStatus es;

	es = AcDbRegion::createFromCurves(lines, regions);
	if (es != Acad::eOk)
	{
		acutPrintf(L"\nFailed to create region\n");
		return nullptr;
	}

	return AcDbRegion::cast((AcRxObject*)regions[0]);
}

AcGeVector3d Construction::GetExtrusionVector(const Vec3& vector, double height)
{
	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d(vector.x() * height, vector.y() * height, vector.z() * height);

	return vecExtru;
}

AcDb2dPolyline* Construction::CreatePolyline(AcGePoint3dArray& ptArr, AcDbVoidPtrArray& lines)
{
	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;
	if (pNewPline == NULL)
	{
		pEntity->close();
		return;
	}

	pNewPline->explode(lines);
	pNewPline->close();

	return pNewPline;
}

void Construction::DeplacementObjet3DMappedItem(AcDb3dSolid* solid, const Matrix4& transform)
{
	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);
	AcGePoint3d srcpt4 = AcGePoint3d::AcGePoint3d(0, 1, 0);

	double x1 = transform(3);	//PointDeplacement x
	double y1 = transform(7);	//PointDeplacement y
	double z1 = transform(11); //PointDeplacement z
	double x2 = transform(0);	//Direction1 x
	double y2 = transform(4);	//Direction1 y
	double z2 = transform(8);	//Direction1 z
	double x3 = transform(1);	//Direction2 x
	double y3 = transform(5);	//Direction2 y
	double z3 = transform(9);	//Direction2 z
	double x4 = transform(2);	//Direction3 x
	double y4 = transform(6);	//Direction3 y
	double z4 = transform(10); //Direction3 z

	// 3 destination points
	AcGePoint3d destpt1 = AcGePoint3d::AcGePoint3d(x1, y1, z1);
	AcGePoint3d destpt2 = AcGePoint3d::AcGePoint3d(x1 + x2, y1 + y2, z1 + z2);
	AcGePoint3d destpt3 = AcGePoint3d::AcGePoint3d(x1 + x3, y1 + y3, z1 + z3);
	AcGePoint3d destpt4 = AcGePoint3d::AcGePoint3d(x1 + x4, y1 + y4, z1 + z4);

	AcGePoint3d fromOrigin = srcpt1;
	AcGeVector3d fromXaxis = srcpt2 - srcpt1;
	AcGeVector3d fromYaxis = srcpt3 - srcpt1;
	AcGeVector3d fromZaxis = srcpt4 - srcpt1;

	AcGePoint3d toOrigin = destpt1;
	AcGeVector3d toXaxis = (destpt2 - destpt1).normal() * (fromXaxis.length());
	AcGeVector3d toYaxis = (destpt3 - destpt1).normal() * (fromYaxis.length());
	AcGeVector3d toZaxis = (destpt4 - destpt1).normal() * (fromZaxis.length());

	// Get the transformation matrix for aligning coordinate systems
	AcGeMatrix3d mat = AcGeMatrix3d::AcGeMatrix3d();
	mat = mat.alignCoordSys(fromOrigin, fromXaxis, fromYaxis, fromZaxis, toOrigin, toXaxis, toYaxis, toZaxis);

	solid->transformBy(mat);
}

void Construction::DeplacementObjet3D(AcDb3dSolid* solid, const Matrix4& transform)
{
	Acad::ErrorStatus es;
	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);

	double x1 = transform(12);  //PointDeplacement x
	double y1 = transform(13); //PointDeplacement y
	double z1 = transform(14); //PointDeplacement z
	double x2 = transform(8); //Direction1 x
	double y2 = transform(9); //Direction1 y
	double z2 = transform(10); //Direction1 z
	double x3 = transform(0); //Direction2 x
	double y3 = transform(1); //Direction2 y
	double z3 = transform(2); //Direction2 z

	// 3 destination points
	AcGePoint3d destpt1 = AcGePoint3d::AcGePoint3d(x1, y1, z1);
	AcGePoint3d destpt2 = AcGePoint3d::AcGePoint3d(x1 + x2, y1 + y2, z1 + z2);
	AcGePoint3d destpt3 = AcGePoint3d::AcGePoint3d(x1 + x3, y1 + y3, z1 + z3);

	AcGePoint3d fromOrigin = srcpt1;
	AcGeVector3d fromXaxis = srcpt2 - srcpt1;
	AcGeVector3d fromYaxis = srcpt3 - srcpt1;
	AcGeVector3d fromZaxis = fromXaxis.crossProduct(fromYaxis);

	AcGePoint3d toOrigin = destpt1;
	AcGeVector3d toXaxis = (destpt2 - destpt1).normal() * (fromXaxis.length());
	AcGeVector3d toYaxis = (destpt3 - destpt1).normal() * (fromYaxis.length());
	AcGeVector3d toZaxis = toXaxis.crossProduct(toYaxis);


	// Get the transformation matrix for aligning coordinate systems
	AcGeMatrix3d mat = AcGeMatrix3d::AcGeMatrix3d();
	mat = mat.alignCoordSys(fromOrigin, fromXaxis, fromYaxis, fromZaxis, toOrigin, toXaxis, toYaxis, toZaxis);

	es = solid->transformBy(mat);
}

void Construction::DeplacementObjet2D(AcDb3dSolid* solid, const Matrix4& transform)
{
	AcGeContext::gTol.setEqualPoint(0.001);
	AcGeContext::gTol.setEqualVector(0.001);

	Acad::ErrorStatus es;
	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);

	double x1 = transform(12);  //PointDeplacement x
	double y1 = transform(13); //PointDeplacement y
	double z1 = transform(14); //PointDeplacement z
	double x2 = transform(8); //Direction1 x
	double y2 = transform(9); //Direction1 y
	double z2 = transform(10); //Direction1 z
	double x3 = transform(0); //Direction2 x
	double y3 = transform(1); //Direction2 y
	double z3 = transform(2); //Direction2 z

	// 3 destination points
	AcGePoint3d destpt1 = AcGePoint3d::AcGePoint3d(x1, y1, z1);
	AcGePoint3d destpt2 = AcGePoint3d::AcGePoint3d(x1 + x2, y1 + y2, z1 + z2);
	AcGePoint3d destpt3 = AcGePoint3d::AcGePoint3d(x1 + x3, y1 + y3, z1 + z3);

	AcGePoint3d fromOrigin = srcpt1;
	AcGeVector3d fromXaxis = srcpt2 - srcpt1;
	AcGeVector3d fromYaxis = srcpt3 - srcpt1;
	AcGeVector3d fromZaxis = fromXaxis.crossProduct(fromYaxis);

	AcGePoint3d toOrigin = destpt1;
	AcGeVector3d toXaxis = (destpt2 - destpt1).normal() * (fromXaxis.length());
	AcGeVector3d toYaxis = (destpt3 - destpt1).normal() * (fromYaxis.length());
	AcGeVector3d toZaxis = toXaxis.crossProduct(toYaxis);

	// Get the transformation matrix for aligning coordinate systems
	AcGeMatrix3d mat = AcGeMatrix3d::AcGeMatrix3d();
	mat = mat.alignCoordSys(fromOrigin, fromXaxis, fromYaxis, fromZaxis, toOrigin, toXaxis, toYaxis, toZaxis);

	es = solid->transformBy(mat);
}
