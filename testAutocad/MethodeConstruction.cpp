#include "MethodeConstruction.h"
#include "CreateConstructionPointVisitor.h"
#include "tchar.h"
#include "tests.h"
#include <vector>
#include <iterator>


const wchar_t* GetWCM(const char* c, ...)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

void extrusion(int key, std::string entity, Object& object, std::vector<ObjectVoid>& listVoid, Style styleDessin)
{

	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((entity.c_str()));
	// Check to see if the layer exists

	if (entity == "IfcWallStandardCase")
	{
		layerName = _T("Mur");
	}
	if (entity == "IfcWall")
	{
		layerName = _T("Mur");
	}
	if (entity == "IfcSlab")
	{
		layerName = _T("Dalle");
	}
	if (entity == "IfcRoof")
	{
		layerName = _T("Toit");
	}
	if (entity == "IfcCovering")
	{
		layerName = _T("Revêtement");
	}
	if (entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}




	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record

		pLayerTableRecord->close();

	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	AcDbRegion* pRegion = nullptr;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr;
	AcDbVoidPtrArray lines;
	AcDbVoidPtrArray regions;

	if (object.OuterCurveName == "IfcCompositeCurve")
	{
		pRegion = createCompositeCurve(object.CompositeCurveSegment, object.Transform, object.IsMappedItem, object.TransformationOperator3D);
	}
	else if (object.OuterCurveName == "IfcPolyline")
	{
		if (object.ListNbArg.size() > 1)
		{
			object.ListNbArg.erase(object.ListNbArg.begin());
			object.Points.pop_front();
		}

		ptArr.setLogicalLength(object.ListNbArg[0]);
		Vec3 pointOrigine = { object.Transform[12], object.Transform[13] , object.Transform[14] };

		int i = 0;

		for (const auto& point : object.Points)
		{
			if (i == object.ListNbArg[0])
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

		pNewPline->explode(lines);
		pNewPline->close();

		// Create a region from the set of lines.

		es = AcDbRegion::createFromCurves(lines, regions);
		if (Acad::eOk != es)
		{
			pNewPline->close();
			acutPrintf(L"\nFailed to create region\n");
			return;
		}

		pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);
	}

	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)object.VecteurExtrusion.x() * object.HauteurExtrusion, (double)object.VecteurExtrusion.y() * object.HauteurExtrusion, (double)object.VecteurExtrusion.z() * object.HauteurExtrusion);
	AcDbSweepOptions options;
	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	//es = pSolid->extrude(pRegion, hauteurExtrusion, 0.0);
	es = pSolid->createExtrudedSolid(pRegion, vecExtru, options);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	int nbPlan = object.ListPlan.size();


	if (object.ListNbArg.size() > 0)
	{
		for (int i = 0; i < object.ListNbArg[0]; i++)
		{
			object.Points.pop_front();
		}
	}

	object.ListNbArg.erase(object.ListNbArg.begin());

	for (int a = 0; a < nbPlan; a++)
	{
		CreationSection(pSolid, object);

		object.ListPlan.pop_front();
	}

	if (object.IsMappedItem)
	{
		DeplacementObjet3DMappedItem(pSolid, object.TransformationOperator3D);
	}
	else
		DeplacementObjet3D(pSolid, object.Transform);

	for (int v = 0; v < listVoid.size(); v++)
	{
		if (key == listVoid[v].keyForVoid)
		{
			if (listVoid[v].NameProfilDef == "IfcArbitraryClosedProfileDef")
			{
				CreationVoid(pSolid, listVoid[v], object.CompositeCurveSegment, object.Transform, object.NbPolylineComposite, object.IsMappedItem, object.TransformationOperator3D);
				//listVoid.erase(listVoid.begin() + v);
			}
			else if (listVoid[v].NameProfilDef == "IfcCircleProfileDef")
			{
				CreationVoidCircle(pSolid, listVoid[v], object.CompositeCurveSegment, object.Transform, object.NbPolylineComposite, object.IsMappedItem, object.TransformationOperator3D);
				//listVoid.erase(listVoid.begin() + v);
			}
			else if (listVoid[v].NameProfilDef == "IfcRectangleProfileDef")
			{
				CreationVoidRectangle(pSolid, listVoid[v], object.CompositeCurveSegment, object.Transform, object.NbPolylineComposite, object.IsMappedItem, object.TransformationOperator3D);
				//listVoid.erase(listVoid.begin() + v);
			}
		}
	}

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

static void DeplacementObjet3D(AcDb3dSolid* pSolid, Matrix4 transform1) {

	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);

	double x1 = transform1.operator()(12);  //PointDeplacement x

	double y1 = transform1.operator()(13); //PointDeplacement y

	double z1 = transform1.operator()(14); //PointDeplacement z

	double x2 = transform1.operator()(8); //Direction1 x

	double y2 = transform1.operator()(9); //Direction1 y

	double z2 = transform1.operator()(10); //Direction1 z

	double x3 = transform1.operator()(0); //Direction2 x

	double y3 = transform1.operator()(1); //Direction2 y

	double z3 = transform1.operator()(2); //Direction2 z

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

	pSolid->transformBy(mat);

}

static void DeplacementObjet3D(AcDbSubDMesh* pSubDMesh, Matrix4 transform1) {

	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);

	double x1 = transform1.operator()(12);  //PointDeplacement x

	double y1 = transform1.operator()(13); //PointDeplacement y

	double z1 = transform1.operator()(14); //PointDeplacement z

	double x2 = transform1.operator()(8); //Direction1 x

	double y2 = transform1.operator()(9); //Direction1 y

	double z2 = transform1.operator()(10); //Direction1 z

	double x3 = transform1.operator()(0); //Direction2 x

	double y3 = transform1.operator()(1); //Direction2 y

	double z3 = transform1.operator()(2); //Direction2 z

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

	pSubDMesh->transformBy(mat);

}

static void DeplacementObjet3DMappedItem(AcDb3dSolid* pSolid, Matrix4 transformationOperator3D) {

	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);
	AcGePoint3d srcpt4 = AcGePoint3d::AcGePoint3d(0, 1, 0);

	double x1 = transformationOperator3D.operator()(3);  //PointDeplacement x

	double y1 = transformationOperator3D.operator()(7); //PointDeplacement y

	double z1 = transformationOperator3D.operator()(11); //PointDeplacement z

	double x2 = transformationOperator3D.operator()(0); //Direction1 x

	double y2 = transformationOperator3D.operator()(4); //Direction1 y

	double z2 = transformationOperator3D.operator()(8); //Direction1 z

	double x3 = transformationOperator3D.operator()(1); //Direction2 x

	double y3 = transformationOperator3D.operator()(5); //Direction2 y

	double z3 = transformationOperator3D.operator()(9); //Direction2 z

	double x4 = transformationOperator3D.operator()(2); //Direction3 x

	double y4 = transformationOperator3D.operator()(6); //Direction3 y

	double z4 = transformationOperator3D.operator()(10); //Direction3 z

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

	pSolid->transformBy(mat);

}

