#include "DessinExemple.h"
#include "tchar.h"
#include "aced.h"
#include "rxregsvc.h" 
#include "dbapserv.h"
#include "dbents.h"
#include "dbsol3d.h"
#include "dbregion.h"
#include "dbsymutl.h"
#include "dbplanesurf.h"
#include "AcApDMgr.h"
#include <Windows.h>
#include <string.h>
#include <iostream>
#include <math.h> 

using namespace std;





static void makeLayer()

{
	// Open the Layer table for read
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbLayerTable* pLayerTable;
	pDb->getLayerTable(pLayerTable, AcDb::kForRead);

	// Check to see if the layer exists
	if (pLayerTable->has(_T("OBJ")) == false)
	{
		// Open the Layer table for write
		pLayerTable->upgradeOpen();

		// Create the new layer and assign it the name 'OBJ'
		AcDbLayerTableRecord* pLayerTableRecord = new AcDbLayerTableRecord();
		pLayerTableRecord->setName(_T("OBJ"));

		// Set the color of the layer to cyan
		AcCmColor color;
		color.setColorIndex(4);
		pLayerTableRecord->setColor(color);

		// Add the new layer to the Layer table
		pLayerTable->add(pLayerTableRecord);

		// Close the Layer table and record
		pLayerTable->close();
		pLayerTableRecord->close();
	}
}

static void addLine()

{
	// Get the current database
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();

	// Open the Block Table for read-only
	AcDbBlockTable* pBlockTable;
	pDb->getSymbolTable(pBlockTable, AcDb::kForRead);



	// Get the Model Space block
	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();


	//Définition des points de la ligne
	AcGePoint3d startPt(7.0, 3.0, 0.0);
	AcGePoint3d endPt(11.0, 3.0, 0.0);

	// Création de l'objet Line
	AcDbLine* pLine = new AcDbLine(startPt, endPt);

	pBlockTableRecord->appendAcDbEntity(pLine);
	pBlockTableRecord->close();
	pLine->close();

}

static void listObjects()

{
	// Get the current database
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();

	// Get the current space object
	AcDbBlockTableRecord* pBlockTableRecord;
	Acad::ErrorStatus es = acdbOpenObject(pBlockTableRecord, pDb->currentSpaceId(), AcDb::kForRead);

	// Create a new block iterator that will be used to
	// step through each object in the current space
	AcDbBlockTableRecordIterator* pItr;
	pBlockTableRecord->newIterator(pItr);

	// Create a variable AcDbEntity type which is a generic
	// object to represent a Line, Circle, Arc, among other objects

	AcDbEntity* pEnt;

	// Step through each object in the current space
	for (pItr->start(); !pItr->done(); pItr->step())
	{
		// Get the entity and open it for read
		pItr->getEntity(pEnt, AcDb::kForRead);

		acutPrintf(_T("\nNom de la class: %s"), pEnt->isA()->name());
		pEnt->close();

	}

	pBlockTableRecord->close();

	delete pItr;

	acedTextScr();  //affichage de la console Autocad

}

static void createPolyline()

{
	AcGePoint3dArray ptArr;
	ptArr.setLogicalLength(4);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(10, 0, 0.0);
	ptArr[2].set(10, 10, 0.0);
	ptArr[3].set(0, 10, 0.0);

	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
		AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);

	// Get the current database
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();


	// Open the Block Table for read-only
	AcDbBlockTable* pBlockTable;
	pDb->getSymbolTable(pBlockTable, AcDb::kForRead);

	// Get the Model Space block
	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();


	// Add the new Line object to Model space
	pBlockTableRecord->appendAcDbEntity(pNewPline);

	pBlockTableRecord->close();

	pNewPline->close();

}

