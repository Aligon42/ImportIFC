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

#define PI 3.14159265358979323846


struct ObjectVoid
{
	int keyForVoid;
	std::string NameProfilDef;
	Vec3 VecteurExtrusion;
	std::list<Vec3> points1;
	std::vector<int> nbArg;
	float XDim;
	float YDim;
	float radius;
	std::list<Matrix4> listPlan;
	std::list<Matrix4> listLocationPolygonal;
	std::vector<bool> AgreementHalf;
	std::vector<bool> AgreementPolygonal;
	std::vector<std::string> listEntityHalf;
	std::vector<std::string> listEntityPolygonal;
	Matrix4 transform1;
};

static std::vector<ObjectVoid> listVoid;
static ObjectVoid _objectVoid;

void extrusion(int key, std::vector<std::string> nameItems, std::list<Vec3> points1, std::vector<int> nbArg, Vec3 VecteurExtrusion,
	Matrix4 transform1, std::list<Matrix4> listPlan, std::list<Matrix4> listLocationPolygonal,
	std::vector<bool> AgreementHalf, std::vector<bool> AgreementPolygonal,
	std::vector<std::string> listEntityHalf, std::vector<std::string> listEntityPolygonal,
	std::vector<ObjectVoid> listVoid, CompositeCurveSegment _compositeCurveSegment);
static void DeplacementObjet3D(AcDb3dSolid* pSolid, Matrix4 transform1);
static void DeplacementObjet3D(AcDbPlaneSurface* pSurface, Matrix4 transform1);
static void CreationSection(AcDb3dSolid* extrusion, Vec3 VecteurExtrusion, std::list<Vec3>& points1,
	std::vector<int>& nbArg, std::list<Matrix4>& listPlan, std::list<Matrix4>& listLocationPolygonal,
	std::vector<bool>& AgreementHalf, std::vector<bool>& AgreementPolygonal, std::vector<std::string>& listEntityHalf,
	std::vector<std::string>& listEntityPolygonal);

//Void
static void CreationVoid(AcDb3dSolid* extrusion, ObjectVoid Void);
static void CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid Void);
static void CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid Void);

//profilDef
void createSolid3dProfilIPE(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilIPN(I_profilDef IprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilL8(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilL9(L_profilDef LprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilT10(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilT12(T_profilDef TprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilUPE(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilUPN(U_profilDef UprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilC(C_profilDef CprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilZ(Z_profilDef ZprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilAsyI(AsymmetricI_profilDef AsymmetricIprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilCircHollow(CircleHollow_profilDef CircleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilRectHollow(RectangleHollow_profilDef RectangleHollowprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createSolid3dProfilRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
AcDbRegion* createPolyCircle(Circle_profilDef CircleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
AcDbRegion* createPolyRectangle(Rectangle_profilDef RectangleprofilDef, Vec3 VecteurExtrusion, Matrix4 transform1);
void createBoundingBox();
void createFaceSolid(std::list<Vec3> points1, std::vector<int> ListNbArg, bool orientation, Matrix4 transformFace);
AcDbRegion* createCompositeCurve(CompositeCurveSegment _compositeCurveSegment, Matrix4 transform);