static void DeplacementObjet3DMappedItem(AcDbSubDMesh* pSubDMesh, Matrix4 transformationOperator3D) {

	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);
	AcGePoint3d srcpt4 = AcGePoint3d::AcGePoint3d(0, 1, 0);

	double x1 = transformationOperator3D.operator()(3);  //PointDeplacement x

	double y1 = transformationOperator3D.operator()(7); //PointDeplacement y

	double z1 = transformationOperator3D.operator()(11); //PointDeplacement z

	double x2 = transformationOperator3D.operator()(0); //Direction1 x

	double y2 = transformationOperator3D.operator()(4); //Direction1 y

	double z2 = transformationOperator3D.operator()(8); //Direction1 z

	double x3 = transformationOperator3D.operator()(1); //Direction2 x

	double y3 = transformationOperator3D.operator()(5); //Direction2 y

	double z3 = transformationOperator3D.operator()(9); //Direction2 z

	double x4 = transformationOperator3D.operator()(2); //Direction3 x

	double y4 = transformationOperator3D.operator()(6); //Direction3 y

	double z4 = transformationOperator3D.operator()(10); //Direction3 z

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

static void CreationSection(AcDb3dSolid* extrusion, Object object)
{

	AcGeVector3d v1 = AcGeVector3d::AcGeVector3d(0, 0, 0);             // Vector 1 (x,y,z) & Vector 2 (x,y,z)
	AcGeVector3d v2 = AcGeVector3d::AcGeVector3d(0, 0, 0);
	AcGeVector3d normal = AcGeVector3d::AcGeVector3d(0, 0, 0);


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


	AcGeVector3d v3d = AcGeVector3d::AcGeVector3d();

	AcGePoint3d p33 = AcGePoint3d::AcGePoint3d();

	AcGePoint3d p5 = AcGePoint3d::AcGePoint3d();

	////Coordonnées du repère du plan

	p0x = object.ListPlan.front()[12];  //x																		
	p0y = object.ListPlan.front()[13]; //y
	p0z = object.ListPlan.front()[14]; //z

	p0 = AcGePoint3d::AcGePoint3d(p0x, p0y, p0z);
	acutPrintf(_T("Coordonnées du repère du plan : [ %f, %f, %f]\n"), object.ListPlan.front()[12], object.ListPlan.front()[13], object.ListPlan.front()[14]);


	///Direction1 plan
	p1x = object.ListPlan.front()[8];  //x
	p1y = object.ListPlan.front()[9]; //y
	p1z = object.ListPlan.front()[10]; //z

	V1 = AcGeVector3d::AcGeVector3d(p1x, p1y, p1z);
	V2 = V1.normal();
	p3 = AcGePoint3d::AcGePoint3d(V2.x + p0x, V2.y + p0y, V2.z + p0z);
	acutPrintf(_T("Direction1 plan : [ %f, %f, %f]\n"), object.ListPlan.front()[8], object.ListPlan.front()[9], object.ListPlan.front()[10]);

	///Direction2 plan
	p2x = object.ListPlan.front()[0];  //x
	p2y = object.ListPlan.front()[1]; //y
	p2z = object.ListPlan.front()[2]; //z

	acutPrintf(_T("Direction2 plan : [ %f, %f, %f]\n"), object.ListPlan.front()[0], object.ListPlan.front()[1], object.ListPlan.front()[2]);

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

	AcGePlane Poly_plane = AcGePlane::AcGePlane(p0, p5, p2);

	AcDb3dSolid* negSolid = new AcDb3dSolid();

	Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal().negate());

	if (object.ListEntityHalf.size() > 0)
	{
		extrusion->getSlice(Poly_plane, object.AgreementHalf.at(0), negSolid);

		if (object.ListEntityHalf.size() > 0)
		{
			object.ListEntityHalf.erase(object.ListEntityHalf.begin());
			object.AgreementHalf.erase(object.AgreementHalf.begin());
		}
	}
	else
	{
		Acad::ErrorStatus es;

		AcGePoint3dArray ptArr;
		ptArr.setLogicalLength((int)((object.ListNbArg.front())));

		int i = 0;

		for (const auto& point : object.Points)
		{
			if (i == object.ListNbArg[0])
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

		AcDbRegion* pRegion;

		if (object.CompositeCurveSegment.listPolyligne.size() > 0 || object.CompositeCurveSegment.listTrimmedCurve.size() > 0 ||
			object.CompositeCurveSegment.listParentCurve.size() > 0)
			pRegion = createCompositeCurve(object.CompositeCurveSegment, object.Transform, object.IsMappedItem, object.TransformationOperator3D);
		else
			pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);

		AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)object.VecteurExtrusion.x() * object.HauteurExtrusion, (double)object.VecteurExtrusion.y() * object.HauteurExtrusion, (double)object.VecteurExtrusion.z() * object.HauteurExtrusion);
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

		if (object.IsMappedItem)
			DeplacementObjet3DMappedItem(pSolid, object.TransformationOperator3D);
		else
			DeplacementObjet3D(pSolid, object.ListLocationPolygonal.front());

		AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

		pSolid->close();

		extrusion->getSlice(Poly_plane, object.AgreementPolygonal.at(0), pSolid);
		extrusion->booleanOper(AcDb::kBoolSubtract, pSolid);

		if (object.NbPolylineComposite > 0)
		{
			for (int j = 0; j < object.NbPolylineComposite; j++)
			{
				for (size_t i = 0; i < object.ListNbArg[0]; i++)
				{
					object.Points.pop_front();
				}

				object.ListNbArg.erase(object.ListNbArg.begin());
			}
		}
		else
		{
			for (size_t i = 0; i < object.ListNbArg[0]; i++)
			{
				object.Points.pop_front();
			}

			object.ListNbArg.erase(object.ListNbArg.begin());
		}


	}

}

static void CreationVoid(AcDb3dSolid* extrusion, ObjectVoid Void, CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, int nbPolylineComposite, bool isMappedItem, Matrix4 transformationOperator3D)
{
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr;



	if (Void.nbArg.size() > 1)
	{
		Void.nbArg.erase(Void.nbArg.begin());
		Void.points1.pop_front();
	}

	ptArr.setLogicalLength(Void.nbArg[0]);
	Vec3 pointOrigine = { Void.transform1[12], Void.transform1[13] , Void.transform1[14] };

	int i = 0;

	for (const auto& point : Void.points1)
	{
		if (i == Void.nbArg[0])
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




	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)Void.VecteurExtrusion.x() * Void.hauteurExtrusion, (double)Void.VecteurExtrusion.y() * Void.hauteurExtrusion, (double)Void.VecteurExtrusion.z() * Void.hauteurExtrusion);
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

	int nbPlan = Void.listPlan.size();

	for (int a = 0; a < nbPlan; a++)
	{
		for (size_t i = 0; i < Void.nbArg[0]; i++)
		{
			Void.points1.pop_front();
		}

		Void.nbArg.erase(Void.nbArg.begin());

		if (Void.points1.size() > 0 && Void.nbArg.size() > 0)
		{
			Object object;
			object.VecteurExtrusion = Void.VecteurExtrusion;
			object.HauteurExtrusion = Void.hauteurExtrusion;
			object.Points = Void.points1;
			object.ListNbArg = Void.nbArg;
			object.ListPlan = Void.listPlan;
			object.ListLocationPolygonal = Void.listLocationPolygonal;
			object.AgreementHalf = Void.AgreementHalf;
			object.AgreementPolygonal = Void.AgreementPolygonal;
			object.ListEntityHalf = Void.listEntityHalf;
			object.ListEntityHalf = Void.listEntityPolygonal;
			object.CompositeCurveSegment = _compositeCurveSegment;
			object.Transform = transform;
			object.NbPolylineComposite = nbPolylineComposite;
			object.IsMappedItem = isMappedItem;
			object.TransformationOperator3D = transformationOperator3D;

			CreationSection(extrusion_void, object);
			CreationSection(extrusion_void2, object);

			Void.listPlan.pop_front();
		}
	}

	if (isMappedItem)
		DeplacementObjet3DMappedItem(extrusion_void, transformationOperator3D);
	else
		DeplacementObjet3D(extrusion_void, Void.transform1);

	if (isMappedItem)
		DeplacementObjet3DMappedItem(extrusion_void2, transformationOperator3D);
	else
		DeplacementObjet3D(extrusion_void2, Void.transform1);

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

