#include "Autocad.h"

#include <adscodes.h>
#include <dbapserv.h>
#include <dbsol3d.h>
#include <dbsymutl.h>
#include <AcApDMgr.h>
#include <rxregsvc.h>
#include <dbregion.h>

std::string Autocad::s_LogPath;
std::map<std::string, std::vector<std::string>> Autocad::s_Logs;
std::map<int, std::vector<std::shared_ptr<IFCObject>>> Autocad::s_ObjectVoids;
std::map<int, Style> Autocad::s_Styles;

Timer::Timer(int key, const std::string& entity, const std::string& name)
	: m_Key(key), m_Entity(entity), m_Name(name)
{
	m_StartTime = std::chrono::high_resolution_clock::now();
}

Timer::~Timer()
{
	auto end = std::chrono::high_resolution_clock::now() - m_StartTime;
	std::stringstream ss;
	ss << "Key: " << m_Key << " - " << m_Entity << " - " << m_Name << " - " << std::chrono::duration_cast<std::chrono::microseconds>(end).count() / 1000.0 << " ms. \n";
	Autocad::s_Logs["Rendu"].push_back(ss.str());
}

Autocad::Autocad()
{
	m_StartTime = std::chrono::high_resolution_clock::now();
	InitLayerTable();
	AcGeContext::gTol.setEqualPoint(0.001);
	AcGeContext::gTol.setEqualVector(0.001);
}

Autocad::Autocad(std::shared_ptr<IFCObject> ifcObject)
	: m_IfcObject(ifcObject)
{
	m_StartTime = std::chrono::high_resolution_clock::now();
	InitLayerTable();
	AcGeContext::gTol.setEqualPoint(0.001);
	AcGeContext::gTol.setEqualVector(0.001);
}

Autocad::~Autocad()
{
	auto end = std::chrono::high_resolution_clock::now() - m_StartTime;
	std::stringstream ss;
	ss << "Key: " << m_IfcObject->Key << " - " << m_IfcObject->Entity << " - Rendu Autocad - " << std::chrono::duration_cast<std::chrono::microseconds>(end).count() / 1000.0 << " ms. \n";
	s_Logs["Rendu"].push_back(ss.str());
}

std::string Autocad::OpenAndSelectFile()
{
	AcDbObjectId transId;
	TCHAR* fname;

	struct resbuf* rb;
	// Get a ifc file from the user.
	//
	rb = acutNewRb(RTSTR);
	int stat = acedGetFileD(_T("Pick a IFC file"), NULL, _T("ifc"), 0, rb);
	if ((stat != RTNORM) || (rb == NULL))
	{
		acutPrintf(_T("\nYou must pick a ifc file."));
		return "";
	}
	fname = (TCHAR*)acad_malloc((_tcslen(rb->resval.rstring) + 1) * sizeof(TCHAR));
	_tcscpy(fname, rb->resval.rstring);
	acutRelRb(rb);

	return WideCharToString(fname);
}

void Autocad::Extrusion()
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	const ACHAR* layerName = GetLayerName(m_IfcObject->Entity);
	AddTableRecord(layerName);
	DrawExtrusion(layerName);
}

void Autocad::CreationFaceSolid()
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	const ACHAR* layerName = GetLayerName(m_IfcObject->Entity);
	AddTableRecord(layerName);
	DrawFaces(layerName);
}

Acad::ErrorStatus Autocad::InitLayerTable()
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	m_Database = acdbHostApplicationServices()->workingDatabase();
	return m_Database->getLayerTable(m_LayerTable, AcDb::kForRead);
}

Acad::ErrorStatus Autocad::AddTableRecord(const ACHAR* layerName)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

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

const ACHAR* Autocad::GetLayerName(std::string& entity)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	std::string outLayerName = entity;

	if (entity == "IfcBeam")
	{
		outLayerName = "Poutre";
	}
	else if (entity == "IfcColumn")
	{
		outLayerName = "Colonne";
	}
	else if (entity == "IfcWall" || entity == "IfcWallStandardCase")
	{
		outLayerName = "Mur";
	}
	else if (entity == "IfcSlab")
	{
		outLayerName = "Dalle";
	}
	else if (entity == "IfcRoof")
	{
		outLayerName = "Toit";
	}
	else if (entity == "IfcCovering")
	{
		outLayerName = "Revêtement";
	}
	else if (entity == "IfcPlate")
	{
		outLayerName = "Plaque";
	}
	else if (entity == "IfcFooting")
	{
		outLayerName = "Pied";
	}
	else if (entity == "IfcBuildingElementProxy")
	{
		outLayerName = "Proxy";
	}
	else if (entity == "IfcDoor")
	{
		outLayerName = "Porte";
	}
	else if (entity == "IfcSite")
	{
		outLayerName = "Site";
	}
	else if (entity == "IfcStair")
	{
		outLayerName = "Escalier";
	}
	else if (entity == "IfcRailling")
	{
		outLayerName = "Balustrade";
	}
	else if (entity == "IfcWindow")
	{
		outLayerName = "Fenêtre";
	}
	else if (entity == "IfcMappedItem")
	{
		outLayerName = "Element";
	}

	return ConvertToWideChar(outLayerName.c_str());
}

