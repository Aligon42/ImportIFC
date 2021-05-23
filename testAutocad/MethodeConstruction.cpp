#include "MethodeConstruction.h"
#include "CreateConstructionPointVisitor.h"
#include <vector>

void MethodeConstruction::createSolid3d(ObjectToConstruct object)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr;

	std::vector<ElementToConstruct>::iterator it = object.ElementsToConstruct.begin();
	for (size_t i = 0; i < object.ElementsToConstruct.size(); i++)
	{
		if ((*it).Points.size() == 0)
			object.ElementsToConstruct.erase(it);

		it++;
	}

	int totalPts = 0;
	int totalArgs = 0;
	for (auto& els : object.ElementsToConstruct)
	{
		totalPts += els.Points.size();

		for (auto& el : els.Args)
		{
			totalArgs += el;
		}
	}

	if (totalPts == 0 || totalArgs == 0) return;

	if (object.ElementsToConstruct.size() > 1 && object.ElementsToConstruct[0].Args[0] == 1)
	{
		object.ElementsToConstruct[0].Args.erase(object.ElementsToConstruct[0].Args.begin());
		object.ElementsToConstruct[0].Points.pop_front();

		if (object.ElementsToConstruct[0].Points.size() == 0)
			object.ElementsToConstruct.erase(object.ElementsToConstruct.begin());
	}

	ptArr.setLogicalLength(object.ElementsToConstruct[0].Args[0]);
	Vec3 pointOrigine = { object.Transform[12], object.Transform[13] , object.Transform[14] };

	int i = 0;

	for (const auto& point : object.ElementsToConstruct[0].Points)
	{
		if (i == object.ElementsToConstruct[0].Args[0])
		{
			break;
		}

		ptArr[i].set(point.x(), point.y(), point.z());

		i++;
	}

	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;
	if (pNewPline == NULL)
	{
		pEntity->close();
		return;
	}
	AcDbVoidPtrArray lines;
	pNewPline->explode(lines);
	pNewPline->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(lines, regions);
	if (Acad::eOk != es)
	{
		pNewPline->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}

	AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);
	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	es = pSolid->extrude(pRegion, object.VecteurExtrusion.z(), 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	if (object.ElementsToConstruct.size() == 1)
	{
		int nbPlan = object.ElementsToConstruct[0].Plans.size();
		for (size_t i = 0; i < nbPlan; i++)
		{
			CreationSection(pSolid, object.VecteurExtrusion, object.ElementsToConstruct[0]);

			object.ElementsToConstruct[0].Plans.pop_front();
		}
	}

	for (int a = 1; a < object.ElementsToConstruct.size(); a++)
	{
		int nbPlan = object.ElementsToConstruct[a].Plans.size();
		for (size_t i = 0; i < nbPlan; i++)
		{
			CreationSection(pSolid, object.VecteurExtrusion, object.ElementsToConstruct[a]);

			object.ElementsToConstruct[a].Plans.pop_front();
		}
	}

	DeplacementObjet3D(pSolid, object.Transform);

	for (int v = 0; v < _listVoid.size(); v++)
	{
		if (object.Key == _listVoid[v].Key)
		{
			if (_listVoid[v].NameProfilDef == "IfcArbitraryClosedProfileDef")
			{
				CreationVoid(pSolid, _listVoid[v]);
				//listVoid.erase(listVoid.begin() + v);
			}
			else if (_listVoid[v].NameProfilDef == "IfcCircleProfileDef")
			{
				CreationVoidCircle(pSolid, _listVoid[v]);
				//listVoid.erase(listVoid.begin() + v);
			}
			else if (_listVoid[v].NameProfilDef == "IfcRectangleProfileDef")
			{
				CreationVoidRectangle(pSolid, _listVoid[v]);
				//listVoid.erase(listVoid.begin() + v);
			}
		}
	}

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	if (Acad::eOk == es)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);
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