static void CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid Void, CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, int nbPolylineComposite, bool isMappedItem, Matrix4 transformationOperator3D)
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;
	AcGePoint3dArray ptArr2;

	float Radius = Void.radius;

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
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(0, 0, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)Void.VecteurExtrusion.x() * Void.hauteurExtrusion, (double)Void.VecteurExtrusion.y() * Void.hauteurExtrusion, (double)Void.VecteurExtrusion.z() * Void.hauteurExtrusion);
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

	int nbPlan = Void.listPlan.size();

	for (int a = 0; a < nbPlan; a++)
	{
		for (size_t i = 0; i < Void.nbArg[0]; i++)
		{
			Void.points1.pop_front();
		}

		Void.nbArg.erase(Void.nbArg.begin());

		if (Void.points1.size() > 0 && Void.nbArg.size() > 0)
		{
			Object object;
			object.VecteurExtrusion = Void.VecteurExtrusion;
			object.HauteurExtrusion = Void.hauteurExtrusion;
			object.Points = Void.points1;
			object.ListNbArg = Void.nbArg;
			object.ListPlan = Void.listPlan;
			object.ListLocationPolygonal = Void.listLocationPolygonal;
			object.AgreementHalf = Void.AgreementHalf;
			object.AgreementPolygonal = Void.AgreementPolygonal;
			object.ListEntityHalf = Void.listEntityHalf;
			object.ListEntityHalf = Void.listEntityPolygonal;
			object.CompositeCurveSegment = _compositeCurveSegment;
			object.Transform = transform;
			object.NbPolylineComposite = nbPolylineComposite;
			object.IsMappedItem = isMappedItem;
			object.TransformationOperator3D = transformationOperator3D;

			CreationSection(extrusion_void, object);
			CreationSection(extrusion_void2, object);

			Void.listPlan.pop_front();
		}
	}

	if (isMappedItem)
		DeplacementObjet3DMappedItem(extrusion_void, transformationOperator3D);
	else
		DeplacementObjet3D(extrusion_void, Void.transform1);

	if (isMappedItem)
		DeplacementObjet3DMappedItem(extrusion_void2, transformationOperator3D);
	else
		DeplacementObjet3D(extrusion_void2, Void.transform1);

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

static void CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid Void, CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, int nbPolylineComposite, bool isMappedItem, Matrix4 transformationOperator3D)
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float XDim = Void.XDim;
	float YDim = Void.YDim;

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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)Void.VecteurExtrusion.x() * Void.hauteurExtrusion, (double)Void.VecteurExtrusion.y() * Void.hauteurExtrusion, (double)Void.VecteurExtrusion.z() * Void.hauteurExtrusion);
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

	int nbPlan = Void.listPlan.size();

	for (int a = 0; a < nbPlan; a++)
	{
		for (size_t i = 0; i < Void.nbArg[0]; i++)
		{
			Void.points1.pop_front();
		}

		Void.nbArg.erase(Void.nbArg.begin());

		if (Void.points1.size() > 0 && Void.nbArg.size() > 0)
		{
			Object object;
			object.VecteurExtrusion = Void.VecteurExtrusion;
			object.HauteurExtrusion = Void.hauteurExtrusion;
			object.Points = Void.points1;
			object.ListNbArg = Void.nbArg;
			object.ListPlan = Void.listPlan;
			object.ListLocationPolygonal = Void.listLocationPolygonal;
			object.AgreementHalf = Void.AgreementHalf;
			object.AgreementPolygonal = Void.AgreementPolygonal;
			object.ListEntityHalf = Void.listEntityHalf;
			object.ListEntityHalf = Void.listEntityPolygonal;
			object.CompositeCurveSegment = _compositeCurveSegment;
			object.Transform = transform;
			object.NbPolylineComposite = nbPolylineComposite;
			object.IsMappedItem = isMappedItem;
			object.TransformationOperator3D = transformationOperator3D;

			CreationSection(extrusion_void, object);
			CreationSection(extrusion_void2, object);

			Void.listPlan.pop_front();
		}
	}

	if (isMappedItem)
		DeplacementObjet3DMappedItem(extrusion_void, transformationOperator3D);
	else
		DeplacementObjet3D(extrusion_void, Void.transform1);

	if (isMappedItem)
		DeplacementObjet3DMappedItem(extrusion_void2, transformationOperator3D);
	else
		DeplacementObjet3D(extrusion_void2, Void.transform1);

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

AcDbRegion* createCompositeCurve(CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, bool isMappedItem, Matrix4 transformationOperator3D)
{
	AcDbVoidPtrArray lines;
	Acad::ErrorStatus es;
	AcGePoint3dArray ptArr;

	int sizeListPolyligne = _compositeCurveSegment.listPolyligne.size();

	for (int i = 0; i < sizeListPolyligne; i++)
	{
		ptArr.setLogicalLength(_compositeCurveSegment.listPolyligne[0].size());
		Vec3 pointOrigine = { transform[12], transform[13] , transform[14] };



		int j = 0;

		for (const auto& point : _compositeCurveSegment.listPolyligne[0])
		{
			if (j == _compositeCurveSegment.listPolyligne[0].size())
			{
				break;
			}
			ptArr[j].set(roundoff(point.x(), 2), roundoff(point.y(), 2), roundoff(point.z(), 2));

			j++;
		}
		AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
			AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kFalse);
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
		_compositeCurveSegment.listPolyligne.erase(_compositeCurveSegment.listPolyligne.begin());
	}


	//Arc
	AcDbArc* arc = new AcDbArc();

	for (int i = 0; i < _compositeCurveSegment.listTrimmedCurve.size(); i++)
	{

		float pointX = roundoff(_compositeCurveSegment.listTrimmedCurve[i].centreCircle.x(), 2);
		float pointY = roundoff(_compositeCurveSegment.listTrimmedCurve[i].centreCircle.y(), 2);
		float radius = roundoff(_compositeCurveSegment.listTrimmedCurve[i].radius, 2);

		AcGePoint3d center = AcGePoint3d::AcGePoint3d(pointX, pointY, 0);
		arc->setCenter(center);
		arc->setRadius(_compositeCurveSegment.listTrimmedCurve[i].radius);

		if (_compositeCurveSegment.listTrimmedCurve[i].senseArgreement)
		{
			arc->setStartAngle(((_compositeCurveSegment.listTrimmedCurve[i].trim1) * PI) / 180);
			arc->setEndAngle(((_compositeCurveSegment.listTrimmedCurve[i].trim2) * PI) / 180);
		}
		else {
			arc->setStartAngle(((-(_compositeCurveSegment.listTrimmedCurve[i].trim1) * PI) / 180));
			arc->setEndAngle(((-(_compositeCurveSegment.listTrimmedCurve[i].trim2) * PI) / 180));
		}

		lines.append(arc);

		arc->close();
		_compositeCurveSegment.listTrimmedCurve.erase(_compositeCurveSegment.listTrimmedCurve.begin());
	}

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(lines, regions);

	if (Acad::eOk != es)
	{
		AcDbRegion* pRegionFail = nullptr;
		acutPrintf(L"\nFailed to create region\n");
		return pRegionFail;
	}
	AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);

	_compositeCurveSegment.listParentCurve.clear();

	return pRegion;
}

