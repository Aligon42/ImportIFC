#include "MethodeConstruction.h"
#include "CreateConstructionPointVisitor.h"

void createSolid3d(int key, std::list<Vec3> points1, std::vector<int> ListNbArg,
	Vec3 VecteurExtrusion, Matrix4 transform1, std::list<Matrix4> listPlan, 
	std::list<Matrix4> listLocationPolygonal, std::vector<bool> AgreementHalf, 
	std::vector<bool> AgreementPolygonal, std::vector<std::string> listEntityHalf, 
	std::vector<std::string> listEntityPolygonal, std::vector<ObjectVoid> listVoid)
{
    Acad::ErrorStatus es;

    ads_name polyName;

    ads_point ptres;

    AcGePoint3dArray ptArr;

	if (ListNbArg.size() > 1)
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

	int nbPlan = listPlan.size();


	for (int i = 0; i < ListNbArg[0]; i++)
	{
		points1.pop_front();
	}

	ListNbArg.erase(ListNbArg.begin());

	for (int a = 0; a < nbPlan; a++)
	{
			CreationSection(pSolid, VecteurExtrusion, points1, ListNbArg, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal);
	
			listPlan.pop_front();
	}

	

	DeplacementObjet3D(pSolid, transform1);

	for (int v = 0; v < listVoid.size(); v++)
	{
		if (key == listVoid[v].keyForVoid)
		{
			if (listVoid[v].NameProfilDef == "IfcArbitraryClosedProfileDef")
			{
				CreationVoid(pSolid, listVoid[v]);
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

static void CreationSection(AcDb3dSolid* extrusion, Vec3 VecteurExtrusion, std::list<Vec3>& points1,
	std::vector<int>& nbArg, std::list<Matrix4>& listPlan, std::list<Matrix4>& listLocationPolygonal,
	std::vector<bool>& AgreementHalf, std::vector<bool>& AgreementPolygonal,
	std::vector<std::string>& listEntityHalf, std::vector<std::string>& listEntityPolygonal)
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

	Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal().negate());
	
	if (listEntityHalf.size() > 0)
	{
		extrusion->getSlice(Poly_plane, AgreementHalf.at(0), negSolid);

		if (listEntityHalf.size() > 0)
		{
			listEntityHalf.erase(listEntityHalf.begin());
			AgreementHalf.erase(AgreementHalf.begin());
		}
	}
	else
	{
		Acad::ErrorStatus es;

		AcGePoint3dArray ptArr;
		ptArr.setLogicalLength((int)((nbArg.front())));

		int i = 0;

		for (const auto& point : points1)
		{
			if (i == nbArg[0])
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
		es = pSolid->extrude(pRegion, VecteurExtrusion.z(), 0.0);

		for (int i = 0; i < lines.length(); i++)
		{
			delete (AcRxObject*)lines[i];
		}
		for (int ii = 0; ii < regions.length(); ii++)
		{
			delete (AcRxObject*)regions[ii];
		}

		DeplacementObjet3D(pSolid, listLocationPolygonal.front());

		AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
		/*AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		pBlockTableRecord->appendAcDbEntity(pSolid);
		pBlockTableRecord->close();*/

		pSolid->close();

		extrusion->getSlice(Poly_plane, AgreementPolygonal.at(0), pSolid);
		extrusion->booleanOper(AcDb::kBoolSubtract, pSolid);

		for (size_t i = 0; i < nbArg[0]; i++)
		{
			points1.pop_front();
		}

		nbArg.erase(nbArg.begin());
	}
	
}

static void CreationVoid(AcDb3dSolid* extrusion, ObjectVoid Void)
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

		 for (int i = 0; i < lines.length(); i++)
		 {
			 delete (AcRxObject*)lines[i];
		 }
		 for (int ii = 0; ii < regions.length(); ii++)
		 {
			 delete (AcRxObject*)regions[ii];
		 }

	
	// Extrude the region to create a solid.
	AcDb3dSolid* extrusion_void = new AcDb3dSolid();
	es = extrusion_void->extrude(pRegion, Void.VecteurExtrusion.z(), 0.0);
	//es = extrusion_void->extrude(pRegion, 10, 0.0);

	

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
			CreationSection(extrusion_void, Void.VecteurExtrusion, Void.points1,
				Void.nbArg, Void.listPlan, Void.listLocationPolygonal, Void.AgreementHalf,
				Void.AgreementPolygonal, Void.listEntityHalf, Void.listEntityPolygonal);


			Void.listPlan.pop_front();
		}
	}

	DeplacementObjet3D(extrusion_void, Void.transform1);

	/*AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	AcDbDatabase* pDb = curDoc()->database();
	AcDbObjectId modelId;
	modelId = acdbSymUtil()->blockModelSpaceId(pDb);
	AcDbBlockTableRecord* pBlockTableRecord;
	acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
	pBlockTableRecord->appendAcDbEntity(extrusion_void);
	pBlockTableRecord->close();*/
	
	extrusion->booleanOper(AcDb::kBoolSubtract, extrusion_void);
	extrusion_void->close();
}


//*** ProfilDef ***