void Autocad::SetColor(AcDb3dSolid* solid, int colorIndex)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	if (colorIndex == 0) return;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();

	RGBA& color = *s_Styles[colorIndex].Styles.begin();
	couleurRGB.setRGB(color.Red * 255, color.Green * 255, color.Blue * 255);
	solid->setColor(couleurRGB, false);

	double opa = abs((color.Alpha * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	solid->setTransparency(transparence);
}

void Autocad::SetColor(AcDbSubDMesh* pSubDMesh, int colorIndex)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcCmColor couleurRGB = AcCmColor::AcCmColor();

	RGBA& color = *s_Styles[colorIndex].Styles.begin();
	couleurRGB.setRGB(color.Red * 255, color.Green * 255, color.Blue * 255);
	pSubDMesh->setColor(couleurRGB, false);

	double opa = abs((color.Alpha * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSubDMesh->setTransparency(transparence);
}

void Autocad::DrawExtrusion(const ACHAR* layerName)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	Acad::ErrorStatus es;
	AcDbRegion* pRegion = nullptr;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	IFCShapeRepresentation& shapeRepresentations = *m_IfcObject->ShapeRepresentations.begin();

	if (shapeRepresentations.OuterCurveName == "IfcCompositeCurve")
	{
		pRegion = CreateCompositeCurve(shapeRepresentations.CompositeCurve, shapeRepresentations.Transformation);
	}
	else if (shapeRepresentations.OuterCurveName == "IfcPolyline")
	{
		ptArr.setLogicalLength(shapeRepresentations.Points.size());
		Vec3 pointOrigine = { m_IfcObject->LocalTransform[12], m_IfcObject->LocalTransform[13] , m_IfcObject->LocalTransform[14] };

		int i = 0;

		for (const auto& point : shapeRepresentations.Points)
		{
			ptArr[i].set(point.x(), point.y(), point.z());
			i++;
		}

		AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines);
		pRegion = CreateRegion(pNewPline, lines, regions);
	}

	AcGeVector3d vecExtru = GetExtrusionVector(shapeRepresentations.ExtrusionVector, shapeRepresentations.ExtrusionHeight);

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

	if (m_IfcObject->ShapeRepresentations.size() > 1)
	{
		DeplacementObjet(pSolid, shapeRepresentations.Transformation2D);
		DeplacementObjet(pSolid, shapeRepresentations.Transformation);

		for (int a = 1; a < m_IfcObject->ShapeRepresentations.size(); a++)
		{
			CreationSection(pSolid, m_IfcObject->ShapeRepresentations[a]);
		}
	}

	HandleDeplacements(pSolid, shapeRepresentations, !(m_IfcObject->ShapeRepresentations.size() > 1));
	DrawVoids(pSolid);
	SetColor(pSolid, shapeRepresentations.Key);
	DrawElement(layerName, pSolid, es);
}

void Autocad::DrawFaces(const ACHAR* layerName)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	int k = 0;
	auto& shapeReps = m_IfcObject->ShapeRepresentations;

	if (m_IfcObject->IsMappedItem)
	{
		for (auto& parentShape : shapeReps)
		{
			for (auto& subShape : parentShape.SubShapeRepresentations)
			{
				DrawFacesMappedItem(subShape, parentShape);
			}
		}
	}
	else
	{
		Acad::ErrorStatus es = Acad::ErrorStatus::eOk;
		AcArray<Adesk::Int32> faceArray;
		AcDbSubDMesh* pSubDMesh = new AcDbSubDMesh();
		AcGePoint3dArray ptArr;
		AcArray<AcCmEntityColor> clrArray;
		AcCmEntityColor vColor;

		for (auto& shape : shapeReps)
		{
			for (auto& face : shape.IfcFaces)
			{
				RGBA& colorRGB = *s_Styles[shape.Key].Styles.begin();
				Adesk::RGBQuad color((((int)(colorRGB.Red * 255) & 0xff) << 16) + (((int)(colorRGB.Green * 255) & 0xff) << 8) + ((int)(colorRGB.Blue * 255) & 0xff));

				faceArray.append(face.Points.size());
				for (const auto& point : face.Points)
				{
					AcGePoint3d point3d = AcGePoint3d::AcGePoint3d(point.x(), point.y(), point.z());
					ptArr.append(point3d);

					vColor.setRGB(color);
					clrArray.append(vColor);
					faceArray.append(k++);
				}
			}

			es = pSubDMesh->setSubDMesh(ptArr, faceArray, 0);
			if (Acad::eOk != es)
			{
				pSubDMesh->close();
				acutPrintf(L"\nFailed to set \n");
				return;
			}

			if (shape.Scale > 0.0)
			{
				AcGeMatrix3d matrix = AcGeMatrix3d::AcGeMatrix3d();
				matrix.setToScaling(shape.Scale);
				pSubDMesh->transformBy(matrix);
			}

			HandleDeplacements(pSubDMesh, shape, false);
		}

		DrawElement(layerName, pSubDMesh, clrArray, es);
	}
}