//*** ProfilDef ***

void createSolid3dProfilIPE(I_profilDef IprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((IprofilDef.Entity.c_str()));
	// Check to see if the layer exists
	if (IprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (IprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (IprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (IprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (IprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Width = IprofilDef.OverallWidth;
	float Depth = IprofilDef.OverallDepth;
	float WebThickness = IprofilDef.webThickness;
	float FlangeThickness = IprofilDef.flangeThickness;

	ptArr.setLogicalLength(12);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, FlangeThickness, 0.0);
	ptArr[3].set((Width + WebThickness) / 2, FlangeThickness, 0.0);
	ptArr[4].set((Width + WebThickness) / 2, Depth - FlangeThickness, 0.0);
	ptArr[5].set(Width, Depth - FlangeThickness, 0.0);
	ptArr[6].set(Width, Depth, 0.0);
	ptArr[7].set(0, Depth, 0.0);
	ptArr[8].set(0, Depth - FlangeThickness, 0.0);
	ptArr[9].set((Width - WebThickness) / 2, Depth - FlangeThickness, 0.0);
	ptArr[10].set((Width - WebThickness) / 2, FlangeThickness, 0.0);
	ptArr[11].set(0, FlangeThickness, 0.0);


	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)IprofilDef.VecteurExtrusion.x() * IprofilDef.HauteurExtrusion, (double)IprofilDef.VecteurExtrusion.y() * IprofilDef.HauteurExtrusion, (double)IprofilDef.VecteurExtrusion.z() * IprofilDef.HauteurExtrusion);
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

	if (IprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, IprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, IprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilIPN(I_profilDef IprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((IprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (IprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (IprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (IprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (IprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (IprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Width = IprofilDef.OverallWidth;
	float Depth = IprofilDef.OverallDepth;
	float WebThickness = IprofilDef.webThickness;
	float FlangeThickness = IprofilDef.flangeThickness;
	float FlangeSlope = IprofilDef.FlangeSlope;

	float dy = (Width - Width / 4) * tan(FlangeSlope);
	double pi = PI;


	ptArr.setLogicalLength(16);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, FlangeThickness - dy, 0.0);
	ptArr[3].set(Width - Width / 4, FlangeThickness, 0.0);
	ptArr[4].set((Width + WebThickness) / 2, FlangeThickness + dy, 0.0);
	ptArr[5].set((Width + WebThickness) / 2, Depth - FlangeThickness - dy, 0.0);
	ptArr[6].set(Width - Width / 4, Depth - FlangeThickness, 0.0);
	ptArr[7].set(Width, Depth - FlangeThickness + dy, 0.0);
	ptArr[8].set(Width, Depth, 0.0);
	ptArr[9].set(0, Depth, 0.0);
	ptArr[10].set(0, Depth - FlangeThickness + dy, 0.0);
	ptArr[11].set(Width / 4, Depth - FlangeThickness, 0.0);
	ptArr[12].set((Width - WebThickness) / 2, Depth - FlangeThickness - dy, 0.0);
	ptArr[13].set((Width - WebThickness) / 2, FlangeThickness + dy, 0.0);
	ptArr[14].set(Width / 4, FlangeThickness, 0.0);
	ptArr[15].set(0, FlangeThickness - dy, 0.0);


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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)IprofilDef.VecteurExtrusion.x() * IprofilDef.HauteurExtrusion, (double)IprofilDef.VecteurExtrusion.y() * IprofilDef.HauteurExtrusion, (double)IprofilDef.VecteurExtrusion.z() * IprofilDef.HauteurExtrusion);
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

	if (IprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, IprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, IprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilL8(L_profilDef LprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((LprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (LprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (LprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (LprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (LprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (LprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = LprofilDef.Depth;
	float Width = LprofilDef.Width;
	float Thickness = LprofilDef.Thickness;
	float FilletRadius = LprofilDef.FilletRadius;

	ptArr.setLogicalLength(6);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, Thickness, 0.0);
	ptArr[3].set(Thickness, Thickness, 0.0);
	ptArr[4].set(Thickness, Depth, 0.0);
	ptArr[5].set(0, Depth, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);


	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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

	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)LprofilDef.VecteurExtrusion.x() * LprofilDef.HauteurExtrusion, (double)LprofilDef.VecteurExtrusion.y() * LprofilDef.HauteurExtrusion, (double)LprofilDef.VecteurExtrusion.z() * LprofilDef.HauteurExtrusion);
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

	if (LprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, LprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, LprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilL9(L_profilDef LprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((LprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (LprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (LprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (LprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (LprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (LprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = LprofilDef.Depth;
	float Width = LprofilDef.Width;
	float Thickness = LprofilDef.Thickness;
	float FilletRadius = LprofilDef.FilletRadius;
	float LegSlope = LprofilDef.LegSlope;

	double dy = (Width - Thickness) * tan(LegSlope);

	ptArr.setLogicalLength(6);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, Thickness, 0.0);
	ptArr[3].set(Thickness + dy, Thickness + dy, 0.0);
	ptArr[4].set(Thickness, Depth, 0.0);
	ptArr[5].set(0, Depth, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)LprofilDef.VecteurExtrusion.x() * LprofilDef.HauteurExtrusion, (double)LprofilDef.VecteurExtrusion.y() * LprofilDef.HauteurExtrusion, (double)LprofilDef.VecteurExtrusion.z() * LprofilDef.HauteurExtrusion);
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

	if (LprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, LprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, LprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilT10(T_profilDef TprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((TprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (TprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (TprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (TprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (TprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (TprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = TprofilDef.Depth;
	float Width = TprofilDef.FlangeWidth;
	float WebThickness = TprofilDef.WebThickness;
	float FlangeThickness = TprofilDef.FlangeThickness;
	float FilletRadius = TprofilDef.FilletRadius;

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, FlangeThickness, 0.0);
	ptArr[3].set((Width + WebThickness) / 2, FlangeThickness, 0.0);
	ptArr[4].set((Width + WebThickness) / 2, Depth, 0.0);
	ptArr[5].set((Width - WebThickness) / 2, Depth, 0.0);
	ptArr[6].set((Width - WebThickness) / 2, FlangeThickness, 0.0);
	ptArr[7].set(0, FlangeThickness, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);


	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)TprofilDef.VecteurExtrusion.x() * TprofilDef.HauteurExtrusion, (double)TprofilDef.VecteurExtrusion.y() * TprofilDef.HauteurExtrusion, (double)TprofilDef.VecteurExtrusion.z() * TprofilDef.HauteurExtrusion);
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

	if (TprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, TprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, TprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilT12(T_profilDef TprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((TprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (TprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (TprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (TprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (TprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (TprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = TprofilDef.Depth;
	float Width = TprofilDef.FlangeWidth;
	float WebThickness = TprofilDef.WebThickness;
	float FlangeThickness = TprofilDef.FlangeThickness;
	float FilletRadius = TprofilDef.FilletRadius;
	float WebSlope​ = TprofilDef.WebSlope;
	float FlangeSlope = TprofilDef.FlangeSlope;

	double dy1 = (Width - FlangeThickness) * tan(FlangeSlope);
	double dy2 = (Depth - WebThickness) * tan(WebSlope​);

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, FlangeThickness - dy1, 0.0);
	ptArr[3].set((Width + WebThickness) / 2, FlangeThickness, 0.0);
	ptArr[4].set(((Width + WebThickness) / 2) - dy2, Depth, 0.0);
	ptArr[5].set(((Width - WebThickness) / 2) + dy2, Depth, 0.0);
	ptArr[6].set((Width - WebThickness) / 2, FlangeThickness, 0.0);
	ptArr[7].set(0, FlangeThickness - dy1, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)TprofilDef.VecteurExtrusion.x() * TprofilDef.HauteurExtrusion, (double)TprofilDef.VecteurExtrusion.y() * TprofilDef.HauteurExtrusion, (double)TprofilDef.VecteurExtrusion.z() * TprofilDef.HauteurExtrusion);
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

	if (TprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, TprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, TprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilUPE(U_profilDef UprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((UprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (UprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (UprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (UprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (UprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (UprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = UprofilDef.Depth;
	float Width = UprofilDef.FlangeWidth;
	float WebThickness = UprofilDef.WebThickness;
	float FlangeThickness = UprofilDef.FlangeThickness;
	float FilletRadius = UprofilDef.FilletRadius;

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, FlangeThickness, 0.0);
	ptArr[3].set(WebThickness, FlangeThickness, 0.0);
	ptArr[4].set(WebThickness, Depth - FlangeThickness, 0.0);
	ptArr[5].set(Width, Depth - FlangeThickness, 0.0);
	ptArr[6].set(Width, Depth, 0.0);
	ptArr[7].set(0, Depth, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)UprofilDef.VecteurExtrusion.x() * UprofilDef.HauteurExtrusion, (double)UprofilDef.VecteurExtrusion.y() * UprofilDef.HauteurExtrusion, (double)UprofilDef.VecteurExtrusion.z() * UprofilDef.HauteurExtrusion);
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

	if (UprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, UprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, UprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilUPN(U_profilDef UprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((UprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (UprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (UprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (UprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (UprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (UprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = UprofilDef.Depth;
	float Width = UprofilDef.FlangeWidth;
	float WebThickness = UprofilDef.WebThickness;
	float FlangeThickness = UprofilDef.FlangeThickness;
	float FilletRadius = UprofilDef.FilletRadius;

	ptArr.setLogicalLength(8);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, FlangeThickness, 0.0);
	ptArr[3].set(WebThickness, FlangeThickness, 0.0);
	ptArr[4].set(WebThickness, Depth - FlangeThickness, 0.0);
	ptArr[5].set(Width, Depth - FlangeThickness, 0.0);
	ptArr[6].set(Width, Depth, 0.0);
	ptArr[7].set(0, Depth, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)UprofilDef.VecteurExtrusion.x() * UprofilDef.HauteurExtrusion, (double)UprofilDef.VecteurExtrusion.y() * UprofilDef.HauteurExtrusion, (double)UprofilDef.VecteurExtrusion.z() * UprofilDef.HauteurExtrusion);
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

	if (UprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, UprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, UprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilC(C_profilDef CprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((CprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (CprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (CprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (CprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (CprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (CprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = CprofilDef.Depth;
	float Width = CprofilDef.Width;
	float WallThickness = CprofilDef.WallThickness;
	float Girth = CprofilDef.Depth;
	//float FilletRadius = 15;

	ptArr.setLogicalLength(12);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(Width, 0, 0.0);
	ptArr[2].set(Width, Girth, 0.0);
	ptArr[3].set(Width - WallThickness, Girth, 0.0);
	ptArr[4].set(Width - WallThickness, WallThickness, 0.0);
	ptArr[5].set(WallThickness, WallThickness, 0.0);
	ptArr[6].set(WallThickness, Depth - WallThickness, 0.0);
	ptArr[7].set(Width - WallThickness, Depth - WallThickness, 0.0);
	ptArr[8].set(Width - WallThickness, Depth - Girth, 0.0);
	ptArr[9].set(Width, Depth - Girth, 0.0);
	ptArr[10].set(Width, Depth, 0.0);
	ptArr[11].set(0, Depth, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)CprofilDef.VecteurExtrusion.x() * CprofilDef.HauteurExtrusion, (double)CprofilDef.VecteurExtrusion.y() * CprofilDef.HauteurExtrusion, (double)CprofilDef.VecteurExtrusion.z() * CprofilDef.HauteurExtrusion);
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

	if (CprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, CprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, CprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilZ(Z_profilDef ZprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((ZprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (ZprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (ZprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (ZprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (ZprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (ZprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = ZprofilDef.Depth;
	float Width = ZprofilDef.FlangeWidth;
	float WebThickness = ZprofilDef.WebThickness;
	float FlangeThickness = ZprofilDef.FlangeThickness;
	//float FilletRadius = 15;

	ptArr.setLogicalLength(9);

	ptArr[0].set(Width - WebThickness, 0, 0.0);
	ptArr[1].set(Width - WebThickness + Width, 0, 0.0);
	ptArr[2].set(Width - WebThickness + Width, FlangeThickness, 0.0);
	ptArr[3].set(Width, FlangeThickness, 0.0);
	ptArr[4].set(Width, Depth, 0.0);
	ptArr[5].set(0, Depth, 0);
	ptArr[6].set(0, Depth - FlangeThickness, 0.0);
	ptArr[7].set(Width - WebThickness, Depth - FlangeThickness, 0.0);
	ptArr[8].set(0, Depth, 0.0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width + WebThickness / 2, -Depth / 2, 0);
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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)ZprofilDef.VecteurExtrusion.x() * ZprofilDef.HauteurExtrusion, (double)ZprofilDef.VecteurExtrusion.y() * ZprofilDef.HauteurExtrusion, (double)ZprofilDef.VecteurExtrusion.z() * ZprofilDef.HauteurExtrusion);
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

	if (ZprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, ZprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, ZprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilAsyI(AsymmetricI_profilDef AsymmetricIprofilDef, Style styleDessin)
{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((AsymmetricIprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (AsymmetricIprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (AsymmetricIprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (AsymmetricIprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (AsymmetricIprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (AsymmetricIprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float BottomFlangeWidth = AsymmetricIprofilDef.OverallWidth;
	float OverallDepth = AsymmetricIprofilDef.OverallDepth;
	float WebThickness = AsymmetricIprofilDef.WebThickness;
	float BottomFlangeThickness = AsymmetricIprofilDef.FlangeThickness;
	float TopFlangeWidth = AsymmetricIprofilDef.TopFlangeWidth;
	float TopFlangeThickness = AsymmetricIprofilDef.TopFlangeThickness;
	float TopMinus = (BottomFlangeWidth - TopFlangeWidth) / 2;



	ptArr.setLogicalLength(12);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(BottomFlangeWidth, 0, 0.0);
	ptArr[2].set(BottomFlangeWidth, BottomFlangeThickness, 0.0);
	ptArr[3].set((BottomFlangeWidth + WebThickness) / 2, BottomFlangeThickness, 0.0);
	ptArr[4].set((BottomFlangeWidth + WebThickness) / 2, OverallDepth - TopFlangeThickness, 0.0);
	ptArr[5].set(BottomFlangeWidth - TopMinus, OverallDepth - TopFlangeThickness, 0.0);
	ptArr[6].set(BottomFlangeWidth - TopMinus, OverallDepth, 0.0);
	ptArr[7].set(TopMinus, OverallDepth, 0.0);
	ptArr[8].set(TopMinus, OverallDepth - TopFlangeThickness, 0.0);
	ptArr[9].set((BottomFlangeWidth - WebThickness) / 2, OverallDepth - TopFlangeThickness, 0.0);
	ptArr[10].set((BottomFlangeWidth - WebThickness) / 2, BottomFlangeThickness, 0.0);
	ptArr[11].set(0, BottomFlangeThickness, 0.0);


	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);


	//get the boundary curves of the polyline
	AcDbEntity* pEntity = NULL;

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-BottomFlangeWidth / 2, -OverallDepth / 2, 0);
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();
	pNewPline->transformBy(matrix3d.translation(acVec3d));

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


	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)AsymmetricIprofilDef.VecteurExtrusion.x() * AsymmetricIprofilDef.HauteurExtrusion, (double)AsymmetricIprofilDef.VecteurExtrusion.y() * AsymmetricIprofilDef.HauteurExtrusion, (double)AsymmetricIprofilDef.VecteurExtrusion.z() * AsymmetricIprofilDef.HauteurExtrusion);
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

	if (AsymmetricIprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, AsymmetricIprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, AsymmetricIprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilCircHollow(CircleHollow_profilDef CircleHollowprofilDef, Style styleDessin) {

	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((CircleHollowprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (CircleHollowprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (CircleHollowprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (CircleHollowprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (CircleHollowprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (CircleHollowprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;


	AcGePoint3dArray ptArr2;

	float Radius = CircleHollowprofilDef.Radius;
	float WallThickness = CircleHollowprofilDef.WallThickness;

	AcGePoint3d center = AcGePoint3d::AcGePoint3d(Radius, Radius, 0);
	/// <summary>
	/// Première polyline
	/// </summary>


	AcDbCircle* circle1 = new AcDbCircle();
	circle1->setCenter(center);
	circle1->setRadius(Radius);

	AcGeMatrix3d matrix3d1 = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D1 = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius / 2, 0);
	AcGeVector3d acVec3d1 = pointDeplacement3D1.asVector();
	circle1->transformBy(matrix3d1.translation(acVec3d1));

	//get the boundary curves of the polyline
	AcDbEntity* pEntity1 = NULL;

	if (circle1 == NULL)
	{
		pEntity1->close();
		return;
	}
	AcDbVoidPtrArray lines1;
	lines1.append(circle1);

	circle1->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions1;
	es = AcDbRegion::createFromCurves(lines1, regions1);

	if (Acad::eOk != es)
	{
		circle1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;

	}
	AcDbRegion* pRegion1 = AcDbRegion::cast((AcRxObject*)regions1[0]);


	AcDbCircle* circle2 = new AcDbCircle();
	circle2->setCenter(center);
	circle2->setRadius(Radius - WallThickness);

	AcGeMatrix3d matrix3d2 = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d2 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D2 = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius / 2, 0);
	AcGeVector3d acVec3d2 = pointDeplacement3D2.asVector();
	circle2->transformBy(matrix3d2.translation(acVec3d2));

	//get the boundary curves of the polyline
	AcDbEntity* pEntity2 = NULL;

	if (circle2 == NULL)
	{
		pEntity2->close();
		return;
	}
	AcDbVoidPtrArray lines2;
	lines2.append(circle2);

	circle2->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions2;
	es = AcDbRegion::createFromCurves(lines2, regions2);

	if (Acad::eOk != es)
	{
		circle1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;

	}
	AcDbRegion* pRegion2 = AcDbRegion::cast((AcRxObject*)regions2[0]);

	pRegion1->booleanOper(AcDb::kBoolSubtract, pRegion2);

	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)CircleHollowprofilDef.VecteurExtrusion.x() * CircleHollowprofilDef.HauteurExtrusion, (double)CircleHollowprofilDef.VecteurExtrusion.y() * CircleHollowprofilDef.HauteurExtrusion, (double)CircleHollowprofilDef.VecteurExtrusion.z() * CircleHollowprofilDef.HauteurExtrusion);
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

	if (CircleHollowprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, CircleHollowprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, CircleHollowprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilRectHollow(RectangleHollow_profilDef RectangleHollowprofilDef, Style styleDessin) {

	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((RectangleHollowprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (RectangleHollowprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (RectangleHollowprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (RectangleHollowprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (RectangleHollowprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (RectangleHollowprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr1;
	AcGePoint3dArray ptArr2;

	float XDim = RectangleHollowprofilDef.XDim;
	float YDim = RectangleHollowprofilDef.YDim;
	float WallThickness = RectangleHollowprofilDef.WallThickness;

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

	AcGeMatrix3d matrix3d1 = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D1 = AcGePoint3d::AcGePoint3d(-XDim / 2, -YDim / 2, 0);
	AcGeVector3d acVec3d1 = pointDeplacement3D1.asVector();
	pNewPline1->transformBy(matrix3d1.translation(acVec3d1));

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
	es = AcDbRegion::createFromCurves(lines1, regions1);

	if (Acad::eOk != es)
	{
		pNewPline1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;

	}
	AcDbRegion* pRegion1 = AcDbRegion::cast((AcRxObject*)regions1[0]);



	/// <summary>
	/// Deuxième polyline
	/// </summary>
	ptArr2.setLogicalLength(4);

	ptArr2[0].set(WallThickness, WallThickness, 0.0);
	ptArr2[1].set(XDim - WallThickness, WallThickness, 0.0);
	ptArr2[2].set(XDim - WallThickness, YDim - WallThickness, 0.0);
	ptArr2[3].set(WallThickness, YDim - WallThickness, 0.0);

	AcDb2dPolyline* pNewPline2 = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr2, 0.0, Adesk::kTrue);
	pNewPline2->setColorIndex(3);

	AcGeMatrix3d matrix3d2 = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d2 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D2 = AcGePoint3d::AcGePoint3d(-XDim / 2, -YDim / 2, 0);
	AcGeVector3d acVec3d2 = pointDeplacement3D2.asVector();
	pNewPline2->transformBy(matrix3d2.translation(acVec3d2));

	//get the boundary curves of the polyline
	AcDbEntity* pEntity2 = NULL;

	if (pNewPline2 == NULL)
	{
		pEntity2->close();
		return;
	}
	AcDbVoidPtrArray lines2;
	pNewPline2->explode(lines2);
	pNewPline2->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions2;
	es = AcDbRegion::createFromCurves(lines2, regions2);

	if (Acad::eOk != es)
	{
		pNewPline2->close();
		acutPrintf(L"\nFailed to create region\n");
		return;

	}
	AcDbRegion* pRegion2 = AcDbRegion::cast((AcRxObject*)regions2[0]);


	pRegion1->booleanOper(AcDb::kBoolSubtract, pRegion2);

	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)RectangleHollowprofilDef.VecteurExtrusion.x() * RectangleHollowprofilDef.HauteurExtrusion, (double)RectangleHollowprofilDef.VecteurExtrusion.y() * RectangleHollowprofilDef.HauteurExtrusion, (double)RectangleHollowprofilDef.VecteurExtrusion.z() * RectangleHollowprofilDef.HauteurExtrusion);
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

	for (int y = 0; y < lines2.length(); y++)
	{
		delete (AcRxObject*)lines2[y];
	}

	for (int yy = 0; yy < regions2.length(); yy++)
	{
		delete (AcRxObject*)regions2[yy];
	}

	if (RectangleHollowprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, RectangleHollowprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, RectangleHollowprofilDef.Transform);


	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilCircle(Circle_profilDef CircleprofilDef, Style styleDessin) {

	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((CircleprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (CircleprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (CircleprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (CircleprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (CircleprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (CircleprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
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

	if (circle1 == NULL)
	{
		pEntity1->close();
		return;
	}
	AcDbVoidPtrArray lines1;
	lines1.append(circle1);

	//circle1->explode(lines1);
	circle1->close();

	// Create a region from the set of lines.
	AcDbVoidPtrArray regions1;
	es = AcDbRegion::createFromCurves(lines1, regions1);

	if (Acad::eOk != es)
	{
		circle1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}
	AcDbRegion* pRegion1 = AcDbRegion::cast((AcRxObject*)regions1[0]);

	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)CircleprofilDef.VecteurExtrusion.x() * CircleprofilDef.HauteurExtrusion, (double)CircleprofilDef.VecteurExtrusion.y() * CircleprofilDef.HauteurExtrusion, (double)CircleprofilDef.VecteurExtrusion.z() * CircleprofilDef.HauteurExtrusion);
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

	if (CircleprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, CircleprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, CircleprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createSolid3dProfilRectangle(Rectangle_profilDef RectangleprofilDef, Style styleDessin) {

	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((RectangleprofilDef.Entity.c_str()));
	// Check to see if the layer exists

	if (RectangleprofilDef.Entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	if (RectangleprofilDef.Entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	if (RectangleprofilDef.Entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	if (RectangleprofilDef.Entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	if (RectangleprofilDef.Entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}

	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
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
	es = AcDbRegion::createFromCurves(lines1, regions1);

	if (Acad::eOk != es)
	{
		pNewPline1->close();
		acutPrintf(L"\nFailed to create region\n");
		return;
	}
	AcDbRegion* pRegion1 = AcDbRegion::cast((AcRxObject*)regions1[0]);

	AcGeVector3d vecExtru = AcGeVector3d::AcGeVector3d((double)RectangleprofilDef.VecteurExtrusion.x() * RectangleprofilDef.HauteurExtrusion, (double)RectangleprofilDef.VecteurExtrusion.y() * RectangleprofilDef.HauteurExtrusion, (double)RectangleprofilDef.VecteurExtrusion.z() * RectangleprofilDef.HauteurExtrusion);
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

	if (RectangleprofilDef.IsMappedItem)
		DeplacementObjet3DMappedItem(pSolid, RectangleprofilDef.TransformationOperator3D);
	else
		DeplacementObjet3D(pSolid, RectangleprofilDef.Transform);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	pSolid->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSolid->setTransparency(transparence);

	pSolid->setLayer(layerName, Adesk::kFalse, false);
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

void createBoundingBox(Box box, std::string entity, Style styleDessin) {

	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((entity.c_str()));
	// Check to see if the layer exists


	if (entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	else if (entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	else if (entity == "IfcWallStandardCase")
	{
		layerName = _T("Mur");
	}
	else if (entity == "IfcWall")
	{
		layerName = _T("Mur");
	}
	else if (entity == "IfcSlab")
	{
		layerName = _T("Dalle");
	}
	else if (entity == "IfcRoof")
	{
		layerName = _T("Toit");
	}
	else if (entity == "IfcCovering")
	{
		layerName = _T("Revêtement");
	}
	else if (entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	else if (entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	else if (entity == "IfcBuildingElementProxy")
	{
		layerName = _T("Proxy");
	}
	else if (entity == "IfcDoor")
	{
		layerName = _T("Porte");
	}
	else if (entity == "IfcSite")
	{
		layerName = _T("Site");
	}
	else if (entity == "IfcStair")
	{
		layerName = _T("Escalier");
	}
	else if (entity == "IfcRailling")
	{
		layerName = _T("Balustrade");
	}
	else if (entity == "IfcWindow")
	{
		layerName = _T("Fenêtre");
	}
	if (entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}


	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}
	pLayerTable->close();
	Acad::ErrorStatus es;

	// Extrude the region to create a solid.

	AcDb3dSolid* box3d = new AcDb3dSolid();

	es = box3d->createBox(box.XDimBox, box.YDimBox, box.ZDimBox);  /// Creation de la box au point 0,0,0 avec les dimensions

	AcGeMatrix3d matrix3d = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(box.Corner.x(), box.Corner.y(), box.Corner.z());
	AcGeVector3d acVec3d = pointDeplacement3D.asVector();
	box3d->transformBy(matrix3d.translation(acVec3d));

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(styleDessin.red * 255, styleDessin.green * 255, styleDessin.blue * 255);
	box3d->setColor(couleurRGB, false);

	double opa = abs((styleDessin.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	box3d->setTransparency(transparence);

	box3d->setLayer(layerName, Adesk::kFalse, false);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	if (Acad::eOk == es)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);

		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);



		pBlockTableRecord->appendAcDbEntity(box3d);
		pBlockTableRecord->close();
		box3d->close();
	}

}

void createFaceSolid(std::string entity, Object& object, std::map<int, Style>* listStyles) {

	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	const ACHAR* layerName = GetWCM((entity.c_str()));
	// Check to see if the layer exists


	if (entity == "IfcBeam")
	{
		layerName = _T("Poutre");
	}
	else if (entity == "IfcColumn")
	{
		layerName = _T("Colonne");
	}
	else if (entity == "IfcWallStandardCase")
	{
		layerName = _T("Mur");
	}
	else if (entity == "IfcWall")
	{
		layerName = _T("Mur");
	}
	else if (entity == "IfcSlab")
	{
		layerName = _T("Dalle");
	}
	else if (entity == "IfcRoof")
	{
		layerName = _T("Toit");
	}
	else if (entity == "IfcCovering")
	{
		layerName = _T("Revêtement");
	}
	else if (entity == "IfcPlate")
	{
		layerName = _T("Plaque");
	}
	else if (entity == "IfcFooting")
	{
		layerName = _T("Pied");
	}
	else if (entity == "IfcBuildingElementProxy")
	{
		layerName = _T("Proxy");
	}
	else if (entity == "IfcDoor")
	{
		layerName = _T("Porte");
	}
	else if (entity == "IfcSite")
	{
		layerName = _T("Site");
	}
	else if (entity == "IfcStair")
	{
		layerName = _T("Escalier");
	}
	else if (entity == "IfcRailling")
	{
		layerName = _T("Balustrade");
	}
	else if (entity == "IfcWindow")
	{
		layerName = _T("Fenêtre");
	}
	if (entity == "IfcMappedItem")
	{
		layerName = _T("Element");
	}


	if (!pLayerTable->has(layerName))
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(layerName);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTableRecord->close();
	}

	pLayerTable->close();
	Acad::ErrorStatus es = Acad::eInvalidInput;
	AcArray<Adesk::Int32> faceArray;
	AcDbSubDMesh* pSubDMesh = new AcDbSubDMesh();
	AcDb3dSolid* pSolid = new AcDb3dSolid();

	AcGePoint3dArray ptArr;
	int k = 0;

	////test
	//int indexNbArg = 0;
	//std::vector<int> listIndexNbArg;
	//int arg = ListNbArg[0];
	//int arg2 = 0;
	//std::vector<int> newListNbArg;
	//newListNbArg.push_back(arg);
	//std::vector<Vec3> newPoints;
	//std::vector<std::vector<Vec3>> listPointsFace;
	//std::vector<Vec3> pontPoints;
	//auto iterator = pontPoints.begin();
	//int newIterator = 0;
	//int differentesFaces = 1;
	//int sizePoints = points1.size();
	//int sizePontPoints = pontPoints.size();

	//for (int i = 0; i < sizePoints; i++)
	//{
	//	pontPoints.push_back(points1.front());
	//	points1.pop_front();
	//}

	//for (int i = 0; i < ListNbArg.size(); i++)
	//{
	//	if (ListNbArg[0] != ListNbArg[i + 1])
	//	{
	//		differentesFaces++;
	//	}
	//}

	//for (int j = 0; j < sizePontPoints; j++)
	//{
	//	newPoints.push_back(pontPoints.front());
	//	pontPoints.erase(pontPoints.begin());
	//}

	//for (int face = 0; face < differentesFaces; face++)
	//{
	//	std::vector<Vec3> points;
	//	for (int i = 1; i < ListNbArg.size(); i++)
	//	{
	//		if (arg = ListNbArg[i])
	//		{
	//			for (int j = 0; j < ListNbArg[i]; j++)
	//			{
	//				points.push_back(newPoints.at(newIterator));
	//				newPoints.erase(iterator);
	//			}
	//			indexNbArg++;
	//			
	//		}
	//		else
	//		{
	//			for (int it = 0; it < ListNbArg[i]; it++)
	//			{
	//				iterator++;
	//				newIterator++;
	//			}
	//			if (i = sizeNbArg - 1)
	//			{
	//				arg2 = ListNbArg[i];
	//			}
	//		}
	//	}
	//	listPointsFace.push_back(points);
	//	arg = arg2;
	//	newListNbArg.push_back(arg);
	//}
	//fin test

	Style style;

	if (object.KeyItems.size() > 0)
	{
		if (listStyles->count(object.KeyItems[0]))
		{
			style = listStyles->at(object.KeyItems[0]);
			listStyles->erase(object.KeyItems[0]);
		}
	}

	AcCmColor couleurRGB = AcCmColor::AcCmColor();
	couleurRGB.setRGB(style.red * 255, style.green * 255, style.blue * 255);

	pSubDMesh->setColor(couleurRGB, false, pDb);

	double opa = abs((style.transparence * 255) - 255);
	Adesk::UInt8 alpha = opa;
	AcCmTransparency transparence = AcCmTransparency::AcCmTransparency(alpha);
	pSubDMesh->setTransparency(transparence);

	int argsUsed = 0;
	int offset = 0;
	auto pointIterator = object.Points.cbegin();
	for (int i = 0; i < object.ListNbArgFace.size(); i++)
	{
		argsUsed = object.ListNbArgFace[i];
		int totalRange = offset + argsUsed;

		for (int p = offset; p < totalRange; p++)
		{
			const auto& point = *pointIterator++;
			AcGePoint3d point3d = AcGePoint3d::AcGePoint3d(point.x(), point.y(), point.z());
			ptArr.append(point3d);
		}

		faceArray.append(object.ListNbArgFace[i]);
		for (int j = 0; j < object.ListNbArgFace[i]; j++)
		{
			faceArray.append(k);
			k++;
		}

		es = pSubDMesh->setSubDMesh(ptArr, faceArray, 0);
		if (es != Acad::eOk)
		{
			pSubDMesh->close();
			acutPrintf(L"\nFailed to set \n");
			return;
		}

		if (object.IsMappedItem)
		{
			DeplacementObjet3DMappedItem(pSubDMesh, object.TransformationOperator3D);
		}
		else
			DeplacementObjet3D(pSubDMesh, object.TransformFace);

		pSubDMesh->setLayer(layerName, Adesk::kFalse, false);

		offset += object.ListNbArgFace[i];
	}

	if (es == Acad::eOk)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		pBlockTableRecord->appendAcDbEntity(pSubDMesh);
		pBlockTableRecord->close();
		pSubDMesh->close();
	}
	else
	{
		delete pSolid;
	}
}

float roundoff(float value, unsigned char prec)
{
	float pow_10 = pow(10.0f, (float)prec);
	return round(value * pow_10) / pow_10;
}
