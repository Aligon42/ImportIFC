#pragma once
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
#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>
#include "CreateConstructionPointVisitor.h"
#include "ProfilDef.h"
#include "DataObject.h"

class MethodeConstruction
{
public:
	void createSolid3d(ObjectToConstruct object);
	
	//Base profilDef
	void createSolid3dProfil(BaseProfilDef* profilDef);
	

	inline std::vector<ObjectVoid>& getObjectsVoid() { return _listVoid; }
	inline void AddObjectVoid(ObjectVoid objv) { _listVoid.push_back(objv); }

private:
	std::vector<ObjectVoid> _listVoid;

private:
	void DeplacementObjet3D(AcDb3dSolid* pSolid, Matrix4 transform);
	void CreationSection(AcDb3dSolid* extrusion, Vec3 vecteurExtrusion, ElementToConstruct& elementToConstruct);

	//profilDef
	void drawForProfil(AcGePoint3dArray* ptArr[], AcGeVector3d* acVec3d[], int count, Vec3& vecteurExtrusion, Matrix4& transform);
	void drawForProfilCircle(AcDbCircle* circle[], AcGeVector3d* acVec3d[], int count, Vec3& vecteurExtrusion, Matrix4& transform);

	void createSolid3dProfilIPE(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilIPN(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilL8(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilL9(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilT10(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilT12(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilUPE(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilUPN(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilC(C_profilDef CprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilZ(Z_profilDef ZprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilAsyI(AsymmetricI_profilDef AsymmetricIprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilCircHollow(CircleHollow_profilDef CircleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilRectHollow(RectangleHollow_profilDef RectangleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfilRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);

	//Void
	void CreationVoid(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);
	void CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);
	void CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);

	AcDbRegion* createPolyCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	AcDbRegion* createPolyRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	AcDbRegion* createCompositeCurve(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
};