void createSolid3dProfil(BaseProfilDef* profilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{
	if (profilDef->Name == "IfcIShapeProfileDef")
	{
		if (((I_profilDef*)profilDef)->nbArg == 5)
		{
			createSolid3dProfilIPE(*(I_profilDef*)profilDef, VecteurExtrusion, transform1);
		}
		else
		{
			//createSolid3dProfilIPN(IprofilDef, VecteurExtrusion, transform1);
		}
	}
	else if (profilDef->Name == "IfcLShapeProfileDef")
	{
		if (((L_profilDef*)profilDef)->nbArg == 5)
		{
			createSolid3dProfilL8(*(L_profilDef*)profilDef, VecteurExtrusion, transform1);
		}
		else
		{
			createSolid3dProfilL9(*(L_profilDef*)profilDef, VecteurExtrusion, transform1);
		}
	}
	else if (profilDef->Name == "IfcTShapeProfileDef")
	{
		if (((T_profilDef*)profilDef)->nbArg == 7)
		{
			createSolid3dProfilT10(*(T_profilDef*)profilDef, VecteurExtrusion, transform1);
		}
		else
		{
			createSolid3dProfilT12(*(T_profilDef*)profilDef, VecteurExtrusion, transform1);
		}
	}
	else if (profilDef->Name == "IfcUShapeProfileDef")
	{
		if (((U_profilDef*)profilDef)->nbArg == 5)
		{
			createSolid3dProfilUPE(*(U_profilDef*)profilDef, VecteurExtrusion, transform1);
		}
		else
		{
			createSolid3dProfilUPN(*(U_profilDef*)profilDef, VecteurExtrusion, transform1);
		}
	}
	else if (profilDef->Name == "IfcCShapeProfileDef")
	{
		createSolid3dProfilC(*(C_profilDef*)profilDef, VecteurExtrusion, transform1);
	}
	else if (profilDef->Name == "IfcZShapeProfileDef")
	{
		createSolid3dProfilZ(*(Z_profilDef*)profilDef, VecteurExtrusion, transform1);
	}
	else if (profilDef->Name == "IfcAsymmetricIShapeProfileDef")
	{
		createSolid3dProfilAsyI(*(AsymmetricI_profilDef*)profilDef, VecteurExtrusion, transform1);
	}
	else if (profilDef->Name == "IfcCircleHollowProfileDef")
	{
		createSolid3dProfilCircHollow(*(CircleHollow_profilDef*)profilDef, VecteurExtrusion, transform1);
	}
	else if (profilDef->Name == "IfcRectangleHollowProfileDef")
	{
		createSolid3dProfilRectHollow(*(RectangleHollow_profilDef*)profilDef, VecteurExtrusion, transform1);
	}
	else if (profilDef->Name == "IfcCircleProfileDef")
	{
		createSolid3dProfilCircle(*(Circle_profilDef*)profilDef, VecteurExtrusion, transform1);
	}
	else if (profilDef->Name == "IfcRectangleProfileDef")
	{
		createSolid3dProfilRectangle(*(Rectangle_profilDef*)profilDef, VecteurExtrusion, transform1);
	}
}

void createSolid3dProfilIPE(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilIPN(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilL8(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilL9(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilT10(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilT12(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilUPE(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilUPN(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilC(C_profilDef CprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilZ(Z_profilDef ZprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{

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
	AcGePoint3d pointDeplacement3D = AcGePoint3d::AcGePoint3d(-Width + WebThickness/ 2, -Depth / 2, 0);
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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilAsyI(AsymmetricI_profilDef AsymmetricIprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1)
{


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

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilCircHollow(CircleHollow_profilDef CircleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1) {

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
	circle1->setColorIndex(3);

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


	AcDbCircle* circle2 = new AcDbCircle();
	circle2->setCenter(center);
	circle2->setRadius(Radius - WallThickness);
	circle2->setColorIndex(3);

	AcGeMatrix3d matrix3d2 = AcGeMatrix3d::AcGeMatrix3d();

	AcGePoint3d Pt3d2 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d pointDeplacement3D2 = AcGePoint3d::AcGePoint3d(-Radius / 2, -Radius	 / 2, 0);
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

	//circle1->explode(lines1);
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

	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	es = pSolid->extrude(pRegion1, VecteurExtrusion.z(), 0.0);

	for (int i = 0; i < lines1.length(); i++)
	{
		delete (AcRxObject*)lines1[i];
	}

	for (int ii = 0; ii < regions1.length(); ii++)
	{
		delete (AcRxObject*)regions1[ii];
	}

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilRectHollow(RectangleHollow_profilDef RectangleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1) {

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


	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	es = pSolid->extrude(pRegion1, VecteurExtrusion.z(), 0.0);

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

	DeplacementObjet3D(pSolid, transform1);


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

void createSolid3dProfilCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1) {

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

	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	es = pSolid->extrude(pRegion1, VecteurExtrusion.z(), 0.0);

	for (int i = 0; i < lines1.length(); i++)
	{
		delete (AcRxObject*)lines1[i];
	}

	for (int ii = 0; ii < regions1.length(); ii++)
	{
		delete (AcRxObject*)regions1[ii];
	}

	DeplacementObjet3D(pSolid, transform1);

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

void createSolid3dProfilRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1) {



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




	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	es = pSolid->extrude(pRegion1, VecteurExtrusion.z(), 0.0);



	for (int i = 0; i < lines1.length(); i++)
	{
		delete (AcRxObject*)lines1[i];
	}



	for (int ii = 0; ii < regions1.length(); ii++)
	{
		delete (AcRxObject*)regions1[ii];
	}


	DeplacementObjet3D(pSolid, transform1);


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

AcDbRegion* createPolyCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1) {

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

AcDbRegion* createPolyRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1) {



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

