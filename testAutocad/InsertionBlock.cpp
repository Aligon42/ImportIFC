#include "tchar.h"
#include <aced.h>
#include <rxregsvc.h> 
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <ctime>
#include <thread>

#include <Step/CallBack.h>

#include "CreateConstructionPointVisitor.h"
#include "ComputePlacementVisitor.h"
#include "MethodeConstruction.h"
#include "tests.h"
#include "MethodeConstruction.h"
#include <adscodes.h>

#include <iostream>

const wchar_t* PathBlock(const ACHAR* cheminBlock, const ACHAR* nomBlock)
{
	std::string cheminBlockS;
	std::wstring ws1(cheminBlock);
	std::string str1(ws1.begin(), ws1.end());

	std::wstring ws2(nomBlock);
	std::string str2(ws2.begin(), ws2.end());

	cheminBlockS = str1 + "\\" + str2 + ".dwg";

	const char* cstr = cheminBlockS.c_str();
	const ACHAR* cheminComplet = GetWCM(cstr);
	return cheminComplet;
}

const wchar_t* nomBlockH(const ACHAR* nomBlock, int m, int longueur, int largeur)
{
	std::string cheminBlockS;
	std::wstring ws1(nomBlock);
	std::string str1(ws1.begin(), ws1.end());

	std::string str2 = std::to_string(m);
	std::string str3 = std::to_string(longueur);
	std::string str4 = std::to_string(largeur);
	int pos = str1.find('[');
	if (pos > 0)
	{
		cheminBlockS = str1.substr(0, pos) + "[" + str3 + "x" + str4 + "]-" + str2;
	}
	else {
		cheminBlockS = str1 + "[" + str3 + "x" + str4 + "]-" + str2;
	}

	const char* cstr = cheminBlockS.c_str();
	const ACHAR* cheminComplet = GetWCM(cstr);
	return cheminComplet;
}

