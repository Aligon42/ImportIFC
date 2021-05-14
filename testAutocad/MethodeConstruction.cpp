#include "MethodeConstruction.h"

void createSolid3d(std::list<Vec3> points1, std::vector<int> ListNbArg, Vec3 VecteurExtrusion, Matrix4 tranform1, std::list<Matrix4> listPlan, std::list<Matrix4> listLocationPolygonal, bool Agreement)
{
    Acad::ErrorStatus es;

    ads_name polyName;

    ads_point ptres;

    AcGePoint3dArray ptArr;
    ptArr.setLogicalLength((int)((ListNbArg.at(1)) - 1));

    Vec3 pointOrigine = *points1.begin();
    points1.pop_front();


    int i = 0;

    for (const auto& point : points1)
    {
		if (i == ListNbArg.at(1) - 1)
		{
			break;
		}

		ptArr[i].set(point.x(), point.y(), point.z());

		i++;
    }
	
	std::vector<int> newlistArg;
	newlistArg.push_back(ListNbArg[1]);

	if (ListNbArg.size() > 2)
	{
		for (size_t y = 2; y < ListNbArg.size(); y++)
		{
			int itemList = ListNbArg[y];
			newlistArg.push_back(itemList);
		}
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
    es = pSolid->extrude(pRegion, VecteurExtrusion.z(), 0.0);

    for (int i = 0; i < lines.length(); i++)
    {
        delete (AcRxObject*)lines[i];
    }

    for (int ii = 0; ii < regions.length(); ii++)
    {
        delete (AcRxObject*)regions[ii];
    }

	for (int a = 0; a <= listPlan.size(); a++)
	{
		CreationSection(pSolid, VecteurExtrusion, points1, newlistArg, listPlan, listLocationPolygonal, Agreement);

	}
	DeplacementObjet3D(pSolid, tranform1);

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

static void CreationSection(AcDb3dSolid* extrusion, Vec3 VecteurExtrusion, std::list<Vec3> points1, std::vector<int> nbArg, std::list<Matrix4> listPlan, std::list<Matrix4> listLocationPolygonal, bool Agreement)
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





	//switch (PLanDeSection.ElementAt(i).s_Entity_second)
	//{


	//case "IFCPOLYGONALBOUNDEDHALFSPACE":
		//switch (PLanDeSection.ElementAt(i).methode)
		//{
		//case "IFCPOLYLINE":

			/*PromptStringOptions pStrOpts = new PromptStringOptions("\nCreation de fonction ");
			pStrOpts.AllowSpaces = true;
			PromptResult pStrRes = acDoc.Editor.GetString(pStrOpts);*/

	Acad::ErrorStatus es;

	AcGePoint3dArray ptArr;
	ptArr.setLogicalLength((int)((nbArg.front()) - 1));

	Vec3 pointOrigine = points1.front();

	for (size_t i = 1; i < nbArg[0]; i++)
	{
		points1.pop_front();
	}

	int i = 0;

	for (const auto& point : points1)
	{
		if (i == nbArg[1] - nbArg[0])
		{
			break;
		}

		ptArr[i].set(point.x() * -1, point.y() * -1, point.z() * -1);
		acutPrintf(_T("point : [%f, %f, %f]\n"), point.x() * -1, point.y() * -1, point.z() * -1);
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
	es = pSolid->extrude(pRegion, VecteurExtrusion.z(), 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	//DeplacementObjet3D(pSolid, listLocationPolygonal.front());
	acutPrintf(_T("Matrice : \n{ %f, %f, %f, %f,\n %f, %f, %f, %f,\n %f, %f, %f, %f,\n %f, %f, %f, %f,\n"),
		listLocationPolygonal.front()[0], listLocationPolygonal.front()[1], listLocationPolygonal.front()[2], listLocationPolygonal.front()[3],
		listLocationPolygonal.front()[4], listLocationPolygonal.front()[5], listLocationPolygonal.front()[6], listLocationPolygonal.front()[7],
		listLocationPolygonal.front()[8], listLocationPolygonal.front()[9], listLocationPolygonal.front()[10], listLocationPolygonal.front()[11],
		listLocationPolygonal.front()[12], listLocationPolygonal.front()[13], listLocationPolygonal.front()[14], listLocationPolygonal.front()[15]);


	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	AcDbDatabase* pDb = curDoc()->database();
	AcDbObjectId modelId;
	modelId = acdbSymUtil()->blockModelSpaceId(pDb);
	AcDbBlockTableRecord* pBlockTableRecord;
	acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
	pBlockTableRecord->appendAcDbEntity(pSolid);
	pBlockTableRecord->close();
	pSolid->close();
	

	////Coordonnées du repère du plan
	//p0x = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[0]); 
	p0x = listPlan.front()[12];  //x																		
	//p0y = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[1]);
	p0y = listPlan.front()[13]; //y
	//p0z = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[2]);
	p0z = listPlan.front()[14]; //z

	p0 = AcGePoint3d::AcGePoint3d(p0x, p0y, p0z);
	acutPrintf(_T("Coordonnées du repère du plan : [ %f, %f, %f]\n"), listPlan.front()[12], listPlan.front()[13], listPlan.front()[14]);


	///Direction1 plan
	//p1x = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[0]);
	p1x = listPlan.front()[8];  //x
	//p1y = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[1]);
	p1y = listPlan.front()[9]; //y
	//p1z = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[2]);
	p1z = listPlan.front()[10]; //z

	V1 = AcGeVector3d::AcGeVector3d(p1x, p1y, p1z);
	V2 = V1.normal();
	p3 = AcGePoint3d::AcGePoint3d(V2.x + p0x, V2.y + p0y, V2.z + p0z);
	acutPrintf(_T("Direction1 plan : [ %f, %f, %f]\n"), listPlan.front()[8], listPlan.front()[9], listPlan.front()[10]);

	///Direction2 plan
	//p2x = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[0]);
	p2x = listPlan.front()[0];  //x
	//p2y = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[1]);
	p2y = listPlan.front()[1]; //y
	//p2z = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[2]);
	p2z = listPlan.front()[2]; //z
	//Vector3d p2 = new Vector3d(p0x + p2x, p0y + p2y, p0z + p2z);

	acutPrintf(_T("Direction2 plan : [ %f, %f, %f]\n"), listPlan.front()[0], listPlan.front()[1], listPlan.front()[2]);

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
	
	extrusion->getSlice(Poly_plane, Agreement, pSolid);

	extrusion->booleanOper(AcDb::kBoolSubtract, pSolid);
	//extrusion.setColor(2);

	listPlan.pop_front();
}