static void createSolid3d()
{
	Acad::ErrorStatus es;

	ads_name polyName;

	ads_point ptres;

	AcGePoint3dArray ptArr;
	ptArr.setLogicalLength(4);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(10, 0, 0.0);
	ptArr[2].set(10, 10, 0.0);
	ptArr[3].set(0, 10, 0.0);

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	//DeplacementObjet3D(pSolid);
	//CreationSection(pSolid);
	CreationVoid(pSolid);

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


static void DeplacementObjet3D(AcDb3dSolid* pSolid) {

	// 3 source points
	AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
	AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
	AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);



	//double x1 = System.Math.Round(double.Parse(PointDeplacement[0]), 3); 
	double x1 = round(0);  //PointDeplacement x

	//double y1 = System.Math.Round(double.Parse(PointDeplacement[1]), 3);
	double y1 = round(10); //PointDeplacement y

	//double z1 = System.Math.Round(double.Parse(PointDeplacement[2]), 3);
	double z1 = round(0); //PointDeplacement z

	//double x2 = System.Math.Round(double.Parse(Direction1[0]), 3);
	double x2 = round(0); //Direction1 x

	//double y2 = System.Math.Round(double.Parse(Direction1[1]), 3);
	double y2 = round(0); //Direction1 y

	//double z2 = System.Math.Round(double.Parse(Direction1[2]), 3);
	double z2 = round(1); //Direction1 z

	//double x3 = System.Math.Round(double.Parse(Direction2[0]), 3);
	double x3 = round(1); //Direction2 x

	//double y3 = System.Math.Round(double.Parse(Direction2[1]), 3);
	double y3 = round(0); //Direction2 y

	//double z3 = System.Math.Round(double.Parse(Direction2[2]), 3);
	double z3 = round(0); //Direction2 z

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

static void CreationSection(AcDb3dSolid* extrusion)
{


	//PromptStringOptions pStrOpts = new PromptStringOptions("\nDand Fonction section ");
	//pStrOpts.AllowSpaces = true;
	//PromptResult pStrRes = acDoc.Editor.GetString(pStrOpts);

	int cote = 0;

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
	ptArr.setLogicalLength(4);

	ptArr[0].set(0, 0, 0.0);
	ptArr[1].set(10, 0, 0.0);
	ptArr[2].set(10, 10, 0.0);
	ptArr[3].set(0, 10, 0.0);

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	//DeplacementObjet3D(pSolid);


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

	////Coordonnées du repère du plan
	//p0x = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[0]); 
	p0x = 1;  //x																		
	//p0y = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[1]);
	p0y = 1; //y
	//p0z = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[2]);
	p0z = 1; //z

	p0 = AcGePoint3d::AcGePoint3d(p0x, p0y, p0z);

	///Direction1 plan
	//p1x = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[0]);
	p1x = 1;  //x
	//p1y = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[1]);
	p1y = 1; //y
	//p1z = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[2]);
	p1z = 1; //z

	V1 = AcGeVector3d::AcGeVector3d(p1x, p1y, p1z);
	V2 = V1.normal();
	p3 = AcGePoint3d::AcGePoint3d(V2.x + p0x, V2.y + p0y, V2.z + p0z);



	///Direction2 plan
	//p2x = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[0]);
	p2x = 1;  //x
	//p2y = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[1]);
	p2y = 1; //y
	//p2z = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[2]);
	p2z = 1; //z
	//Vector3d p2 = new Vector3d(p0x + p2x, p0y + p2y, p0z + p2z);

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
	//Plane plane = new Plane(p0, p5, p2);
	//DeplacementObjet3DPlan(plane, ifcCoordonneesRepere, Direction1, Direction2);

	//if (PLanDeSection.ElementAt(i).boolOperation == ".T.")
	//{
	Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal().negate());
	//}
	//else if (PLanDeSection.ElementAt(i).boolOperation == ".F.")
	//{
	Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal());
	//}

	extrusion->getSlice(Poly_plane, false, pSolid);

	extrusion->booleanOper(AcDb::kBoolSubtract, pSolid);
	//extrusion.setColor(2);



	//break;
	//case "IFCCOMPOSITECURVE":

	//break;
//}
//break;