void MethodeConstruction::createSolid3dProfil(BaseProfilDef* profilDef)
{
	if (profilDef->Name == "IfcIShapeProfileDef")
	{
		if (((I_profilDef*)profilDef)->nbArg == 5)
		{
			createSolid3dProfilIPE(*(I_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
			delete (I_profilDef*)profilDef;
		}
		else
		{
			//createSolid3dProfilIPN(IprofilDef, VecteurExtrusion, transform1);
			delete (I_profilDef*)profilDef;
		}
	}
	else if (profilDef->Name == "IfcLShapeProfileDef")
	{
		if (((L_profilDef*)profilDef)->nbArg == 5)
		{
			createSolid3dProfilL8(*(L_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
			delete (L_profilDef*)profilDef;
		}
		else
		{
			createSolid3dProfilL9(*(L_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
			delete (L_profilDef*)profilDef;
		}
	}
	else if (profilDef->Name == "IfcTShapeProfileDef")
	{
		if (((T_profilDef*)profilDef)->nbArg == 7)
		{
			createSolid3dProfilT10(*(T_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
			delete (T_profilDef*)profilDef;
		}
		else
		{
			createSolid3dProfilT12(*(T_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
			delete (T_profilDef*)profilDef;
		}
	}
	else if (profilDef->Name == "IfcUShapeProfileDef")
	{
		if (((U_profilDef*)profilDef)->nbArg == 5)
		{
			createSolid3dProfilUPE(*(U_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
			delete (U_profilDef*)profilDef;
		}
		else
		{
			createSolid3dProfilUPN(*(U_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
			delete (U_profilDef*)profilDef;
		}
	}
	else if (profilDef->Name == "IfcCShapeProfileDef")
	{
		createSolid3dProfilC(*(C_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
		delete (C_profilDef*)profilDef;
	}
	else if (profilDef->Name == "IfcZShapeProfileDef")
	{
		createSolid3dProfilZ(*(Z_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
		delete (Z_profilDef*)profilDef;
	}
	else if (profilDef->Name == "IfcAsymmetricIShapeProfileDef")
	{
		createSolid3dProfilAsyI(*(AsymmetricI_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
		delete (AsymmetricI_profilDef*)profilDef;
	}
	else if (profilDef->Name == "IfcCircleHollowProfileDef")
	{
		createSolid3dProfilCircHollow(*(CircleHollow_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
		delete (CircleHollow_profilDef*)profilDef;
	}
	else if (profilDef->Name == "IfcRectangleHollowProfileDef")
	{
		createSolid3dProfilRectHollow(*(RectangleHollow_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
		delete (RectangleHollow_profilDef*)profilDef;
	}
	else if (profilDef->Name == "IfcCircleProfileDef")
	{
		createSolid3dProfilCircle(*(Circle_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
		delete (Circle_profilDef*)profilDef;
	}
	else if (profilDef->Name == "IfcRectangleProfileDef")
	{
		createSolid3dProfilRectangle(*(Rectangle_profilDef*)profilDef, profilDef->VecteurExtrusion, profilDef->Transform);
		delete (Rectangle_profilDef*)profilDef;
	}
}

void MethodeConstruction::DeplacementObjet3D(AcDb3dSolid* pSolid, Matrix4 transform)
{
	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);

	double x1 = transform.operator()(12);  //PointDeplacement x

	double y1 = transform.operator()(13); //PointDeplacement y

	double z1 = transform.operator()(14); //PointDeplacement z

	double x2 = transform.operator()(8); //Direction1 x

	double y2 = transform.operator()(9); //Direction1 y

	double z2 = transform.operator()(10); //Direction1 z

	double x3 = transform.operator()(0); //Direction2 x

	double y3 = transform.operator()(1); //Direction2 y

	double z3 = transform.operator()(2); //Direction2 z

	// 3 destination points
	AcGePoint3d destpt1 = AcGePoint3d::AcGePoint3d(x1, y1, z1);
	AcGePoint3d destpt2 = AcGePoint3d::AcGePoint3d(x1 + x2, y1 + y2, z1 + z2);
	AcGePoint3d destpt3 = AcGePoint3d::AcGePoint3d(x1 + x3, y1 + y3, z1 + z3);

	AcGePoint3d fromOrigin = srcpt1;
	AcGeVector3d fromXaxis = srcpt2 - srcpt1;
	AcGeVector3d fromYaxis = srcpt3 - srcpt1;
	AcGeVector3d fromZaxis = fromXaxis.crossProduct(fromYaxis);

	//ed.WriteMessage("\nVecteur Origine X : " + fromXaxis.X + " , " + fromXaxis.Y + " , " + fromXaxis.Z);
	//ed.WriteMessage("\nVecteur Origine Y : " + fromYaxis.X + " , " + fromYaxis.Y + " , " + fromYaxis.Z);
	//ed.WriteMessage("\nVecteur Origine Z : " + fromZaxis.X + " , " + fromZaxis.Y + " , " + fromZaxis.Z);

	AcGePoint3d toOrigin = destpt1;
	AcGeVector3d toXaxis = (destpt2 - destpt1).normal() * (fromXaxis.length());
	AcGeVector3d toYaxis = (destpt3 - destpt1).normal() * (fromYaxis.length());
	AcGeVector3d toZaxis = toXaxis.crossProduct(toYaxis);

	//ed.WriteMessage("\nVecteur Destination X : " + toXaxis.X + " , " + toXaxis.Y + " , " + toXaxis.Z);
	//ed.WriteMessage("\nVecteur Destination Y : " + toYaxis.X + " , " + toYaxis.Y + " , " + toYaxis.Z);
	//ed.WriteMessage("\nVecteur Destination Z : " + toZaxis.X + " , " + toZaxis.Y + " , " + toZaxis.Z);

	// Get the transformation matrix for aligning coordinate systems
	AcGeMatrix3d mat = AcGeMatrix3d::AcGeMatrix3d();
	mat = mat.alignCoordSys(fromOrigin, fromXaxis, fromYaxis, fromZaxis, toOrigin, toXaxis, toYaxis, toZaxis);

	pSolid->transformBy(mat);
}

void MethodeConstruction::CreationSection(AcDb3dSolid* extrusion, Vec3 vecteurExtrusion, ElementToConstruct& elementToConstruct)
{
	AcGeVector3d v1 = AcGeVector3d::AcGeVector3d(0, 0, 0);             // Vector 1 (x,y,z) & Vector 2 (x,y,z)
	AcGeVector3d v2 = AcGeVector3d::AcGeVector3d(0, 0, 0);
	AcGeVector3d normal = AcGeVector3d::AcGeVector3d(0, 0, 0);

	//for (int i = 0; i < PLanDeSection.Count(); i++)
	//{

	float p0x;
	float p0y;
	float p0z;
	AcGePoint3d p0 = AcGePoint3d::AcGePoint3d();

	float p1x;
	float p1y;
	float p1z;
	AcGeVector3d V1 = AcGeVector3d::AcGeVector3d();
	AcGeVector3d V2 = AcGeVector3d::AcGeVector3d();
	AcGePoint3d p3 = AcGePoint3d::AcGePoint3d();

	float p2x;
	float p2y;
	float p2z;
	//Vector3d p2 = new Vector3d(p0x + p2x, p0y + p2y, p0z + p2z);

	float p1xx;
	float p1yy;
	float p1zz;

	float p2xx;
	float p2yy;
	float p2zz;

	AcGePoint3d p1 = AcGePoint3d::AcGePoint3d();
	AcGePoint3d p2 = AcGePoint3d::AcGePoint3d();

	//AcDbLine line = AcDbLine::AcDbLine();
	AcGeVector3d v3d = AcGeVector3d::AcGeVector3d();

	AcGePoint3d p33 = AcGePoint3d::AcGePoint3d();

	AcGePoint3d p5 = AcGePoint3d::AcGePoint3d();

	////Coordonnées du repère du plan
	//p0x = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[0]); 
	p0x = elementToConstruct.Plans.front()[12];  //x																		
	//p0y = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[1]);
	p0y = elementToConstruct.Plans.front()[13]; //y
	//p0z = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[2]);
	p0z = elementToConstruct.Plans.front()[14]; //z

	p0 = AcGePoint3d::AcGePoint3d(p0x, p0y, p0z);
	acutPrintf(_T("Coordonnées du repère du plan : [ %f, %f, %f]\n"), elementToConstruct.Plans.front()[12], elementToConstruct.Plans.front()[13], elementToConstruct.Plans.front()[14]);

	///Direction1 plan
	//p1x = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[0]);
	p1x = elementToConstruct.Plans.front()[8];  //x
	//p1y = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[1]);
	p1y = elementToConstruct.Plans.front()[9]; //y
	//p1z = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[2]);
	p1z = elementToConstruct.Plans.front()[10]; //z

	V1 = AcGeVector3d::AcGeVector3d(p1x, p1y, p1z);
	V2 = V1.normal();
	p3 = AcGePoint3d::AcGePoint3d(V2.x + p0x, V2.y + p0y, V2.z + p0z);
	acutPrintf(_T("Direction1 plan : [ %f, %f, %f]\n"), elementToConstruct.Plans.front()[8], elementToConstruct.Plans.front()[9], elementToConstruct.Plans.front()[10]);

	///Direction2 plan
	//p2x = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[0]);
	p2x = elementToConstruct.Plans.front()[0];  //x
	//p2y = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[1]);
	p2y = elementToConstruct.Plans.front()[1]; //y
	//p2z = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[2]);
	p2z = elementToConstruct.Plans.front()[2]; //z
	//Vector3d p2 = new Vector3d(p0x + p2x, p0y + p2y, p0z + p2z);

	acutPrintf(_T("Direction2 plan : [ %f, %f, %f]\n"), elementToConstruct.Plans.front()[0], elementToConstruct.Plans.front()[1], elementToConstruct.Plans.front()[2]);

	p1xx = p0x + p1x;
	p1yy = p0x + p1x;
	p1zz = p0x + p1x;

	//p1xx = (p2y * p1z) - (p2z * p1y);
	//p1yy = (p2x * p1z) - (p2z * p1x);
	//p1zz = (p2x * p1y) - (p2y * p1x);

	p1xx = (p1y * p2z) - (p1z * p2y);
	p1yy = (p1z * p2x) - (p1x * p2z);
	p1zz = (p1x * p2y) - (p1y * p2x);

	p1 = AcGePoint3d::AcGePoint3d(p1x + p0x, p1y + p0y, p1z + p0z);
	p2 = AcGePoint3d::AcGePoint3d(p2x + p0x, p2y + p0y, p2z + p0z);

	AcDbLine* line = new AcDbLine(p0, p1);
	v3d = line->normal();

	p33 = AcGePoint3d::AcGePoint3d(v3d.x, v3d.y, v3d.z);

	p5 = AcGePoint3d::AcGePoint3d(p1xx + p0x, p1yy + p0y, p1zz + p0z);

	//var s = (Solid3d)tr.GetObject(id, OpenMode.ForWrite);
	AcGePlane Poly_plane = AcGePlane::AcGePlane(p0, p5, p2);

	AcDb3dSolid* negSolid = new AcDb3dSolid();

	Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal().negate());

	if (elementToConstruct.EntitiesHalf.size() > 0)
	{
		extrusion->getSlice(Poly_plane, elementToConstruct.AgreementHalf.at(0), negSolid);

		if (elementToConstruct.EntitiesHalf.size() > 0)
		{
			elementToConstruct.EntitiesHalf.erase(elementToConstruct.EntitiesHalf.begin());
			elementToConstruct.AgreementHalf.erase(elementToConstruct.AgreementHalf.begin());
		}
	}
	else
	{
		Acad::ErrorStatus es;

		AcGePoint3dArray ptArr;
		ptArr.setLogicalLength((int)((elementToConstruct.Args.front())));

		int i = 0;

		for (const auto& point : elementToConstruct.Points)
		{
			if (i == elementToConstruct.Args[0])
			{
				break;
			}

			ptArr[i].set(point.x() * -1.0f, point.y() * -1.0f, point.z() * -1.0f);
			acutPrintf(_T("point : [%f, %f, %f]\n"), point.x() * -1.0f, point.y() * -1.0f, point.z() * -1.0f);
			i++;
		}

		AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
			AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
		pNewPline->setColorIndex(3);

		//get the boundary curves of the polyline
		AcDbEntity* pEntity = NULL;
		if (pNewPline == NULL)
		{
			pEntity->close();
			return;
		}
		AcDbVoidPtrArray lines;
		pNewPline->explode(lines);
		pNewPline->close();

		// Create a region from the set of lines.
		AcDbVoidPtrArray regions;
		es = AcDbRegion::createFromCurves(lines, regions);
		if (Acad::eOk != es)
		{
			pNewPline->close();
			acutPrintf(L"\nFailed to create region\n");
			return;
		}

		AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);

		// Extrude the region to create a solid.
		AcDb3dSolid* pSolid = new AcDb3dSolid();
		es = pSolid->extrude(pRegion, vecteurExtrusion.z(), 0.0);

		for (int i = 0; i < lines.length(); i++)
		{
			delete (AcRxObject*)lines[i];
		}
		for (int ii = 0; ii < regions.length(); ii++)
		{
			delete (AcRxObject*)regions[ii];
		}

		DeplacementObjet3D(pSolid, elementToConstruct.LocationsPolygonal.front());

		AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
		/*AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		pBlockTableRecord->appendAcDbEntity(pSolid);
		pBlockTableRecord->close();*/

		pSolid->close();

		extrusion->getSlice(Poly_plane, elementToConstruct.AgreementPolygonal.at(0), pSolid);
		extrusion->booleanOper(AcDb::kBoolSubtract, pSolid);

		for (size_t i = 0; i < elementToConstruct.Args[0]; i++)
		{
			elementToConstruct.Points.pop_front();
		}

		elementToConstruct.Args.erase(elementToConstruct.Args.begin());
	}
}

void MethodeConstruction::drawForProfil(AcGePoint3dArray* ptArr[], AcGeVector3d* acVec3d[], int count, Vec3& vecteurExtrusion, Matrix4& transform)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcDbRegion* pRegion = nullptr;
	AcDbVoidPtrArray* lines = new AcDbVoidPtrArray[count];
	AcDbVoidPtrArray* regions = new AcDbVoidPtrArray[count];

	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();

	for (size_t i = 0; i < count; i++)
	{
		AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
			AcDb::k2dSimplePoly, *ptArr[i], 0.0, Adesk::kTrue);
		pNewPline->setColorIndex(3);

		if (acVec3d[i] != nullptr)
		{
			AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
			pNewPline->transformBy(matrix3d.translation(*acVec3d[i]));
		}

		//get the boundary curves of the polyline
		AcDbEntity* pEntity = nullptr;

		if (pNewPline == nullptr)
		{
			pEntity->close();
			return;
		}

		pNewPline->explode(lines[i]);
		pNewPline->close();

		// Create a region from the set of lines.
		es = AcDbRegion::createFromCurves(lines[i], regions[i]);

		if (Acad::eOk != es)
		{
			pNewPline->close();
			acutPrintf(L"\nFailed to create region\n");
			return;
		}

		if (i > 0)
			pRegion->booleanOper(AcDb::kBoolSubtract, AcDbRegion::cast((AcRxObject*)regions[i][0]));
		else
			pRegion = AcDbRegion::cast((AcRxObject*)regions[i][0]);
	}

	es = pSolid->extrude(pRegion, vecteurExtrusion.z(), 0.0);

	for (size_t i = 0; i < count; i++)
	{
		for (size_t j = 0; j < lines[i].length(); j++)
		{
			delete (AcRxObject*)lines[i][j];
		}

		for (int ii = 0; ii < regions[i].length(); ii++)
		{
			delete (AcRxObject*)regions[i][ii];
		}
	}

	delete[] lines;
	delete[] regions;

	DeplacementObjet3D(pSolid, transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	if (Acad::eOk == es)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		pBlockTableRecord->appendAcDbEntity(pSolid);
		pBlockTableRecord->close();
		pSolid->close();
	}
	else
	{
		delete pSolid;
	}
}

void MethodeConstruction::drawForProfilCircle(AcDbCircle* circle[], AcGeVector3d* acVec3d[], int count, Vec3& vecteurExtrusion, Matrix4& transform)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcDbRegion* pRegion = nullptr;
	AcDbVoidPtrArray* lines = new AcDbVoidPtrArray[count];
	AcDbVoidPtrArray* regions = new AcDbVoidPtrArray[count];

	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();

	for (size_t i = 0; i < count; i++)
	{
		if (acVec3d[i] != nullptr)
		{
			AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();
			circle[i]->transformBy(matrix3d.translation(*acVec3d[i]));
		}

		//get the boundary curves of the polyline
		AcDbEntity* pEntity = NULL;

		if (circle == NULL)
		{
			pEntity->close();
			return;
		}

		lines[i].append(circle);
		circle[i]->close();

		// Create a region from the set of lines.
		es = AcDbRegion::createFromCurves(lines[i], regions[i]);

		if (Acad::eOk != es)
		{
			circle[i]->close();
			acutPrintf(L"\nFailed to create region\n");
			return;
		}

		if (i > 0)
			pRegion->booleanOper(AcDb::kBoolSubtract, AcDbRegion::cast((AcRxObject*)regions[i][0]));
		else
			pRegion = AcDbRegion::cast((AcRxObject*)regions[i][0]);
	}

	es = pSolid->extrude(pRegion, vecteurExtrusion.z(), 0.0);

	for (size_t i = 0; i < count; i++)
	{
		for (int j = 0; j < lines[i].length(); j++)
		{
			delete (AcRxObject*)lines[i][j];
		}

		for (int ii = 0; ii < regions[i].length(); ii++)
		{
			delete (AcRxObject*)regions[i][ii];
		}
	}

	delete[] lines;
	delete[] regions;

	DeplacementObjet3D(pSolid, transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	if (Acad::eOk == es)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		pBlockTableRecord->appendAcDbEntity(pSolid);
		pBlockTableRecord->close();
		pSolid->close();
	}
	else
	{
		delete pSolid;
	}

	delete[] circle;
}

void MethodeConstruction::createSolid3dProfilIPE(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;
	float Width = IprofilDef.OverallWidth;
	float Depth = IprofilDef.OverallDepth;
	float WebThickness = IprofilDef.webThickness;
	float FlangeThickness = IprofilDef.flangeThickness;

	ptArrTemp.setLogicalLength(12);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, FlangeThickness, 0.0);
	ptArrTemp[3].set((Width + WebThickness) / 2, FlangeThickness, 0.0);
	ptArrTemp[4].set((Width + WebThickness) / 2, Depth - FlangeThickness, 0.0);
	ptArrTemp[5].set(Width, Depth - FlangeThickness, 0.0);
	ptArrTemp[6].set(Width, Depth, 0.0);
	ptArrTemp[7].set(0, Depth, 0.0);
	ptArrTemp[8].set(0, Depth - FlangeThickness, 0.0);
	ptArrTemp[9].set((Width - WebThickness) / 2, Depth - FlangeThickness, 0.0);
	ptArrTemp[10].set((Width - WebThickness) / 2, FlangeThickness, 0.0);
	ptArrTemp[11].set(0, FlangeThickness, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilIPN(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Width = IprofilDef.OverallWidth;
	float Depth = IprofilDef.OverallDepth;
	float WebThickness = IprofilDef.webThickness;
	float FlangeThickness = IprofilDef.flangeThickness;
	float FlangeSlope = IprofilDef.FlangeSlope;

	float dy = (Width - Width / 4) * tan(FlangeSlope);

	ptArrTemp.setLogicalLength(16);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, FlangeThickness - dy, 0.0);
	ptArrTemp[3].set(Width - Width / 4, FlangeThickness, 0.0);
	ptArrTemp[4].set((Width + WebThickness) / 2, FlangeThickness + dy, 0.0);
	ptArrTemp[5].set((Width + WebThickness) / 2, Depth - FlangeThickness - dy, 0.0);
	ptArrTemp[6].set(Width - Width / 4, Depth - FlangeThickness, 0.0);
	ptArrTemp[7].set(Width, Depth - FlangeThickness + dy, 0.0);
	ptArrTemp[8].set(Width, Depth, 0.0);
	ptArrTemp[9].set(0, Depth, 0.0);
	ptArrTemp[10].set(0, Depth - FlangeThickness + dy, 0.0);
	ptArrTemp[11].set(Width / 4, Depth - FlangeThickness, 0.0);
	ptArrTemp[12].set((Width - WebThickness) / 2, Depth - FlangeThickness - dy, 0.0);
	ptArrTemp[13].set((Width - WebThickness) / 2, FlangeThickness + dy, 0.0);
	ptArrTemp[14].set(Width / 4, FlangeThickness, 0.0);
	ptArrTemp[15].set(0, FlangeThickness - dy, 0.0);

	AcGePoint3dArray* ptArr[1];

	ptArr[0] = &ptArrTemp;

	drawForProfil(ptArr, nullptr, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilL8(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = LprofilDef.Depth;
	float Width = LprofilDef.Width;
	float Thickness = LprofilDef.Thickness;
	float FilletRadius = LprofilDef.FilletRadius;

	ptArrTemp.setLogicalLength(6);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, Thickness, 0.0);
	ptArrTemp[3].set(Thickness, Thickness, 0.0);
	ptArrTemp[4].set(Thickness, Depth, 0.0);
	ptArrTemp[5].set(0, Depth, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilL9(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = LprofilDef.Depth;
	float Width = LprofilDef.Width;
	float Thickness = LprofilDef.Thickness;
	float FilletRadius = LprofilDef.FilletRadius;
	float LegSlope = LprofilDef.LegSlope;

	double dy = (Width - Thickness) * tan(LegSlope);

	ptArrTemp.setLogicalLength(6);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, Thickness, 0.0);
	ptArrTemp[3].set(Thickness + dy, Thickness + dy, 0.0);
	ptArrTemp[4].set(Thickness, Depth, 0.0);
	ptArrTemp[5].set(0, Depth, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilT10(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = TprofilDef.Depth;
	float Width = TprofilDef.FlangeWidth;
	float WebThickness = TprofilDef.WebThickness;
	float FlangeThickness = TprofilDef.FlangeThickness;
	float FilletRadius = TprofilDef.FilletRadius;

	ptArrTemp.setLogicalLength(8);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, FlangeThickness, 0.0);
	ptArrTemp[3].set((Width + WebThickness) / 2, FlangeThickness, 0.0);
	ptArrTemp[4].set((Width + WebThickness) / 2, Depth, 0.0);
	ptArrTemp[5].set((Width - WebThickness) / 2, Depth, 0.0);
	ptArrTemp[6].set((Width - WebThickness) / 2, FlangeThickness, 0.0);
	ptArrTemp[7].set(0, FlangeThickness, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilT12(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = TprofilDef.Depth;
	float Width = TprofilDef.FlangeWidth;
	float WebThickness = TprofilDef.WebThickness;
	float FlangeThickness = TprofilDef.FlangeThickness;
	float FilletRadius = TprofilDef.FilletRadius;
	float WebSlope​ = TprofilDef.WebSlope;
	float FlangeSlope = TprofilDef.FlangeSlope;

	double dy1 = (Width - FlangeThickness) * tan(FlangeSlope);
	double dy2 = (Depth - WebThickness) * tan(WebSlope​);

	ptArrTemp.setLogicalLength(8);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, FlangeThickness - dy1, 0.0);
	ptArrTemp[3].set((Width + WebThickness) / 2, FlangeThickness, 0.0);
	ptArrTemp[4].set(((Width + WebThickness) / 2) - dy2, Depth, 0.0);
	ptArrTemp[5].set(((Width - WebThickness) / 2) + dy2, Depth, 0.0);
	ptArrTemp[6].set((Width - WebThickness) / 2, FlangeThickness, 0.0);
	ptArrTemp[7].set(0, FlangeThickness - dy1, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilUPE(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = UprofilDef.Depth;
	float Width = UprofilDef.FlangeWidth;
	float WebThickness = UprofilDef.WebThickness;
	float FlangeThickness = UprofilDef.FlangeThickness;
	float FilletRadius = UprofilDef.FilletRadius;

	ptArrTemp.setLogicalLength(8);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, FlangeThickness, 0.0);
	ptArrTemp[3].set(WebThickness, FlangeThickness, 0.0);
	ptArrTemp[4].set(WebThickness, Depth - FlangeThickness, 0.0);
	ptArrTemp[5].set(Width, Depth - FlangeThickness, 0.0);
	ptArrTemp[6].set(Width, Depth, 0.0);
	ptArrTemp[7].set(0, Depth, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilUPN(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = UprofilDef.Depth;
	float Width = UprofilDef.FlangeWidth;
	float WebThickness = UprofilDef.WebThickness;
	float FlangeThickness = UprofilDef.FlangeThickness;
	float FilletRadius = UprofilDef.FilletRadius;

	ptArrTemp.setLogicalLength(8);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, FlangeThickness, 0.0);
	ptArrTemp[3].set(WebThickness, FlangeThickness, 0.0);
	ptArrTemp[4].set(WebThickness, Depth - FlangeThickness, 0.0);
	ptArrTemp[5].set(Width, Depth - FlangeThickness, 0.0);
	ptArrTemp[6].set(Width, Depth, 0.0);
	ptArrTemp[7].set(0, Depth, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilC(C_profilDef CprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = CprofilDef.Depth;
	float Width = CprofilDef.Width;
	float WallThickness = CprofilDef.WallThickness;
	float Girth = CprofilDef.Depth;
	//float FilletRadius = 15;

	ptArrTemp.setLogicalLength(12);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(Width, 0, 0.0);
	ptArrTemp[2].set(Width, Girth, 0.0);
	ptArrTemp[3].set(Width - WallThickness, Girth, 0.0);
	ptArrTemp[4].set(Width - WallThickness, WallThickness, 0.0);
	ptArrTemp[5].set(WallThickness, WallThickness, 0.0);
	ptArrTemp[6].set(WallThickness, Depth - WallThickness, 0.0);
	ptArrTemp[7].set(Width - WallThickness, Depth - WallThickness, 0.0);
	ptArrTemp[8].set(Width - WallThickness, Depth - Girth, 0.0);
	ptArrTemp[9].set(Width, Depth - Girth, 0.0);
	ptArrTemp[10].set(Width, Depth, 0.0);
	ptArrTemp[11].set(0, Depth, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilZ(Z_profilDef ZprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float Depth = ZprofilDef.Depth;
	float Width = ZprofilDef.FlangeWidth;
	float WebThickness = ZprofilDef.WebThickness;
	float FlangeThickness = ZprofilDef.FlangeThickness;
	//float FilletRadius = 15;

	ptArrTemp.setLogicalLength(9);

	ptArrTemp[0].set(Width - WebThickness, 0, 0.0);
	ptArrTemp[1].set(Width - WebThickness + Width, 0, 0.0);
	ptArrTemp[2].set(Width - WebThickness + Width, FlangeThickness, 0.0);
	ptArrTemp[3].set(Width, FlangeThickness, 0.0);
	ptArrTemp[4].set(Width, Depth, 0.0);
	ptArrTemp[5].set(0, Depth, 0);
	ptArrTemp[6].set(0, Depth - FlangeThickness, 0.0);
	ptArrTemp[7].set(Width - WebThickness, Depth - FlangeThickness, 0.0);
	ptArrTemp[8].set(0, Depth, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width + WebThickness / 2, -Depth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilAsyI(AsymmetricI_profilDef AsymmetricIprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float BottomFlangeWidth = AsymmetricIprofilDef.OverallWidth;
	float OverallDepth = AsymmetricIprofilDef.OverallDepth;
	float WebThickness = AsymmetricIprofilDef.WebThickness;
	float BottomFlangeThickness = AsymmetricIprofilDef.FlangeThickness;
	float TopFlangeWidth = AsymmetricIprofilDef.TopFlangeWidth;
	float TopFlangeThickness = AsymmetricIprofilDef.TopFlangeThickness;
	float TopMinus = (BottomFlangeWidth - TopFlangeWidth) / 2;

	ptArrTemp.setLogicalLength(12);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(BottomFlangeWidth, 0, 0.0);
	ptArrTemp[2].set(BottomFlangeWidth, BottomFlangeThickness, 0.0);
	ptArrTemp[3].set((BottomFlangeWidth + WebThickness) / 2, BottomFlangeThickness, 0.0);
	ptArrTemp[4].set((BottomFlangeWidth + WebThickness) / 2, OverallDepth - TopFlangeThickness, 0.0);
	ptArrTemp[5].set(BottomFlangeWidth - TopMinus, OverallDepth - TopFlangeThickness, 0.0);
	ptArrTemp[6].set(BottomFlangeWidth - TopMinus, OverallDepth, 0.0);
	ptArrTemp[7].set(TopMinus, OverallDepth, 0.0);
	ptArrTemp[8].set(TopMinus, OverallDepth - TopFlangeThickness, 0.0);
	ptArrTemp[9].set((BottomFlangeWidth - WebThickness) / 2, OverallDepth - TopFlangeThickness, 0.0);
	ptArrTemp[10].set((BottomFlangeWidth - WebThickness) / 2, BottomFlangeThickness, 0.0);
	ptArrTemp[11].set(0, BottomFlangeThickness, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-BottomFlangeWidth / 2, -OverallDepth / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilCircHollow(CircleHollow_profilDef CircleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	float Radius = CircleHollowprofilDef.Radius;
	float WallThickness = CircleHollowprofilDef.WallThickness;

	AcGePoint3d center = AcGePoint3d::AcGePoint3d(Radius, Radius, 0);
	AcDbCircle* circles[2];
	AcGeVector3d* acVec3d1s[2];

	circles[0] = new AcDbCircle();
	circles[0]->setCenter(center);
	circles[0]->setRadius(Radius);
	circles[0]->setColorIndex(3);

	AcGePoint3d pointDeplacement3D1 = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius / 2, 0);
	auto temp = pointDeplacement3D1.asVector();
	acVec3d1s[0] = &temp;

	circles[1] = new AcDbCircle();
	circles[1]->setCenter(center);
	circles[1]->setRadius(Radius - WallThickness);
	circles[1]->setColorIndex(3);

	AcGePoint3d pointDeplacement3D2 = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius / 2, 0);
	temp = pointDeplacement3D2.asVector();
	acVec3d1s[1] = &temp;

	//drawForProfilCircle(circles, acVec3d1s, 2, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilRectHollow(RectangleHollow_profilDef RectangleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray* ptArr[2];
	AcGeVector3d* acVec3d1s[2];

	AcGePoint3dArray ptArr1;
	AcGePoint3dArray ptArr2;

	float XDim = RectangleHollowprofilDef.XDim;
	float YDim = RectangleHollowprofilDef.YDim;
	float WallThickness = RectangleHollowprofilDef.WallThickness;

	ptArr1.setLogicalLength(4);

	ptArr1[0].set(0, 0, 0.0);
	ptArr1[1].set(XDim, 0, 0.0);
	ptArr1[2].set(XDim, YDim, 0.0);
	ptArr1[3].set(0, YDim, 0.0);

	ptArr[0] = &ptArr1;
	
	AcGePoint3d pointDeplacement3D1 = AcGePoint3d::AcGePoint3d(-XDim / 2, -YDim / 2, 0);
	auto temp = pointDeplacement3D1.asVector();
	acVec3d1s[0] = &temp;

	ptArr2.setLogicalLength(4);

	ptArr2[0].set(WallThickness, WallThickness, 0.0);
	ptArr2[1].set(XDim - WallThickness, WallThickness, 0.0);
	ptArr2[2].set(XDim - WallThickness, YDim - WallThickness, 0.0);
	ptArr2[3].set(WallThickness, YDim - WallThickness, 0.0);

	ptArr[1] = &ptArr2;

	AcGePoint3d pointDeplacement3D2 = AcGePoint3d::AcGePoint3d(-XDim / 2, -YDim / 2, 0);
	temp = pointDeplacement3D2.asVector();
	acVec3d1s[1] = &temp;

	drawForProfil(ptArr, acVec3d1s, 2, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	float Radius = CircleprofilDef.Radius;

	AcGePoint3d center = AcGePoint3d::AcGePoint3d(Radius, Radius, 0);
	AcDbCircle* circles[1];
	AcGeVector3d* acVec3d1s[1];

	circles[0] = new AcDbCircle();
	circles[0]->setCenter(center);
	circles[0]->setRadius(Radius);
	circles[0]->setColorIndex(3);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius / 2, 0);
	auto temp = pointDeplacement3D.asVector();
	acVec3d1s[0] = &temp;

	//drawForProfilCircle(circles, acVec3d1s, 2, VecteurExtrusion, transform);
}

void MethodeConstruction::createSolid3dProfilRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	AcGePoint3dArray ptArrTemp;

	float XDim = RectangleprofilDef.XDim;
	float YDim = RectangleprofilDef.YDim;

	ptArrTemp.setLogicalLength(4);

	ptArrTemp[0].set(0, 0, 0.0);
	ptArrTemp[1].set(XDim, 0, 0.0);
	ptArrTemp[2].set(XDim, YDim, 0.0);
	ptArrTemp[3].set(0, YDim, 0.0);

	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-XDim / 2, -YDim / 2, 0);
	auto acVec3dTemp = pointDeplacement3D.asVector();

	AcGePoint3dArray* ptArr[1];
	AcGeVector3d* acVec3d[1];

	ptArr[0] = &ptArrTemp;
	acVec3d[0] = &acVec3dTemp;

	drawForProfil(ptArr, acVec3d, 1, VecteurExtrusion, transform);
}

void MethodeConstruction::CreationVoid(AcDb3dSolid* extrusion, ObjectVoid& objectVoid)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr;

	if (objectVoid.ElementsToConstruct.size() > 1 && objectVoid.ElementsToConstruct[0].Args[0] == 1)
	{
		objectVoid.ElementsToConstruct[0].Args.erase(objectVoid.ElementsToConstruct[0].Args.begin());
		objectVoid.ElementsToConstruct[0].Points.pop_front();

		if (objectVoid.ElementsToConstruct[0].Points.size() == 0)
			objectVoid.ElementsToConstruct.erase(objectVoid.ElementsToConstruct.begin());
	}

	ptArr.setLogicalLength(objectVoid.ElementsToConstruct[0].Args[0]);
	Vec3 pointOrigine = { objectVoid.Transform[12], objectVoid.Transform[13] , objectVoid.Transform[14] };

	int i = 0;

	for (const auto& point : objectVoid.ElementsToConstruct[0].Points)
	{
		if (i == objectVoid.ElementsToConstruct[0].Args[0])
		{
			break;
		}

		ptArr[i].set(point.x(), point.y(), point.z());

		i++;
	}

	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;
	if (pNewPline == NULL)
	{
		pEntity->close();
		return;
	}
	AcDbVoidPtrArray lines;
	pNewPline->explode(lines);
	pNewPline->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(lines, regions);
	if (Acad::eOk != es)
	{
		pNewPline->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}

	AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);

	// Extrude the region to create a solid.
	AcDb3dSolid* extrusion_void = new AcDb3dSolid();
	AcDb3dSolid* extrusion_void2 = new AcDb3dSolid();
	es = extrusion_void->extrude(pRegion, -(objectVoid.VecteurExtrusion.z()), 0.0);
	es = extrusion_void2->extrude(pRegion, objectVoid.VecteurExtrusion.z(), 0.0);
	//es = extrusion_void->extrude(pRegion, 10, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	for (int a = 1; a < objectVoid.ElementsToConstruct.size(); a++)
	{
		int nbPlan = objectVoid.ElementsToConstruct[a].Plans.size();
		for (size_t i = 0; i < nbPlan; i++)
		{
			if (objectVoid.ElementsToConstruct[i].Points.size() > 0 && objectVoid.ElementsToConstruct[i].Args.size() > 0)
			{
				CreationSection(extrusion_void, objectVoid.VecteurExtrusion, objectVoid.ElementsToConstruct[i]);
				CreationSection(extrusion_void2, objectVoid.VecteurExtrusion, objectVoid.ElementsToConstruct[i]);

				objectVoid.ElementsToConstruct[i].Plans.pop_front();
			}
		}
	}

	DeplacementObjet3D(extrusion_void, objectVoid.Transform);
	DeplacementObjet3D(extrusion_void2, objectVoid.Transform);

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

void MethodeConstruction::CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr2;

	float Radius = objectVoid.Radius;

	AcGePoint3d center = AcGePoint3d::AcGePoint3d(Radius, Radius, 0);

	/// <summary>
	/// Première polyline
	/// </summary>
	AcDbCircle* circle = new AcDbCircle();
	circle->setCenter(center);
	circle->setRadius(Radius);
	circle->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();
	circle->transformBy(matrix3d.translation(acVec3d));

	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;

	if (circle == NULL)
	{
		pEntity->close();
		return;
	}
	AcDbVoidPtrArray lines;
	lines.append(circle);

	//circle1->explode(lines1);
	circle->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(lines, regions);

	if (Acad::eOk != es)
	{
		circle->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}
	AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);



	// Extrude the region to create a solid.
	AcDb3dSolid* extrusion_void = new AcDb3dSolid();
	AcDb3dSolid* extrusion_void2 = new AcDb3dSolid();
	es = extrusion_void->extrude(pRegion, -(objectVoid.VecteurExtrusion.z()), 0.0);
	es = extrusion_void2->extrude(pRegion, objectVoid.VecteurExtrusion.z(), 0.0);
	//es = extrusion_void->extrude(pRegion, 10, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	for (int a = 1; a < objectVoid.ElementsToConstruct.size(); a++)
	{
		int nbPlan = objectVoid.ElementsToConstruct[a].Plans.size();
		for (size_t i = 0; i < nbPlan; i++)
		{
			if (objectVoid.ElementsToConstruct[i].Points.size() > 0 && objectVoid.ElementsToConstruct[i].Args.size() > 0)
			{
				CreationSection(extrusion_void, objectVoid.VecteurExtrusion, objectVoid.ElementsToConstruct[i]);
				CreationSection(extrusion_void2, objectVoid.VecteurExtrusion, objectVoid.ElementsToConstruct[i]);

				objectVoid.ElementsToConstruct[i].Plans.pop_front();
			}
		}
	}

	DeplacementObjet3D(extrusion_void, objectVoid.Transform);
	DeplacementObjet3D(extrusion_void2, objectVoid.Transform);

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

void MethodeConstruction::CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float XDim = objectVoid.XDim;
	float YDim = objectVoid.YDim;

	/// <summary>
	/// Première polyline
	/// </summary>
	ptArr.setLogicalLength(4);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(XDim, 0, 0.0);
	ptArr[2].set(XDim, YDim, 0.0);
	ptArr[3].set(0, YDim, 0.0);

	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-XDim / 2, -YDim / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();
	pNewPline->transformBy(matrix3d.translation(acVec3d));

	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;

	if (pNewPline == NULL)
	{
		pEntity->close();
		return;
	}
	AcDbVoidPtrArray lines;
	pNewPline->explode(lines);
	pNewPline->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(lines, regions);

	if (Acad::eOk != es)
	{
		pNewPline->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}
	AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);

	// Extrude the region to create a solid.
	AcDb3dSolid* extrusion_void = new AcDb3dSolid();
	AcDb3dSolid* extrusion_void2 = new AcDb3dSolid();
	es = extrusion_void->extrude(pRegion, -(objectVoid.VecteurExtrusion.z()), 0.0);
	es = extrusion_void2->extrude(pRegion, objectVoid.VecteurExtrusion.z(), 0.0);
	//es = extrusion_void->extrude(pRegion, 10, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	for (int a = 1; a < objectVoid.ElementsToConstruct.size(); a++)
	{
		int nbPlan = objectVoid.ElementsToConstruct[a].Plans.size();
		for (size_t i = 0; i < nbPlan; i++)
		{
			if (objectVoid.ElementsToConstruct[i].Points.size() > 0 && objectVoid.ElementsToConstruct[i].Args.size() > 0)
			{
				CreationSection(extrusion_void, objectVoid.VecteurExtrusion, objectVoid.ElementsToConstruct[i]);
				CreationSection(extrusion_void2, objectVoid.VecteurExtrusion, objectVoid.ElementsToConstruct[i]);

				objectVoid.ElementsToConstruct[i].Plans.pop_front();
			}
		}
	}

	DeplacementObjet3D(extrusion_void, objectVoid.Transform);
	DeplacementObjet3D(extrusion_void2, objectVoid.Transform);

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

AcDbRegion* MethodeConstruction::createPolyCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr2;

	float Radius = CircleprofilDef.Radius;

	AcGePoint3d center = AcGePoint3d::AcGePoint3d(Radius, Radius, 0);

	/// <summary>
	/// Première polyline
	/// </summary>
	AcDbCircle* circle1 = new AcDbCircle();
	circle1->setCenter(center);
	circle1->setRadius(Radius);
	circle1->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();
	circle1->transformBy(matrix3d.translation(acVec3d));

	//get the boundary curves of the polyline
	AcDbEntity* pEntity1 = NULL;

	/*if (circle1 == NULL)
	{
		pEntity1->close();
		return;
	}*/
	AcDbVoidPtrArray lines1;
	lines1.append(circle1);

	//circle1->explode(lines1);
	circle1->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions1;
	es = AcDbRegion::createFromCurves(lines1, regions1);

	/*if (Acad::eOk != es)
	{
		circle1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}*/
	AcDbRegion* pRegion1 = AcDbRegion::cast((AcRxObject*)regions1[0]);

	for (int i = 0; i < lines1.length(); i++)
	{
		delete (AcRxObject*)lines1[i];
	}
	for (int ii = 0; ii < regions1.length(); ii++)
	{
		delete (AcRxObject*)regions1[ii];
	}

	return pRegion1;
}

AcDbRegion* MethodeConstruction::createPolyRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr1;
	AcGePoint3dArray ptArr2;

	float XDim = RectangleprofilDef.XDim;
	float YDim = RectangleprofilDef.YDim;

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

	/*if (pNewPline1 == NULL)
	{
		pEntity1->close();
		return;
	}*/
	AcDbVoidPtrArray lines1;
	pNewPline1->explode(lines1);
	pNewPline1->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions1;
	es = AcDbRegion::createFromCurves(lines1, regions1);

	/*if (Acad::eOk != es)
	{
		pNewPline1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;

	}*/
	AcDbRegion* pRegion1 = AcDbRegion::cast((AcRxObject*)regions1[0]);

	for (int i = 0; i < lines1.length(); i++)
	{
		delete (AcRxObject*)lines1[i];
	}
	for (int ii = 0; ii < regions1.length(); ii++)
	{
		delete (AcRxObject*)regions1[ii];
	}
	return pRegion1;
}

AcDbRegion* MethodeConstruction::createCompositeCurve(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform)
{
	Acad::ErrorStatus es;

	ads_name polyName;

	ads_point ptres;

	AcGePoint3dArray ptArr;

	// Polyligne
	/*if (ListNbArg.size() > 1)
	{
		ListNbArg.erase(ListNbArg.begin());
		points1.pop_front();
	}

	ptArr.setLogicalLength(ListNbArg[0]);
	Vec3 pointOrigine = { transform1[12], transform1[13] , transform1[14] };

	int i = 0;

	for (const auto& point : points1)
	{
		if (i == ListNbArg[0])
		{
			break;
		}

		ptArr[i].set(point.x(), point.y(), point.z());

		i++;
	}

	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;
	if (pNewPline == NULL)
	{
		pEntity->close();
		return;
	}*/
	AcDbVoidPtrArray lines;
	//pNewPline->explode(lines);
	//pNewPline->close();


	//Arc
	/*
	AcDbArc* arc = new AcDbArc();
	arc->setCenter(center);
	arc->setRadius(radius);

	if (SenseAgreement)
	{
		arc->setStartAngle(270);
		arc->setEndAngle(90)
	}
	else {
		arc->setStartAngle(-270);
		arc->setEndAngle(-90)
	}*/

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(lines, regions);

	/*if (Acad::eOk != es)
	{
		circle1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}*/
	AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	return pRegion;
}