void Autocad::DrawFacesMappedItem(IFCShapeRepresentation& shape, IFCShapeRepresentation& mappedShape)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	const ACHAR* layerName = GetLayerName(m_IfcObject->Entity);
	AddTableRecord(layerName);

	int k = 0;

	Acad::ErrorStatus es = Acad::ErrorStatus::eOk;
	AcArray<Adesk::Int32> faceArray;
	AcDbSubDMesh* pSubDMesh = new AcDbSubDMesh();
	AcGePoint3dArray ptArr;
	AcArray<AcCmEntityColor> clrArray;
	AcCmEntityColor vColor;

	for (auto& face : shape.IfcFaces)
	{
		//if (face.Key == 110749) continue;
		//if (face.Key == 110750) continue;
		//if (face.Key == 110751) continue;
		//if (face.Key == 110752) continue;
		//if (face.Key == 110753) continue;

		//if (face.Type == "IfcFaceBound") continue;

		RGBA& colorRGB = *s_Styles[shape.Key].Styles.begin();
		Adesk::RGBQuad color((((int)(colorRGB.Red * 255) & 0xff) << 16) + (((int)(colorRGB.Green * 255) & 0xff) << 8) + ((int)(colorRGB.Blue * 255) & 0xff));

		faceArray.append(face.Points.size());
		for (const auto& point : face.Points)
		{
			AcGePoint3d point3d = AcGePoint3d::AcGePoint3d(point.x(), point.y(), point.z());
			ptArr.append(point3d);

			vColor.setRGB(color);
			clrArray.append(vColor);
			faceArray.append(k++);
		}
	}

	es = pSubDMesh->setSubDMesh(ptArr, faceArray, 0);
	if (Acad::eOk != es)
	{
		pSubDMesh->close();
		acutPrintf(L"\nFailed to set \n");
		return;
	}

	if (shape.Scale > 0.0)
	{
		AcGeMatrix3d matrix = AcGeMatrix3d::AcGeMatrix3d();
		matrix.setToScaling(shape.Scale);
		pSubDMesh->transformBy(matrix);
	}

	HandleDeplacements(pSubDMesh, mappedShape, false);
	DrawElement(layerName, pSubDMesh, clrArray, es);
}

const wchar_t* Autocad::ConvertToWideChar(const char* c, ...)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

AcDbRegion* Autocad::CreateCompositeCurve(const std::vector<CompositeCurveSegment>& compositeCurve, const Matrix4& transform)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcDbVoidPtrArray lines;
	Acad::ErrorStatus es;
	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray regions;

	for (const auto& compositeCurveSegment : compositeCurve)
	{
		if (compositeCurveSegment.PointsPolyligne.size() > 0)
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
	}

	//Arc
	AcDbArc* arc = new AcDbArc();
	for (const auto& compositeCurveSegment : compositeCurve)
	{
		if (compositeCurveSegment.TrimmedCurves != nullptr)
		{
			double pointX = compositeCurveSegment.TrimmedCurves->centreCircle.x();
			double pointY = compositeCurveSegment.TrimmedCurves->centreCircle.y();
			double radius = compositeCurveSegment.TrimmedCurves->Radius;

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
	}

	// Create a region from the set of lines.
	AcDbRegion* pRegion = CreateRegion(lines, regions);

	return pRegion;
}

void Autocad::DrawVoids(AcDb3dSolid* solid)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	if (s_ObjectVoids.find(m_IfcObject->Key) != s_ObjectVoids.end())
	{
		for (auto objectVoid : s_ObjectVoids[m_IfcObject->Key])
		{
			IFCShapeRepresentation& shapeRepresentations = *objectVoid->ShapeRepresentations.begin();

			if (shapeRepresentations.ProfilDefName == "IfcArbitraryClosedProfileDef")
			{
				CreationVoid(solid, objectVoid);
			}
			else if (shapeRepresentations.ProfilDefName == "IfcCircleProfileDef")
			{
				CreationVoidCircle(solid, objectVoid);
			}
			else if (shapeRepresentations.ProfilDefName == "IfcRectangleProfileDef")
			{
				CreationVoidRectangle(solid, objectVoid);
			}
		}
	}
}

void Autocad::HandleDeplacements(AcDb3dSolid* solid, IFCShapeRepresentation& shape, bool move2D)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	if (m_IfcObject->IsMappedItem)
		DeplacementObjet3DMappedItem(solid, shape.TransformationOperator3D);

	DeplacementObjet(solid, m_IfcObject->LocalTransform);

	if (move2D)
		DeplacementObjet(solid, shape.Transformation2D);
}

void Autocad::HandleDeplacements(AcDbSubDMesh* pSubDMesh, IFCShapeRepresentation& shape, bool move2D)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	if (m_IfcObject->IsMappedItem)
		DeplacementObjet3DMappedItem(pSubDMesh, shape.TransformationOperator3D);

	DeplacementObjet(pSubDMesh, m_IfcObject->LocalTransform);

	if (move2D)
		DeplacementObjet(pSubDMesh, shape.Transformation2D);
}

void Autocad::HandleDeplacements(AcDb3dSolid* solid, bool move2D, ProfilDef* profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	if (m_IfcObject->IsMappedItem)
		DeplacementObjet3DMappedItem(solid, profilDef->TransformationOperator3D);

	DeplacementObjet(solid, m_IfcObject->LocalTransform);

	if (move2D)
		DeplacementObjet(solid, profilDef->Transformation2D);
}

void Autocad::HandleDeplacements(AcDbSubDMesh* pSubDMesh, bool move2D, ProfilDef* profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	if (m_IfcObject->IsMappedItem)
		DeplacementObjet3DMappedItem(pSubDMesh, profilDef->TransformationOperator3D);

	DeplacementObjet(pSubDMesh, m_IfcObject->LocalTransform);

	if (move2D)
		DeplacementObjet(pSubDMesh, profilDef->Transformation2D);
}

void Autocad::DrawElement(const ACHAR* layerName, AcDb3dSolid* pSolid, Acad::ErrorStatus& es)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	if (es == Acad::eOk)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		es = acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		es = pBlockTableRecord->appendAcDbEntity(pSolid);
		es = pBlockTableRecord->close();
		es = pSolid->close();
	}
	else
	{
		delete pSolid;
		acutPrintf(_T("Je ne fais rien du tout"));
		return;
	}
}