/*case "IFCHALFSPACESOLID":




	//Autodesk.AutoCAD.ApplicationServices.Application.DocumentManager.MdiActiveDocument.Editor.WriteMessage("\nPoint Car : " + PLanDeSection.ElementAt(i).list_CartesianPoint_Plane[0] + "," + PLanDeSection.ElementAt(i).list_CartesianPoint_Plane[1] + "," + PLanDeSection.ElementAt(i).list_CartesianPoint_Plane[2]);
	//Autodesk.AutoCAD.ApplicationServices.Application.DocumentManager.MdiActiveDocument.Editor.WriteMessage("\nDirection 1 : " + PLanDeSection.ElementAt(i).list_Direction1_Plane[0] + "," + PLanDeSection.ElementAt(i).list_Direction1_Plane[1] + "," + PLanDeSection.ElementAt(i).list_Direction1_Plane[2]);
	//Autodesk.AutoCAD.ApplicationServices.Application.DocumentManager.MdiActiveDocument.Editor.WriteMessage("\nDirection 2 : " + PLanDeSection.ElementAt(i).list_Direction2_Plane[0] + "," + PLanDeSection.ElementAt(i).list_Direction2_Plane[1] + "," + PLanDeSection.ElementAt(i).list_Direction2_Plane[2]);

	////PromptStringOptions pStrOpts = new PromptStringOptions("\nExecution d'une section : ");
	////pStrOpts.AllowSpaces = true;
	////pStrRes = acDoc.Editor.GetString(pStrOpts);






	////Coordonnées du repère du plan
		//p0x = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[0]);
	p0x = 1;  //x
	//p0y = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[1]);
	p0y = 1; //y
	//p0z = float.Parse(PLanDeSection.ElementAt(i).ifcCoordonneesReperePlane[2]);
	p0z = 1; //z

	p0 = AcGePoint3d::AcGePoint3d(p0x, p0y, p0z);

	///Direction1 plan
	//p1x = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[0]);
	p1x = 1;  //x
	//p1y = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[1]);
	p1y = 1; //y
	//p1z = float.Parse(PLanDeSection.ElementAt(i).list_Direction1_Plane[2]);
	p1z = 1; //z

	V1 = AcGeVector3d::AcGeVector3d(p1x, p1y, p1z);
	V2 = V1.normal();
	p3 = AcGePoint3d::AcGePoint3d(V2.x + p0x, V2.y + p0y, V2.z + p0z);



	///Direction2 plan
	//p2x = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[0]);
	p2x = 1;  //x
	//p2y = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[1]);
	p2y = 1; //y
	//p2z = float.Parse(PLanDeSection.ElementAt(i).list_Direction2_Plane[2]);
	p2z = 1; //z
	//Vector3d p2 = new Vector3d(p0x + p2x, p0y + p2y, p0z + p2z);

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


	AcDbLine line = AcDbLine::AcDbLine(p0, p1);
	v3d = line.normal();

	p33 = AcGePoint3d::AcGePoint3d(v3d.x, v3d.y, v3d.z);

	p5 = AcGePoint3d::AcGePoint3d(p1xx + p0x, p1yy + p0y, p1zz + p0z);




	//var s = (Solid3d)tr.GetObject(id, OpenMode.ForWrite);
	AcGePlane Poly_plane = AcGePlane::AcGePlane(p0, p5, p2);
	//Plane plane = new Plane(p0, p5, p2);
	//DeplacementObjet3DPlan(plane, ifcCoordonneesRepere, Direction1, Direction2);

	if (PLanDeSection.ElementAt(i).boolOperation == ".T.")
	{
		Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal().negate());
	}
	else if (PLanDeSection.ElementAt(i).boolOperation == ".F.")
	{
		Poly_plane.set(Poly_plane.pointOnPlane(), Poly_plane.normal());
	}
	extrusion.Slice(Poly_plane);


	//extrusion.setColor(2);



	break;
}*/


//}


//return extrusion;
}


