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
#include "Core.h"

class MethodeConstruction
{
public:
	void createSolid3d(Ref<ObjectToConstruct> object);
	
	//Base profilDef
	void createSolid3dProfil(Ref<BaseProfilDef> profilDef);

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

	void createSolid3dProfil(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(C_profilDef CprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(Z_profilDef ZprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(AsymmetricI_profilDef AsymmetricIprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(CircleHollow_profilDef CircleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(RectangleHollow_profilDef RectangleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	void createSolid3dProfil(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);

	//Void
	void CreationVoid(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);
	void CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);
	void CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);

	AcDbRegion* createPolyCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	AcDbRegion* createPolyRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
	AcDbRegion* createCompositeCurve(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform);
};