void Autocad::DrawElement(const ACHAR* layerName, AcDbSubDMesh* pSubDMesh, AcArray<AcCmEntityColor>& clrArray, Acad::ErrorStatus& es)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	pSubDMesh->setLayer(layerName, Adesk::kFalse, false);
	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	if (es == Acad::eOk)
	{
		AcDbBlockTable* pBlockTable;
		AcDbBlockTableRecord* pSpaceRecord;

		es = acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);
		es = pBlockTable->getAt(ACDB_MODEL_SPACE, pSpaceRecord, AcDb::kForWrite);
		es = pBlockTable->close();

		AcDbObjectId meshId = AcDbObjectId::kNull;
		es = pSpaceRecord->appendAcDbEntity(meshId, pSubDMesh);
		es = pSubDMesh->setVertexColorArray(clrArray);
		es = pSubDMesh->close();
		es = pSpaceRecord->close();
	}
	else
	{
		delete pSubDMesh;
		acutPrintf(_T("Je ne fais rien du tout"));
		return;
	}
}

void Autocad::CreationVoid(AcDb3dSolid* extrusion, std::shared_ptr<IFCObject> objectVoid)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	Acad::ErrorStatus es;
	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	IFCShapeRepresentation& shapeRepresentations = *objectVoid->ShapeRepresentations.begin();

	ptArr.setLogicalLength(shapeRepresentations.Points.size());
	Vec3 pointOrigine = { objectVoid->LocalTransform[12], objectVoid->LocalTransform[13] , objectVoid->LocalTransform[14] };

	int i = 0;
	for (const auto& point : shapeRepresentations.Points)
	{
		ptArr[i].set(point.x(), point.y(), point.z());
		i++;
	}

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);

	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(shapeRepresentations.ExtrusionVector, shapeRepresentations.ExtrusionHeight);
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

	if (objectVoid->ShapeRepresentations.size() > 1)
	{
		DeplacementObjet(extrusion_void, shapeRepresentations.Transformation2D);
		DeplacementObjet(extrusion_void2, shapeRepresentations.Transformation2D);

		for (int a = 1; a < m_IfcObject->ShapeRepresentations.size(); a++)
		{
			CreationSection(extrusion_void, m_IfcObject->ShapeRepresentations[a]);
			CreationSection(extrusion_void2, m_IfcObject->ShapeRepresentations[a]);
		}
	}

	if (objectVoid->IsMappedItem)
	{
		DeplacementObjet3DMappedItem(extrusion_void, shapeRepresentations.TransformationOperator3D);
		DeplacementObjet3DMappedItem(extrusion_void2, shapeRepresentations.TransformationOperator3D);
	}

	DeplacementObjet(extrusion_void, objectVoid->LocalTransform);
	DeplacementObjet(extrusion_void2, objectVoid->LocalTransform);

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

void Autocad::CreationVoidCircle(AcDb3dSolid* extrusion, std::shared_ptr<IFCObject> objectVoid)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	Acad::ErrorStatus es;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	IFCShapeRepresentation& shapeRepresentations = *objectVoid->ShapeRepresentations.begin();
	auto& profilDef = dynamic_cast<Circle_profilDef&>(*shapeRepresentations.ProfilDef);

	float radius = profilDef.Radius;
	AcGePoint3d center = AcGePoint3d::AcGePoint3d(radius, radius, 0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDbCircle* circle = CreateCircle(center, radius, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(circle, lines, regions);

	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);
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

	if (objectVoid->ShapeRepresentations.size() > 1)
	{
		DeplacementObjet(extrusion_void, profilDef.Transformation2D);
		DeplacementObjet(extrusion_void2, profilDef.Transformation2D);

		for (int a = 1; a < m_IfcObject->ShapeRepresentations.size(); a++)
		{
			CreationSection(extrusion_void, m_IfcObject->ShapeRepresentations[a]);
			CreationSection(extrusion_void2, m_IfcObject->ShapeRepresentations[a]);
		}
	}

	if (objectVoid->IsMappedItem)
	{
		DeplacementObjet3DMappedItem(extrusion_void, shapeRepresentations.TransformationOperator3D);
		DeplacementObjet3DMappedItem(extrusion_void2, shapeRepresentations.TransformationOperator3D);
	}

	DeplacementObjet(extrusion_void, objectVoid->LocalTransform);
	DeplacementObjet(extrusion_void2, objectVoid->LocalTransform);

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

void Autocad::CreationVoidRectangle(AcDb3dSolid* extrusion, std::shared_ptr<IFCObject> objectVoid)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	Acad::ErrorStatus es;
	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	IFCShapeRepresentation& shapeRepresentations = *objectVoid->ShapeRepresentations.begin();
	auto& profilDef = dynamic_cast<Rectangle_profilDef&>(*shapeRepresentations.ProfilDef);

	double xDim = profilDef.XDim;
	double yDim = profilDef.YDim;

	ptArr.setLogicalLength(4);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(xDim, 0, 0.0);
	ptArr[2].set(xDim, yDim, 0.0);
	ptArr[3].set(0, yDim, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-xDim / 2, -yDim / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);

	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);
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

	if (objectVoid->ShapeRepresentations.size() > 1)
	{
		DeplacementObjet(extrusion_void, profilDef.Transformation2D);
		DeplacementObjet(extrusion_void2, profilDef.Transformation2D);

		for (int a = 1; a < m_IfcObject->ShapeRepresentations.size(); a++)
		{
			CreationSection(extrusion_void, m_IfcObject->ShapeRepresentations[a]);
			CreationSection(extrusion_void2, m_IfcObject->ShapeRepresentations[a]);
		}
	}

	if (objectVoid->IsMappedItem)
	{
		DeplacementObjet3DMappedItem(extrusion_void, shapeRepresentations.TransformationOperator3D);
		DeplacementObjet3DMappedItem(extrusion_void2, shapeRepresentations.TransformationOperator3D);
	}

	DeplacementObjet(extrusion_void, objectVoid->LocalTransform);
	DeplacementObjet(extrusion_void2, objectVoid->LocalTransform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	AcDbDatabase* pDb = curDoc()->database();
	AcDbObjectId modelId;
	modelId = acdbSymUtil()->blockModelSpaceId(pDb);
	AcDbBlockTableRecord* pBlockTableRecord;
	acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
	// pBlockTableRecord->appendAcDbEntity(extrusion_void);
	// pBlockTableRecord->appendAcDbEntity(extrusion_void2);
	pBlockTableRecord->close();

	extrusion->booleanOper(AcDb::kBoolSubtract, extrusion_void);
	extrusion->booleanOper(AcDb::kBoolSubtract, extrusion_void2);
	extrusion_void->close();
	extrusion_void2->close();
}