static void CreationVoid(AcDb3dSolid* extrusion)
{
	Acad::ErrorStatus es;

	ads_name polyName;

	ads_point ptres;

	AcGePoint3dArray ptArr;

	/*if (ListNbArg.size() > 1)
	{
		ListNbArg.erase(ListNbArg.begin());
		points1.pop_front();
	}*/

	//ptArr.setLogicalLength(ListNbArg[0]);
	ptArr.setLogicalLength(4);
	//Vec3 pointOrigine = { tranform1[12], tranform1[13] , tranform1[14] };

	int i = 0;

	/*for (const auto& point : points1)
	{
		if (i == ListNbArg[0])
		{
			break;
		}

		ptArr[i].set(point.x(), point.y(), point.z());

		i++;
	}*/


	ptArr[0].set(0, 0, 0);
	ptArr[1].set(5, 0, 0);
	ptArr[2].set(5, 5, 0);
	ptArr[3].set(0, 5, 0);

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
	//es = pSolid->extrude(pRegion, VecteurExtrusion.z(), 0.0);
	es = extrusion_void->extrude(pRegion, 10, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	/*int nbPlan = listPlan.size();

	for (int a = 0; a < nbPlan; a++)
	{
		for (size_t i = 0; i < ListNbArg[0]; i++)
		{
			points1.pop_front();
		}

		ListNbArg.erase(ListNbArg.begin());

		if (points1.size() > 0 && ListNbArg.size() > 0)
		{
			CreationSection(pSolid, VecteurExtrusion, points1, ListNbArg, listPlan, listLocationPolygonal, Agreement);

			listPlan.pop_front();
		}
	}*/

	//DeplacementObjet3D(pSolid, tranform1);


	//pSolid->close();

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
	extrusion_void->close();


	extrusion->booleanOper(AcDb::kBoolSubtract, extrusion_void);
}


static void createSolid3dProfileIPE()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Width = 170;
	float Depth = 360;
	float WebThickness = 8;
	float FlangeThickness = 12.5;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileIPN()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Width = 143;
	float Depth = 360;
	float WebThickness = 13;
	float FlangeThickness = 19.5;
	float FlangeSlope = 0.13962634;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileUPE()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 360;
	float Width = 110;
	float WebThickness = 12;
	float FlangeThickness = 17;
	float FilletRadius = 15;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileUPN()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 360;
	float Width = 110;
	float WebThickness = 12;
	float FlangeThickness = 17;
	float FilletRadius = 15;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileL8()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 60;
	float Width = 60;
	float Thickness = 6;
	float FilletRadius = 8;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileL9()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 60;
	float Width = 40;
	float Thickness = 6;
	float FilletRadius = 8;
	float LegSlope = 2;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileT10()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 140;
	float Width = 140;
	float WebThickness = 15;
	float FlangeThickness = 15;
	float FilletRadius = 15;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileT12()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 140;
	float Width = 140;
	float WebThickness = 15;
	float FlangeThickness = 15;
	float FilletRadius = 15;
	float WebSlope​ = 2;
	float FlangeSlope = 2;

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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileC()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 24;
	float Width = 5.2;
	float WallThickness = 1;
	float Girth = 3.25;
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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileZ()
{

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float Depth = 40;
	float Width = 30;
	float WebThickness = 5;
	float FlangeThickness = 5;
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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createSolid3dProfileAsyI()
{


	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;

	float BottomFlangeWidth = 200;
	float OverallDepth = 400;
	float WebThickness = 10;
	float BottomFlangeThickness = 10;
	float TopFlangeWidth = 100;
	float TopFlangeThickness = 5;
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
	es = pSolid->extrude(pRegion, 10.0, 0.0);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
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
	}
}

static void createFaceSolid() {

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr;


	ptArr.setLogicalLength(8);

	ptArr[0].set(2.5, 0, 0.0);
	ptArr[1].set(5, 0, 0.0);
	ptArr[2].set(7.5, 0, 2.5);
	ptArr[3].set(7.5, 0, 5);
	ptArr[4].set(5, 0, 7.5);
	ptArr[5].set(2.5, 0, 7.5);
	ptArr[6].set(0, 0, 5);
	ptArr[7].set(0, 0, 2.5);


	AcDb3dPolyline* pNewPline = new AcDb3dPolyline(
		AcDb::k3dSimplePoly, ptArr, Adesk::kTrue);
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
	AcDbPlaneSurface* pSurface = new AcDbPlaneSurface();
	es = pSurface->createFromRegion(pRegion);

	for (int i = 0; i < lines.length(); i++)
	{
		delete (AcRxObject*)lines[i];
	}

	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}
	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;

	if (Acad::eOk == es)
	{
		AcDbDatabase* pDb = curDoc()->database();
		AcDbObjectId modelId;
		modelId = acdbSymUtil()->blockModelSpaceId(pDb);
		AcDbBlockTableRecord* pBlockTableRecord;
		acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
		pBlockTableRecord->appendAcDbEntity(pSurface);
		pBlockTableRecord->close();
		pSurface->close();
	}
	else
	{
		delete pSurface;
	}

}