int block(void)
{


	struct resbuf* rb;
	struct resbuf* rbList = acutNewRb(RTLB);
	struct resbuf* pSVal;
	pSVal = rbList;
	int a = 0;
	int angle = 0;
	int64_t nameInt;
	ads_real x = 0.0;
	ads_real y = 0.0;
	ads_real z = 0.0;
	long longs = 0;

	Acad::ErrorStatus es;
	const ACHAR* nomBlock;
	const ACHAR* cheminBlock;
	const ACHAR* cheminComplet;
	const ACHAR* calque;
	float rotation = 0;
	float longueur = 0;

	rb = acedGetArgs();
	if (rb == NULL)
	{
		acutPrintf(_T("\n123 soleil\n"));
		return RTERROR;
	}

	//acutPrintf(L"\n0000");
	nomBlock = rb->resval.rstring;
	rb = rb->rbnext;
	cheminBlock = rb->resval.rstring;
	rb = rb->rbnext;
	longueur = rb->resval.rreal;
	rb = rb->rbnext;
	calque = rb->resval.rstring;
	rb = rb->rbnext;
	rotation = rb->resval.rreal;
	rb = rb->rbnext;

	AcGePoint3d origin = AcGePoint3d::AcGePoint3d(0., 0.0, 0.);
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();

	AcDbObjectId blkTabId = acdbHostApplicationServices()->workingDatabase()->blockTableId();
	AcDbBlockTable* pBlkTab = NULL;

	acdbOpenAcDbObject((AcDbObject*&)pBlkTab, blkTabId, AcDb::kForRead);

	AcDbObjectId blockId = AcDbObjectId::AcDbObjectId();
	///AcDbStub* stub = nullptr;
	//AcDbObjectId blokId2 = AcDbObjectId::AcDbObjectId(stub);

	AcDbDatabase* pBlockDb = new AcDbDatabase();

	AcGeMatrix3d matrix3d;
	AcGeVector3d vectorNorm = AcGeVector3d::AcGeVector3d(0.0, 0.0, 500);

	AcDbDatabase* pDb1 = curDoc()->database();
	AcDbObjectId modelId;
	AcDbObjectId idBlock;
	modelId = acdbSymUtil()->blockModelSpaceId(pDb1);

	acedGetCurrentUCS(matrix3d);

	AcDbBlockTableRecord* pMS = NULL;
	acdbOpenAcDbObject((AcDbObject*&)pMS, modelId, AcDb::kForWrite);

	//acutPrintf(L"\n11111");
	while (rb != NULL)
	{
		short type = rb->restype;

		if (type == RT3DPOINT)
		{
			x = rb->resval.rpoint[0];
			y = rb->resval.rpoint[1];
			z = rb->resval.rpoint[2];

			pBlkTab->getAt(nomBlock, blockId);

			/*
			if (pBlkTab->has(nomBlock))
			{

				acutPrintf(_T("\n*"));
			}
			else
			{
				cheminComplet = PathBlock(cheminBlock, nomBlock);

				// Load the Drawing into a Separate Database

				if (pBlockDb->readDwgFile(cheminComplet) != Acad::eOk)
				{
					delete pBlockDb;
					return(FALSE);
				}
				//Insert this new Drawing as a Block in the Current Database
				pBlockDb->insert(blockId, nomBlock, pBlockDb);
			}

			*/

			origin[0] = x;
			origin[1] = y;
			origin[2] = z;

			AcDbBlockReference* acBlkRef = new AcDbBlockReference(origin, blockId);
			acBlkRef->transformBy(matrix3d);
			acBlkRef->setRotation(rotation);
			acBlkRef->setLayer(calque);

			// Add the block reference to the dwg/database first!!!!
			pMS->appendAcDbEntity(acBlkRef);

			idBlock = acBlkRef->id();

			acBlkRef->close();



			ads_name name;


			acdbGetAdsName(name, idBlock);


			pSVal = pSVal->rbnext = acutNewRb(RTENAME);
			pSVal->resval.rlname[0] = name[0];
			pSVal->resval.rlname[1] = name[1];

		}




		/*if (a < 0) { // Check the argument range.
			acdbFail(_T("Argument should be positive."));
			return RTERROR;
		}
		else if (a > 170)
		{ // Avoid floating-point overflow.
			acdbFail(_T("Argument should be 170 or less."));
			return RTERROR;
		}*/

		rb = rb->rbnext;
	}

	pBlkTab->close();
	pSVal = pSVal->rbnext = acutNewRb(RTLE);
	pMS->close();

	acedRetList(rbList); // Call the function itself, and return the value to AutoLISP.
	acutRelRb(rbList);

	return (RSRSLT);
}

AcDbObjectIdArray postToDatabase(AcDbVoidPtrArray eSet, AcDb3dSolid* pSolid, int typeBool, double rotation, AcGePoint3d origin)