void Autocad::CreationSection(AcDb3dSolid* extrusion, IFCShapeRepresentation& shapeRepresentation)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGeVector3d v1 = AcGeVector3d::AcGeVector3d(0, 0, 0);             // Vector 1 (x,y,z) & Vector 2 (x,y,z)
	AcGeVector3d v2 = AcGeVector3d::AcGeVector3d(0, 0, 0);
	AcGeVector3d normal = AcGeVector3d::AcGeVector3d(0, 0, 0);
	double p0x, p0y, p0z;
	AcGePoint3d p0 = AcGePoint3d::AcGePoint3d();
	double p1x, p1y, p1z;
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
		AcDb2dPolyline* pNewPline = nullptr;
		AcDbRegion* pRegion = nullptr;

		if (shapeRepresentation.Points.size() > 0)
		{
			ptArr.setLogicalLength(shapeRepresentation.Points.size());

			int i = 0;
			for (const auto& point : shapeRepresentation.Points)
			{
				ptArr[i].set(point.x(), point.y(), point.z());
				i++;
			}

			pNewPline = CreatePolyline(ptArr, lines);
			pRegion = CreateRegion(pNewPline, lines, regions);
			if (pRegion == nullptr) return;
		}

		if (shapeRepresentation.CompositeCurve.size() > 0)
		{
			pRegion = CreateCompositeCurve(shapeRepresentation.CompositeCurve, m_IfcObject->LocalTransform);
		}

		if (pRegion == nullptr) return;

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
			DeplacementObjet3DMappedItem(pSolid, shapeRepresentation.TransformationOperator3D);

		DeplacementObjet(pSolid, shapeRepresentation.Transformation);

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

			DeplacementObjet(extrusionBool, shapeRepresentation.TransformBoolean);

			AcDbDatabase* pDb = curDoc()->database();
			AcDbObjectId modelId = acdbSymUtil()->blockModelSpaceId(pDb);
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

			Rectangle_profilDef& rectProfilDef = dynamic_cast<Rectangle_profilDef&>(*shapeRepresentation.ProfilDef);

			double XDim = rectProfilDef.XDim;
			double YDim = rectProfilDef.YDim;

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

void Autocad::CreationProfilDef(AcDbRegion* pRegion, AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions, const AcGeVector3d& vecExtru, ProfilDef* profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	Acad::ErrorStatus es;

	const ACHAR* layerName = GetLayerName(m_IfcObject->Entity);
	AddTableRecord(layerName);

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

	DeplacementObjet(pSolid, profilDef->Transformation2D);
	DeplacementObjet(pSolid, profilDef->Transform);

	if (m_IfcObject->IsMappedItem)
	{
		auto& shapeRep = (*m_IfcObject->ShapeRepresentations.begin());

		if (m_IfcObject != nullptr && shapeRep.SubShapeRepresentations.size() > 1)
		{
			for (int a = 1; a < shapeRep.SubShapeRepresentations.size(); a++)
			{
				CreationSection(pSolid, shapeRep.SubShapeRepresentations[a]);
			}
		}
	}
	else
	{
		if (m_IfcObject != nullptr && m_IfcObject->ShapeRepresentations.size() > 1)
		{
			for (int a = 1; a < m_IfcObject->ShapeRepresentations.size(); a++)
			{
				CreationSection(pSolid, m_IfcObject->ShapeRepresentations[a]);
			}
		}
	}

	HandleDeplacements(pSolid, false, profilDef);
	DrawVoids(pSolid);
	SetColor(pSolid, profilDef->Key);
	DrawElement(layerName, pSolid, es);
}