static void createPolyFaceSolid() {
	// polyline creation 

	AcGePoint3dArray ptArr;
	ptArr.setLogicalLength(4);
	ptArr[0].set(0, 0, 0);
	ptArr[1].set(10, 0, 0);
	ptArr[2].set(10, 10, 0);
	ptArr[3].set(0, 10, 0);



	AcDb2dPolyline* pNewPline = new AcDb2dPolyline(AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
	pNewPline->setColorIndex(3);


	//polygon mesh constructor without any parameter


	AcDbPolygonMesh* pMesh = new AcDbPolygonMesh();
	pMesh->setMSize(1);
	pMesh->setNSize(4);
	pMesh->makeMClosed();
	pMesh->makeNClosed();


	AcDbVoidPtrArray arr;
	arr.append(pMesh);


	AcDbBlockTable* pBlockTable;
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);


	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();


	AcDbObjectId plineObjId;
	pBlockTableRecord->appendAcDbEntity(plineObjId, pNewPline);



	AcDbObjectIterator* pVertIter = pNewPline->vertexIterator();
	AcDb2dVertex* pVertex;
	AcGePoint3d location;
	AcDbObjectId vertexObjId;
	for (int vertexNumber = 0; !pVertIter->done(); vertexNumber++, pVertIter->step())
	{
		vertexObjId = pVertIter->objectId();
		acdbOpenObject(pVertex, vertexObjId, AcDb::kForRead);
		location = pVertex->position();
		pVertex->close();
		AcDbPolygonMeshVertex* polyVertex = new AcDbPolygonMeshVertex(pVertex->position());
		pMesh->appendVertex(polyVertex);
		polyVertex->close();
	}
	delete pVertIter;


	pBlockTableRecord->appendAcDbEntity(pMesh);
	pBlockTableRecord->close();
	pNewPline->close();
	pMesh->close();
}

static void createSolid3dProfileRectHollow() {

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;

	AcGePoint3dArray ptArr1;
	AcGePoint3dArray ptArr2;

	float XDim = 100;
	float YDim = 60;
	float WallThickness = 2.5;

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
	es = pSolid->extrude(pRegion1, 10.0, 0.0);

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

static void createSolid3dProfileCircHollow() {

	Acad::ErrorStatus es;
	ads_name polyName;
	ads_point ptres;


	AcGePoint3dArray ptArr2;

	float Radius = 34;
	float WallThickness = 4;

	AcGePoint3d center = AcGePoint3d::AcGePoint3d(Radius, Radius, 0);
	/// <summary>
	/// Première polyline
	/// </summary>


	AcDbCircle* circle1 = new AcDbCircle();
	circle1->setCenter(center);
	circle1->setRadius(Radius);
	circle1->setColorIndex(3);

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
	es = pSolid->extrude(pRegion1, 10.0, 0.0);

	for (int i = 0; i < lines1.length(); i++)
	{
		delete (AcRxObject*)lines1[i];
	}

	for (int ii = 0; ii < regions1.length(); ii++)
	{
		delete (AcRxObject*)regions1[ii];
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
	}

}

static void createBoundingBox() {

	Acad::ErrorStatus es;

	// Extrude the region to create a solid.

	AcDb3dSolid* box3d = new AcDb3dSolid();

	box3d->createBox(20, 20, 20);  /// Creation de la box au point 0,0,0 avec les dimensions


	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;





	AcDbDatabase* pDb = curDoc()->database();

	AcDbObjectId modelId;

	modelId = acdbSymUtil()->blockModelSpaceId(pDb);



	AcDbBlockTableRecord* pBlockTableRecord;

	acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord,

		modelId, AcDb::kForWrite);



	pBlockTableRecord->appendAcDbEntity(box3d);

	pBlockTableRecord->close();

	box3d->close();


}