{

	Acad::ErrorStatus es;
	AcDbBlockTable* pBtbl;
	AcDbBlockTableRecord* pBtblr;

	AcDbObjectIdArray resultarr;
	ads_name jeuSel;
	//acutPrintf(L"\n0000");
	es = acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBtbl, AcDb::kForRead);
	//acutPrintf(L"\n1111");
	if (es != Acad::eOk)
	{
		acutPrintf(L"\nFailed to open block table");
		return es;
	}

	es = pBtbl->getAt(ACDB_MODEL_SPACE, pBtblr, AcDb::kForWrite);
	//acutPrintf(L"\n2222");
	if (es != Acad::eOk)
	{
		acutPrintf(L"\nFailed to open block table record");
		es = pBtbl->close();
		if (es != Acad::eOk)
		{
			acutPrintf(L"\nFailed to close block table");
		}
		return es;
	}

	es = pBtbl->close();

	if (es != Acad::eOk)
	{
		acutPrintf(L"\nFailed to close block table");
		return es;
	}
	//acutPrintf(L"\n4444");
	AcGePoint3d end = AcGePoint3d::AcGePoint3d(origin.x, origin.y, origin.z + 100);
	ads_point pOrigin;
	ads_point pResult;
	struct resbuf* rbOrigin = acutNewRb(RTSHORT);
	rbOrigin->resval.rint = 0;

	struct resbuf* rbFinal = acutNewRb(RTSHORT);;
	rbFinal->resval.rint = 1;

	pOrigin[0] = origin[0];
	pOrigin[1] = origin[1];
	pOrigin[2] = origin[2];

	acedTrans(pOrigin, rbFinal, rbOrigin, 0, pResult);

	origin[0] = pResult[0];
	origin[1] = pResult[1];
	origin[2] = pResult[2];


	pOrigin[0] = end[0];
	pOrigin[1] = end[1];
	pOrigin[2] = end[2];

	acedTrans(pOrigin, rbFinal, rbOrigin, 0, pResult);

	end[0] = pResult[0];
	end[1] = pResult[1];
	end[2] = pResult[2];


	AcGeVector3d vector3D = origin - end;

	for (int i = 0; i < eSet.length(); i++)
	{

		AcGeMatrix3d matrix3dR;
		AcDbObjectId ObjId;
		AcCmColor colorBlock;
		double volume;
		AcGePoint3d centroid;

		AcArray<AcDb3dSolid*> newSolids;
		double momInertia[3];
		double prodInertia[3];
		double prinMoments[3];
		AcGeVector3d prinAxes[3];
		double radiiGyration[3];
		AcDbExtents extents;
		ads_name name;

		AcDb3dSolid* pNewEnt = AcDb3dSolid::cast((AcRxObject*)eSet[i]);
		AcDb3dSolid* pClone = (AcDb3dSolid*)pSolid->clone();
		colorBlock = pNewEnt->color();
		pNewEnt->setRecordHistory(false);

		pClone->setRecordHistory(false);
		//pClone->close();


		if (typeBool == 0)
		{

			es = pNewEnt->booleanOper(AcDb::kBoolSubtract, pClone);

		}
		else
		{

			es = pNewEnt->booleanOper(AcDb::kBoolIntersect, pClone);
		}



		pNewEnt->setLayer(_T("0"));
		pNewEnt->getMassProp(volume, centroid, momInertia, prodInertia, prinMoments, prinAxes, radiiGyration, extents);
		pNewEnt->setColor(colorBlock);



		matrix3dR.setToRotation(rotation, vector3D, origin);
		es = pNewEnt->transformBy(matrix3dR);
		es = pBtblr->appendAcDbEntity(ObjId, pNewEnt);


		newSolids.setPhysicalLength(100);
		es = pNewEnt->separateBody(newSolids);

		int kl = newSolids.length();

		//for (auto solid3d : newSolids)
		//{

		if (volume > 0)
		{
			resultarr.append(ObjId);
		}

		for (int k = 0; k < newSolids.length(); k++)
		{
			AcDb3dSolid* pSol = newSolids.at(k);
			pSol->getMassProp(volume, centroid, momInertia, prodInertia, prinMoments, prinAxes, radiiGyration, extents);
			pSol->setColor(colorBlock);
			pSol->setLayer(_T("0"));
			pBtblr->appendAcDbEntity(ObjId, pSol);
			pSol->close();
			if (volume > 0)
			{
				resultarr.append(ObjId);
			}

			// open AcDbBlockTableRecord before the for loop -
		}

		newSolids.removeAll();



		if (es != Acad::eOk)
		{
			acutPrintf(L"\nFailed to append entity");
		}

		es = pNewEnt->close();
		if (es != Acad::eOk)
		{
			acutPrintf(L"\nFailed to close entity");
		}
	}

	es = pBtblr->close();
	if (es != Acad::eOk)
	{
		acutPrintf(L"\nFailed to close block table record");
	}

	return resultarr;

}