void Autocad::CreateSolid3dProfilIPE(I_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	double width = profilDef.OverallWidth;
	double depth = profilDef.OverallDepth;
	double webThickness = profilDef.WebThickness;
	double flangeThickness = profilDef.FlangeThickness;

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	ptArr.setLogicalLength(12);
	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, flangeThickness, 0.0);
	ptArr[3].set((width + webThickness) / 2, flangeThickness, 0.0);
	ptArr[4].set((width + webThickness) / 2, depth - flangeThickness, 0.0);
	ptArr[5].set(width, depth - flangeThickness, 0.0);
	ptArr[6].set(width, depth, 0.0);
	ptArr[7].set(0, depth, 0.0);
	ptArr[8].set(0, depth - flangeThickness, 0.0);
	ptArr[9].set((width - webThickness) / 2, depth - flangeThickness, 0.0);
	ptArr[10].set((width - webThickness) / 2, flangeThickness, 0.0);
	ptArr[11].set(0, flangeThickness, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();
	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilIPN(I_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	double width = profilDef.OverallWidth;
	double depth = profilDef.OverallDepth;
	double webThickness = profilDef.WebThickness;
	double flangeThickness = profilDef.FlangeThickness;
	double flangeSlope = profilDef.FlangeSlope;

	double dy = (width - width / 4) * tan(flangeSlope);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	ptArr.setLogicalLength(16);
	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, flangeThickness - dy, 0.0);
	ptArr[3].set(width - width / 4, flangeThickness, 0.0);
	ptArr[4].set((width + webThickness) / 2, flangeThickness + dy, 0.0);
	ptArr[5].set((width + webThickness) / 2, depth - flangeThickness - dy, 0.0);
	ptArr[6].set(width - width / 4, depth - flangeThickness, 0.0);
	ptArr[7].set(width, depth - flangeThickness + dy, 0.0);
	ptArr[8].set(width, depth, 0.0);
	ptArr[9].set(0, depth, 0.0);
	ptArr[10].set(0, depth - flangeThickness + dy, 0.0);
	ptArr[11].set(width / 4, depth - flangeThickness, 0.0);
	ptArr[12].set((width - webThickness) / 2, depth - flangeThickness - dy, 0.0);
	ptArr[13].set((width - webThickness) / 2, flangeThickness + dy, 0.0);
	ptArr[14].set(width / 4, flangeThickness, 0.0);
	ptArr[15].set(0, flangeThickness - dy, 0.0);

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilL8(L_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.Width;
	double thickness = profilDef.Thickness;
	double filletRadius = profilDef.FilletRadius;

	ptArr.setLogicalLength(6);
	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, thickness, 0.0);
	ptArr[3].set(thickness, thickness, 0.0);
	ptArr[4].set(thickness, depth, 0.0);
	ptArr[5].set(0, depth, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilL9(L_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.Width;
	double thickness = profilDef.Thickness;
	double filletRadius = profilDef.FilletRadius;
	double legSlope = profilDef.LegSlope;

	double dy = (width - thickness) * tan(legSlope);

	ptArr.setLogicalLength(6);
	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, thickness, 0.0);
	ptArr[3].set(thickness + dy, thickness + dy, 0.0);
	ptArr[4].set(thickness, depth, 0.0);
	ptArr[5].set(0, depth, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilT10(T_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.FlangeWidth;
	double webThickness = profilDef.WebThickness;
	double flangeThickness = profilDef.FlangeThickness;
	double filletRadius = profilDef.FilletRadius;

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, flangeThickness, 0.0);
	ptArr[3].set((width + webThickness) / 2, flangeThickness, 0.0);
	ptArr[4].set((width + webThickness) / 2, depth, 0.0);
	ptArr[5].set((width - webThickness) / 2, depth, 0.0);
	ptArr[6].set((width - webThickness) / 2, flangeThickness, 0.0);
	ptArr[7].set(0, flangeThickness, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilT12(T_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.FlangeWidth;
	double webThickness = profilDef.WebThickness;
	double flangeThickness = profilDef.FlangeThickness;
	double filletRadius = profilDef.FilletRadius;
	double webSlope​ = profilDef.WebSlope;
	double flangeSlope = profilDef.FlangeSlope;

	double dy1 = (width - flangeThickness) * tan(flangeSlope);
	double dy2 = (depth - webThickness) * tan(webSlope​);

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, flangeThickness - dy1, 0.0);
	ptArr[3].set((width + webThickness) / 2, flangeThickness, 0.0);
	ptArr[4].set(((width + webThickness) / 2) - dy2, depth, 0.0);
	ptArr[5].set(((width - webThickness) / 2) + dy2, depth, 0.0);
	ptArr[6].set((width - webThickness) / 2, flangeThickness, 0.0);
	ptArr[7].set(0, flangeThickness - dy1, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilUPE(U_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.FlangeWidth;
	double webThickness = profilDef.WebThickness;
	double flangeThickness = profilDef.FlangeThickness;
	double filletRadius = profilDef.FilletRadius;

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, flangeThickness, 0.0);
	ptArr[3].set(webThickness, flangeThickness, 0.0);
	ptArr[4].set(webThickness, depth - flangeThickness, 0.0);
	ptArr[5].set(width, depth - flangeThickness, 0.0);
	ptArr[6].set(width, depth, 0.0);
	ptArr[7].set(0, depth, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilUPN(U_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.FlangeWidth;
	double webThickness = profilDef.WebThickness;
	double flangeThickness = profilDef.FlangeThickness;
	double filletRadius = profilDef.FilletRadius;

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, flangeThickness, 0.0);
	ptArr[3].set(webThickness, flangeThickness, 0.0);
	ptArr[4].set(webThickness, depth - flangeThickness, 0.0);
	ptArr[5].set(width, depth - flangeThickness, 0.0);
	ptArr[6].set(width, depth, 0.0);
	ptArr[7].set(0, depth, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilC(C_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.Width;
	double wallThickness = profilDef.WallThickness;
	double girth = profilDef.Depth;

	ptArr.setLogicalLength(12);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(width, 0, 0.0);
	ptArr[2].set(width, girth, 0.0);
	ptArr[3].set(width - wallThickness, girth, 0.0);
	ptArr[4].set(width - wallThickness, wallThickness, 0.0);
	ptArr[5].set(wallThickness, wallThickness, 0.0);
	ptArr[6].set(wallThickness, depth - wallThickness, 0.0);
	ptArr[7].set(width - wallThickness, depth - wallThickness, 0.0);
	ptArr[8].set(width - wallThickness, depth - girth, 0.0);
	ptArr[9].set(width, depth - girth, 0.0);
	ptArr[10].set(width, depth, 0.0);
	ptArr[11].set(0, depth, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilZ(Z_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double depth = profilDef.Depth;
	double width = profilDef.FlangeWidth;
	double webThickness = profilDef.WebThickness;
	double flangeThickness = profilDef.FlangeThickness;

	ptArr.setLogicalLength(9);

	ptArr[0].set(width - webThickness, 0, 0.0);
	ptArr[1].set(width - webThickness + width, 0, 0.0);
	ptArr[2].set(width - webThickness + width, flangeThickness, 0.0);
	ptArr[3].set(width, flangeThickness, 0.0);
	ptArr[4].set(width, depth, 0.0);
	ptArr[5].set(0, depth, 0);
	ptArr[6].set(0, depth - flangeThickness, 0.0);
	ptArr[7].set(width - webThickness, depth - flangeThickness, 0.0);
	ptArr[8].set(0, depth, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-width + webThickness / 2, -depth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilAsyI(AsymmetricI_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double bottomFlangeWidth = profilDef.OverallWidth;
	double overallDepth = profilDef.OverallDepth;
	double webThickness = profilDef.WebThickness;
	double bottomFlangeThickness = profilDef.FlangeThickness;
	double topFlangeWidth = profilDef.TopFlangeWidth;
	double topFlangeThickness = profilDef.TopFlangeThickness;
	double topMinus = (bottomFlangeWidth - topFlangeWidth) / 2;

	ptArr.setLogicalLength(12);
	ptArr[1].set(bottomFlangeWidth, 0, 0.0);
	ptArr[2].set(bottomFlangeWidth, bottomFlangeThickness, 0.0);
	ptArr[3].set((bottomFlangeWidth + webThickness) / 2, bottomFlangeThickness, 0.0);
	ptArr[4].set((bottomFlangeWidth + webThickness) / 2, overallDepth - topFlangeThickness, 0.0);
	ptArr[5].set(bottomFlangeWidth - topMinus, overallDepth - topFlangeThickness, 0.0);
	ptArr[6].set(bottomFlangeWidth - topMinus, overallDepth, 0.0);
	ptArr[7].set(topMinus, overallDepth, 0.0);
	ptArr[8].set(topMinus, overallDepth - topFlangeThickness, 0.0);
	ptArr[9].set((bottomFlangeWidth - webThickness) / 2, overallDepth - topFlangeThickness, 0.0);
	ptArr[10].set((bottomFlangeWidth - webThickness) / 2, bottomFlangeThickness, 0.0);
	ptArr[11].set(0, bottomFlangeThickness, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-bottomFlangeWidth / 2, -overallDepth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilCircHollow(CircleHollow_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcDbVoidPtrArray lines[2];
	AcDbVoidPtrArray regions[2];
	AcDbCircle* pCircle[2];
	AcDbRegion* pRegion[2];

	double radius = profilDef.Radius;
	double wallThickness = profilDef.WallThickness;
	AcGePoint3d center = AcGePoint3d::AcGePoint3d(radius, radius, 0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-radius / 2, -radius / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	pCircle[0] = CreateCircle(center, radius, lines[0], true, &matrix3d, &acVec3d);
	pRegion[0] = CreateRegion(pCircle[0], lines[0], regions[0]);
	if (pRegion[0] == nullptr) return;

	pCircle[1] = CreateCircle(center, radius - wallThickness, lines[1], true, &matrix3d, &acVec3d);
	pRegion[1] = CreateRegion(pCircle[1], lines[1], regions[1]);
	if (pRegion[1] == nullptr) return;

	pRegion[0]->booleanOper(AcDb::kBoolSubtract, pRegion[1]);

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion[0], lines[0], regions[0], vecExtru, &profilDef);

	for (int i = 0; i < lines[1].length(); i++)
	{
		delete (AcRxObject*)lines[1][i];
	}

	for (int ii = 0; ii < regions[1].length(); ii++)
	{
		delete (AcRxObject*)regions[1][ii];
	}
}

void Autocad::CreateSolid3dProfilRectHollow(RectangleHollow_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr[2];
	AcDbVoidPtrArray lines[2];
	AcDbVoidPtrArray regions[2];
	AcDb2dPolyline* pNewPline[2];
	AcDbRegion* pRegion[2];

	double xDim = profilDef.XDim;
	double yDim = profilDef.YDim;
	double wallThickness = profilDef.WallThickness;

	ptArr[0].setLogicalLength(4);
	ptArr[0][0].set(0, 0, 0.0);
	ptArr[0][1].set(xDim, 0, 0.0);
	ptArr[0][2].set(xDim, yDim, 0.0);
	ptArr[0][3].set(0, yDim, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-xDim / 2, -yDim / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	pNewPline[0] = CreatePolyline(ptArr[0], lines[0], true, &matrix3d, &acVec3d);
	pRegion[0] = CreateRegion(pNewPline[0], lines[0], regions[0]);
	if (pRegion[0] == nullptr) return;

	ptArr[1].setLogicalLength(4);
	ptArr[1][0].set(wallThickness, wallThickness, 0.0);
	ptArr[1][1].set(xDim - wallThickness, wallThickness, 0.0);
	ptArr[1][2].set(xDim - wallThickness, yDim - wallThickness, 0.0);
	ptArr[1][3].set(wallThickness, yDim - wallThickness, 0.0);

	pNewPline[1] = CreatePolyline(ptArr[1], lines[1], true, &matrix3d, &acVec3d);
	pRegion[1] = CreateRegion(pNewPline[1], lines[1], regions[1]);
	if (pRegion[1] == nullptr) return;

	pRegion[0]->booleanOper(AcDb::kBoolSubtract, pRegion[1]);

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion[0], lines[0], regions[0], vecExtru, &profilDef);

	for (int i = 0; i < lines[1].length(); i++)
	{
		delete (AcRxObject*)lines[1][i];
	}

	for (int ii = 0; ii < regions[1].length(); ii++)
	{
		delete (AcRxObject*)regions[1][ii];
	}
}

void Autocad::CreateSolid3dProfilCircle(Circle_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double radius = profilDef.Radius;
	AcGePoint3d center = AcGePoint3d::AcGePoint3d(radius, radius, 0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-radius / 2, -radius / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDbCircle* pcircle = CreateCircle(center, radius, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pcircle, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

void Autocad::CreateSolid3dProfilRectangle(Rectangle_profilDef& profilDef)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	double xDim = profilDef.XDim;
	double yDim = profilDef.YDim;

	ptArr.setLogicalLength(4);
	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(xDim, 0, 0.0);
	ptArr[2].set(xDim, yDim, 0.0);
	ptArr[3].set(0, yDim, 0.0);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-xDim / 2, -yDim / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();

	AcDb2dPolyline* pNewPline = CreatePolyline(ptArr, lines, true, &matrix3d, &acVec3d);
	AcDbRegion* pRegion = CreateRegion(pNewPline, lines, regions);
	if (pRegion == nullptr) return;

	AcGeVector3d vecExtru = GetExtrusionVector(profilDef.VecteurExtrusion, profilDef.HauteurExtrusion);

	CreationProfilDef(pRegion, lines, regions, vecExtru, &profilDef);
}

AcDbRegion* Autocad::CreateRegion(AcDb2dPolyline* pNewPline, AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

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

AcDbRegion* Autocad::CreateRegion(AcDbCircle* pCircle, AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	Acad::ErrorStatus es;

	es = AcDbRegion::createFromCurves(lines, regions);
	if (es != Acad::eOk)
	{
		pCircle->close();
		acutPrintf(L"\nFailed to create region\n");
		return nullptr;
	}

	return AcDbRegion::cast((AcRxObject*)regions[0]);
}

AcDbRegion* Autocad::CreateRegion(AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	Acad::ErrorStatus es;

	es = AcDbRegion::createFromCurves(lines, regions);
	if (es != Acad::eOk)
	{
		acutPrintf(L"\nFailed to create region\n");
		return nullptr;
	}

	return AcDbRegion::cast((AcRxObject*)regions[0]);
}

AcGeVector3d Autocad::GetExtrusionVector(const Vec3& vector, double height)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	if (height == 0.0)
	{
		const Vec3& vec = m_IfcObject->ShapeRepresentations.begin()->ExtrusionVector;
		height = m_IfcObject->ShapeRepresentations.begin()->ExtrusionHeight;

		return AcGeVector3d::AcGeVector3d(vec.x() * height, vec.y() * height, vec.z() * height);
	}
	else
	{
		return AcGeVector3d::AcGeVector3d(vector.x() * height, vector.y() * height, vector.z() * height);
	}
}

AcDb2dPolyline* Autocad::CreatePolyline(AcGePoint3dArray& ptArr, AcDbVoidPtrArray& lines, bool shouldTransform, AcGeMatrix3d* matrix3d, AcGeVector3d* acVec3d)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	if (shouldTransform)
	{
		pNewPline->transformBy(matrix3d->translation(*acVec3d));
	}

	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;
	if (pNewPline == NULL)
	{
		pEntity->close();
		return nullptr;
	}

	pNewPline->explode(lines);
	pNewPline->close();

	return pNewPline;
}

AcDbCircle* Autocad::CreateCircle(AcGePoint3d& center, double radius, AcDbVoidPtrArray& lines, bool shouldTransform, AcGeMatrix3d* matrix3d, AcGeVector3d* acVec3d)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

	AcDbCircle* circle = new AcDbCircle();
	circle->setCenter(center);
	circle->setRadius(radius);
	circle->setColorIndex(3);

	if (shouldTransform)
	{
		circle->transformBy(matrix3d->translation(*acVec3d));
	}

	AcDbEntity* pEntity = NULL;
	if (circle == NULL)
	{
		pEntity->close();
		return nullptr;
	}

	lines.append(circle);
	//circle->explode(lines);
	circle->close();

	return circle;
}

void Autocad::DeplacementObjet3DMappedItem(AcDb3dSolid* solid, const Matrix4& transform)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

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

void Autocad::DeplacementObjet3DMappedItem(AcDbSubDMesh* pSubDMesh, const Matrix4& transform)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

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

	pSubDMesh->transformBy(mat);
}

void Autocad::DeplacementObjet(AcDb3dSolid* solid, const Matrix4& transform)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

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

void Autocad::DeplacementObjet(AcDbSubDMesh* pSubDMesh, const Matrix4& transform)
{
	Timer timer(m_IfcObject->Key, m_IfcObject->Entity, __FUNCTION__);

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

	es = pSubDMesh->transformBy(mat);
}