static void createPlane() {

	Acad::ErrorStatus es;

	// Extrude the region to create a solid.

	AcGePlane plane = AcGePlane::AcGePlane();

	AcDbSurface* surface = new AcDbSurface();

	//surface->getPlane(plane, AcDb::Planarity::kLinear);

	AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;


	AcDbDatabase* pDb = curDoc()->database();

	AcDbObjectId modelId;

	modelId = acdbSymUtil()->blockModelSpaceId(pDb);



	AcDbBlockTableRecord* pBlockTableRecord;

	acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord,

		modelId, AcDb::kForWrite);



	pBlockTableRecord->appendAcDbEntity(surface);

	pBlockTableRecord->close();

	surface->close();


}


void initApp()
{
	// register a command with the AutoCAD command mechanism
	acedRegCmds->addCommand(_T("HELLOWORLD_COMMANDS"),
		_T("Hello"),
		_T("Bonjour"),
		ACRX_CMD_TRANSPARENT,
		helloWorld);
	acedRegCmds->addCommand(_T("AUCommands"), _T("AddLine"), _T("AddLine"),

		ACRX_CMD_MODAL, addLine);

	acedRegCmds->addCommand(_T("AUCommands"), _T("MakeLayer"), _T("MakeLayer"),

		ACRX_CMD_MODAL, makeLayer);

	acedRegCmds->addCommand(_T("AUCommands"), _T("ListObjects"), _T("ListObjects"), ACRX_CMD_MODAL, listObjects);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createPolyline"), _T("createPolyline"), ACRX_CMD_MODAL, createPolyline);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createSolid3d"), _T("createSolid3d"), ACRX_CMD_MODAL, createSolid3d);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createSolid3dProfileIPE"), _T("createSolid3dProfileIPE"), ACRX_CMD_MODAL, createSolid3dProfileIPE);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createSolid3dProfileIPN"), _T("createSolid3dProfileIPN"), ACRX_CMD_MODAL, createSolid3dProfileIPN);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createSolid3dProfileUPE"), _T("createSolid3dProfileUPE"), ACRX_CMD_MODAL, createSolid3dProfileUPE);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createFaceSolid"), _T("createFaceSolid"), ACRX_CMD_MODAL, createFaceSolid);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createPolyFaceSolid"), _T("createPolyFaceSolid"), ACRX_CMD_MODAL, createPolyFaceSolid);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createSolid3dProfileRectHollow"), _T("createSolid3dProfileRectHollow"), ACRX_CMD_MODAL, createSolid3dProfileRectHollow);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createSolid3dProfileCircHollow"), _T("createSolid3dProfileCircHollow"), ACRX_CMD_MODAL, createSolid3dProfileCircHollow);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createBoundingBox"), _T("createBoundingBox"), ACRX_CMD_MODAL, createBoundingBox);

	acedRegCmds->addCommand(_T("AUCommands"), _T("createSolid3dProfileAsyI"), _T("createSolid3dProfileAsyI"), ACRX_CMD_MODAL, createSolid3dProfileAsyI);
}

void unloadApp()
{
	acedRegCmds->removeGroup(_T("HELLOWORLD_COMMANDS"));

}

void helloWorld()
{
	acutPrintf(_T("\nHello World!"));
}



extern "C" AcRx::AppRetCode
acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
	switch (msg)
	{

	case AcRx::kInitAppMsg:
		acrxDynamicLinker->unlockApplication(pkt);
		acrxRegisterAppMDIAware(pkt);
		initApp();
		break;
	case AcRx::kUnloadAppMsg:
		unloadApp();
		break;
	default:
		break;

	}

	return AcRx::kRetOK;

}