int ajusterBlocAvecContour(void)
{
	ads_name contour;
	ads_name js;
	ads_name jeuSel;
	ads_name jeuSelG;
	ads_name jeuSelD;
	ads_name objet;
	AcDbObjectId idObj;
	AcDbObjectId idContour;
	AcGePoint3d origin;
	AcDbBlockReference* pEnt;
	AcDbEntity* pPoly;
	struct resbuf* rb;
	struct resbuf* rbList = acutNewRb(RTLB);
	struct resbuf* pSVal;
	pSVal = rbList;
	long longs = 0;
	int typeBool;
	Acad::ErrorStatus es;
	const ACHAR* nomBlock;
	const ACHAR* layer;
	int nbrObj = 0;
	int nbrObjG = 0;
	int nbrObjD = 0;
	int newOri;
	double rotation;

	rb = acedGetArgs();
	if (rb == NULL)
	{
		acutPrintf(_T("\n123 soleil\n"));
		return RTERROR;
	}


	struct resbuf* rbOrigin = acutNewRb(RTSHORT);
	rbOrigin->resval.rint = 0;

	struct resbuf* rbFinal = acutNewRb(RTSHORT);;
	rbFinal->resval.rint = 1;

	typeBool = rb->resval.rint;
	rb = rb->rbnext;
	contour[0] = rb->resval.rlname[0];
	contour[1] = rb->resval.rlname[1];
	rb = rb->rbnext;
	js[0] = rb->resval.rlname[0];
	js[1] = rb->resval.rlname[1];



	es = acdbGetObjectId(idContour, contour);
	es = acdbOpenAcDbEntity(pPoly, idContour, AcDb::kForWrite);

	AcDbVoidPtrArray eSetPoly;
	eSetPoly.setPhysicalLength(10);
	es = pPoly->explode(eSetPoly);
	pPoly->erase();

	pPoly->close();
	AcDbVoidPtrArray regions;

	regions.setPhysicalLength(100);
	es = AcDbRegion::createFromCurves(eSetPoly, regions);
	if (Acad::eOk != es)
	{
		pPoly->close();
		acutPrintf(L"\nFailed to create region\n");
		//return;
	}

	AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);
	// Extrude the region to create a solid.
	AcDb3dSolid* pSolid = new AcDb3dSolid();
	es = pSolid->extrude(pRegion, 500, 0.0);


	auto ext = AcDbExtents();
	ads_point p1Contour;
	ads_point p1resContour;
	ads_point p2Contour;
	ads_point p2resContour;
	AcGePoint3d minPointContour;
	AcGePoint3d maxPointContour;
	//name2->getGeomExtents(extents);
	es = pSolid->getGeomExtents(ext);
	minPointContour = ext.minPoint();
	maxPointContour = ext.maxPoint();
	p1Contour[0] = minPointContour[0];
	p1Contour[1] = minPointContour[1];
	p1Contour[2] = minPointContour[2];
	p2Contour[0] = maxPointContour[0];
	p2Contour[1] = maxPointContour[1];
	p2Contour[2] = maxPointContour[2];

	AcGePoint2d pointRotation;
	AcGePoint2d pointMilieuContour;
	AcGePoint2d pointOrigne2D;
	AcGeMatrix2d mat1;
	AcGeMatrix2d mat2;



	acedTrans(p1Contour, rbOrigin, rbFinal, 0, p1resContour);
	acedTrans(p2Contour, rbOrigin, rbFinal, 0, p2resContour);


	//float xmil = (p2Contour[0] + p1Contour[0]) / 2.;
	//float ymil = (p2Contour[1] + p1Contour[1]) / 2.;

	float xmil = (p2resContour[0] + p1resContour[0]) / 2.;
	float ymil = (p2resContour[1] + p1resContour[1]) / 2.;


	//BacutPrintf(_T("\n(%f %f)   (%f %f) / (%f %f) "), p1resContour[0], p1resContour[1], p2resContour[0], p2resContour[1], xmil, ymil);


	pointMilieuContour.x = xmil;
	pointMilieuContour.y = ymil;

	double x1;
	double y1;

	double x2;
	double y2;



	float xmils;
	float ymils;

	ads_point PmilContour;
	PmilContour[0] = xmil;
	PmilContour[1] = ymil;
	PmilContour[2] = 0;

	for (int i = 0; i < eSetPoly.length(); i++)
	{
		delete (AcRxObject*)eSetPoly[i];
	}
	for (int ii = 0; ii < regions.length(); ii++)
	{
		delete (AcRxObject*)regions[ii];
	}

	pSolid->close();

	regions.removeAll();
	acdbEntDel(contour);


	ads_sslength(js, &nbrObj);
	//acutPrintf(_T("Nombre : %i"), nbrObj);

	AcDbVoidPtrArray eSet;

	for (int i = 0; i < nbrObj; i++)
	{
		ads_ssname(js, i, objet);
		es = acdbGetObjectId(idObj, objet);
		es = acdbOpenAcDbEntity((AcDbEntity*&)pEnt, idObj, AcDb::kForWrite);


		AcDbObjectIdArray resultarr;
		AcDbObjectId ObjId;
		ads_name name;
		AcGeMatrix3d matrix3dG;


		acedGetCurrentUCS(matrix3dG);
		origin = pEnt->position();
		//angle = pEnt->get
		double angleRotation = pEnt->rotation();
		ads_point pOrigin;
		ads_point pResult;
		ads_point p1;
		ads_point p1res;
		ads_point p2;
		ads_point p2res;




		//minPointContour.transformBy(matrotation)

		pOrigin[0] = origin[0];
		pOrigin[1] = origin[1];
		pOrigin[2] = origin[2];

		newOri = acedTrans(pOrigin, rbOrigin, rbFinal, 0, pResult);

		origin[0] = pResult[0];
		origin[1] = pResult[1];
		origin[2] = pResult[2];

		layer = pEnt->layer();
		rotation = pEnt->rotation();

		pointRotation.x = origin[0];
		pointRotation.y = origin[1];

		mat1.setToRotation(-rotation, pointRotation);
		pointMilieuContour.transformBy(mat1);

		mat2.setToRotation(rotation, pointRotation);
		//pointMilieuContour.transformBy(mat12);

		xmil = pointMilieuContour.x;
		ymil = pointMilieuContour.y;







		// Get the objectID of the block definition.
		AcDbObjectId blockDefId = pEnt->blockTableRecord();
		// Close the selected INSERT.
		pEnt->close();

		// Open the block definition.
		AcDbBlockTableRecord* pBlkRecord;
		if (Acad::eOk != (es = acdbOpenObject(pBlkRecord, blockDefId, AcDb::kForRead)))
		{
			acutPrintf(L"\nCannot access block definition.\n");

		}
		// Get the name of the block definition.
		es = pBlkRecord->getName(nomBlock);
		pBlkRecord->close();
		if ((Acad::eOk != es) || !nomBlock)
		{
			acutPrintf(L"\nCannot extract block definition name.\n");

		}

		AcDbEntity* name2;
		AcDbExtents  extents;
		AcGePoint3d minPoint;
		AcGePoint3d maxPoint;
		float longueurG = 0;
		float longueurD = 0;
		float largeurG = 0;
		float largeurD = 0;


		float xminG = 999999999999.99;
		float yminG = 999999999999.99;
		float xminD = 999999999999.99;

		eSet.setPhysicalLength(50);
		es = pEnt->explode(eSet);
		int nbSolid3D = 0;
		nbSolid3D = eSet.length();
		if (es == Acad::eOk)
		{
			pEnt->close();

			//delete the original entity
			acdbEntDel(objet);

			// Add the new entities to the db
			resultarr = postToDatabase(eSet, pSolid, typeBool, rotation, origin);
			/*if (es != Acad::eOk)
			{
				acutPrintf(L"\nFailed to append entites to database");
				acedSSFree(js);
				return 1;
			}*/
			AcDbHandle handleG;
			AcDbHandle handleD;
			acedSSAdd(NULL, NULL, jeuSel);
			acedSSAdd(NULL, NULL, jeuSelG);
			acedSSAdd(NULL, NULL, jeuSelD);

			//acutPrintf(_T("\nRESULTAT %i: %i "), i, resultarr.length());
			if ((resultarr.length() % 2) == 1)
				// Un seul objet
			{
				xminG = 999999999999.99;
				yminG = 999999999999.99;
				for (int t = 0; t < resultarr.length(); t++)
				{
					ObjId = resultarr[t];

					acdbGetAdsName(name, ObjId);
					acdbOpenAcDbEntity(name2, ObjId, AcDb::kForRead);
					handleG = ObjId.handle();
					auto ext = AcDbExtents();
					//name2->getGeomExtents(extents);
					es = name2->getGeomExtents(ext);

					//minPoint = extents.minPoint();
					minPoint = ext.minPoint();
					//maxPoint = extents.maxPoint();
					maxPoint = ext.maxPoint();
					p1[0] = minPoint[0];
					p1[1] = minPoint[1];
					p1[2] = minPoint[2];
					p2[0] = maxPoint[0];
					p2[1] = maxPoint[1];
					p2[2] = maxPoint[2];

					acedTrans(p1, rbOrigin, rbFinal, 0, p1res);
					acedTrans(p2, rbOrigin, rbFinal, 0, p2res);

					//if (longueurG < (maxPoint[0] - minPoint[0]))
					if (longueurG < abs(p2res[0] - p1res[0]))
					{
						longueurG = abs(round(p2res[0] - p1res[0]));
					}
					//if (largeurG < (maxPoint[1] - minPoint[1]))
					if (largeurG < abs(p2res[1] - p1res[1]))
					{
						largeurG = abs(round(p2res[1] - p1res[1]));
					}

					if (p1res[0] < xminG)
					{
						xminG = p1res[0];
					}
					if (p1res[1] < yminG)
					{
						yminG = p1res[1];
					}
					name2->close();
					acedSSAdd(name, jeuSelG, jeuSelG);
				}
			}
			// Il y a des solide coupés en 2
			else {
				for (int t = 0; t < resultarr.length(); t++)
				{
					ObjId = resultarr[t];

					acdbGetAdsName(name, ObjId);
					acdbOpenAcDbEntity(name2, ObjId, AcDb::kForRead);

					auto ext = AcDbExtents();
					//name2->getGeomExtents(extents);
					es = name2->getGeomExtents(ext);

					//minPoint = extents.minPoint();
					minPoint = ext.minPoint();
					//maxPoint = extents.maxPoint();
					maxPoint = ext.maxPoint();
					p1[0] = minPoint[0];
					p1[1] = minPoint[1];
					p1[2] = minPoint[2];

					p2[0] = maxPoint[0];
					p2[1] = maxPoint[1];
					p2[2] = maxPoint[2];

					acedTrans(p1, rbOrigin, rbFinal, 0, p1res);
					acedTrans(p2, rbOrigin, rbFinal, 0, p2res);

					xmils = (p2res[0] + p1res[0]) / 2.;
					ymils = (p2res[1] + p1res[1]) / 2.;
					float pi = 3.14159f;
					std::string _Srotation = std::to_string(rotation);
					std::string _SpiSur2 = std::to_string(pi / 2.0);
					std::string _Spi = std::to_string(pi);

					//acutPrintf(_T("\n avant TRANS %f  - (%f %f)   (%f %f) / (%f %f) "), name[0] , p1[0], p1[1], p2[0], p2[1], xmils, ymils);
					//acutPrintf(_T("\n APRES TRANS %f  - (%f %f)   (%f %f) / (%f %f) "), name[0] , p1res[0], p1res[1], p2res[0], p2res[1], xmils, ymils);


					//acutPrintf(_T(" X %f -  %f\n"), xmils, xmil);
					if (xmils < xmil)
						// il est à gauche
					{
						handleG = ObjId.handle();
						if (longueurG < abs(p2res[0] - p1res[0]))
						{
							longueurG = abs(round(p2res[0] - p1res[0]));
						}
						if (largeurG < abs(p2res[1] - p1res[1]))
						{
							largeurG = abs(round(p2res[1] - p1res[1]));
						}
						if (p1res[0] < xminG)
						{
							xminG = p1res[0];
						}

						name2->close();
						acedSSAdd(name, jeuSelG, jeuSelG);
					}
					// à droite
					else {
						handleD = ObjId.handle();
						if (longueurD < (maxPoint[0] - minPoint[0]))
						{
							longueurD = abs(round(p2res[0] - p1res[0]));
						}
						if (largeurD < (maxPoint[1] - minPoint[1]))
						{
							largeurD = abs(round(p2res[1] - p1res[1]));
						}
						if (p1res[0] < xminD)
						{
							xminD = p1res[0];
						}
						//name2->setColorIndex(2);
						name2->close();
						acedSSAdd(name, jeuSelD, jeuSelD);
					}
				}
			}




			int m;
			// Création du bloc coté Gauche
			ads_sslength(jeuSelG, &nbrObjG);
			if (nbrObjG > 0)
			{

				m = handleG;
				origin.x = xminG;
				nomBlock = nomBlockH(nomBlock, m, (int)longueurG, (int)largeurG);
				//acutPrintf(_T("GGGGG->>>> %f / %f ->>>> %s\n"), longueurG, largeurG, nomBlockH);
				double degrees = rotation * (180.0 / 3.141592653589793238463);
				//acutPrintf(_T("Nombre Gauche: %i -  %s\n"), nbrObjG, nomBlockH);
				acedCommandS(RTSTR, L"_block", RTSTR, nomBlock, RT3DPOINT, origin, RTPICKS, jeuSelG, RTSTR, L"", RTNONE);

				// Insertion du bloc
				AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
				AcDbObjectId blkTabId = acdbHostApplicationServices()->workingDatabase()->blockTableId();
				AcDbBlockTable* pBlkTab = NULL;

				acdbOpenAcDbObject((AcDbObject*&)pBlkTab, blkTabId, AcDb::kForRead);
				AcDbObjectId blockId = AcDbObjectId::AcDbObjectId();
				AcDbDatabase* pBlockDb = new AcDbDatabase();

				if (pBlkTab->has(nomBlock))
				{
					pBlkTab->getAt(nomBlock, blockId);
				}

				AcGeMatrix3d matrix3d;
				AcDbDatabase* pDb1 = curDoc()->database();
				AcDbObjectId modelId;
				AcDbObjectId idBlock;
				modelId = acdbSymUtil()->blockModelSpaceId(pDb1);

				acedGetCurrentUCS(matrix3d);

				AcDbBlockTableRecord* pMS = NULL;
				acdbOpenAcDbObject((AcDbObject*&)pMS, modelId, AcDb::kForWrite);

				pointOrigne2D.x = origin[0];
				pointOrigne2D.y = origin[1];
				pointOrigne2D.transformBy(mat2);
				origin[0] = pointOrigne2D.x;
				origin[1] = pointOrigne2D.y;

				AcDbBlockReference* acBlkRef = new AcDbBlockReference(origin, blockId);
				acBlkRef->transformBy(matrix3d);
				acBlkRef->setRotation(rotation);
				acBlkRef->setLayer(layer);

				// Add the block reference to the dwg/database first!!!!
				pMS->appendAcDbEntity(acBlkRef);

				idBlock = acBlkRef->id();

				acBlkRef->close();
				pMS->close();
				pBlkTab->close();

				// Ajout du bloc dans la liste des résultats
				acdbGetAdsName(name, idBlock);
				pSVal = pSVal->rbnext = acutNewRb(RTENAME);
				pSVal->resval.rlname[0] = name[0];
				pSVal->resval.rlname[1] = name[1];
				//acedCommandS(RTSTR, L"_ERASE", RTPICKS, jeuSelG, RTSTR, L"", RTNONE);
			}

			// Création du bloc coté Droit
			ads_sslength(jeuSelD, &nbrObjD);
			if (nbrObjD > 0)
			{
				//acutPrintf(_T("Nombre Droit: %i\n"), nbrObjG);
				m = handleD;
				origin.x = xminD;
				nomBlock = nomBlockH(nomBlock, m, (int)longueurD, (int)largeurD);
				double degrees = rotation * (180.0 / 3.141592653589793238463);

				//acutPrintf(_T("Nombre Droit: %i -  %s\n"), nbrObjG, nomBlockH);
				//origin.transformBy(mat2);
				//acutPrintf(_T("DDDDD->>>> %f / %f ->>>> %s\n"), longueurG, largeurG, nomBlockH);
				acedCommandS(RTSTR, L"_block", RTSTR, nomBlock, RT3DPOINT, origin, RTPICKS, jeuSelD, RTSTR, L"", RTNONE);

				// Insertion du bloc
				AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
				AcDbObjectId blkTabId = acdbHostApplicationServices()->workingDatabase()->blockTableId();
				AcDbBlockTable* pBlkTab = NULL;

				acdbOpenAcDbObject((AcDbObject*&)pBlkTab, blkTabId, AcDb::kForRead);
				AcDbObjectId blockId = AcDbObjectId::AcDbObjectId();
				AcDbDatabase* pBlockDb = new AcDbDatabase();

				if (pBlkTab->has(nomBlock))
				{
					pBlkTab->getAt(nomBlock, blockId);
				}

				AcGeMatrix3d matrix3d;
				AcDbDatabase* pDb1 = curDoc()->database();
				AcDbObjectId modelId;
				AcDbObjectId idBlock;
				modelId = acdbSymUtil()->blockModelSpaceId(pDb1);

				acedGetCurrentUCS(matrix3d);

				AcDbBlockTableRecord* pMS = NULL;
				acdbOpenAcDbObject((AcDbObject*&)pMS, modelId, AcDb::kForWrite);
				pointOrigne2D.x = origin[0];
				pointOrigne2D.y = origin[1];
				pointOrigne2D.transformBy(mat2);
				origin[0] = pointOrigne2D.x;
				origin[1] = pointOrigne2D.y;

				AcDbBlockReference* acBlkRef = new AcDbBlockReference(origin, blockId);
				acBlkRef->transformBy(matrix3d);
				acBlkRef->setRotation(rotation);
				acBlkRef->setLayer(layer);

				// Add the block reference to the dwg/database first!!!!
				pMS->appendAcDbEntity(acBlkRef);

				idBlock = acBlkRef->id();

				acBlkRef->close();
				pMS->close();
				pBlkTab->close();

				// on passe au suivant
				acdbGetAdsName(name, idBlock);
				pSVal = pSVal->rbnext = acutNewRb(RTENAME);
				pSVal->resval.rlname[0] = name[0];
				pSVal->resval.rlname[1] = name[1];

				//acedCommandS(RTSTR, L"_ERASE", RTPICKS, jeuSelD, RTSTR, L"", RTNONE);
			}

			ads_ssfree(jeuSel);
			ads_ssfree(jeuSelG);
			ads_ssfree(jeuSelD);

		}
		else {//self
			pEnt->close();
		}

		eSet.removeAll();

	}


	pSVal = pSVal->rbnext = acutNewRb(RTLE);

	acedRetList(rbList); // Call the function itself, and return the value to AutoLISP.
	acutRelRb(rbList);
	//acedAlert(_T("zzz"));
	return (RSRSLT);
}

