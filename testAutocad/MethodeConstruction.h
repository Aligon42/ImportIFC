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
#include "dbSubD.h"

#include "Object.h"

#define PI 3.141592653589793

struct ObjectVoid
{
	int keyForVoid;
	std::string NameProfilDef;
	Vec3 VecteurExtrusion;
	float hauteurExtrusion;
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

const wchar_t* GetWCM(const char* c, ...);

void extrusion(int key, std::string entity, IFCObject& object, std::vector<ObjectVoid>& listVoid, Style styleDessin);
void createBoundingBox(Box box,std::string entity, Style styleDessin);
static void DeplacementObjet3D(AcDb3dSolid* pSolid, Matrix4 transform1);
static void DeplacementObjet3D(AcDbSubDMesh* pSubDMesh, Matrix4 transform1);
static void DeplacementObjet3DMappedItem(AcDb3dSolid* pSolid, Matrix4 transformationOperator3D);
static void DeplacementObjet3DMappedItem(AcDbSubDMesh* pSubDMesh, Matrix4 transformationOperator3D);
static void CreationSection(AcDb3dSolid* extrusion, IFCObject object);

//Void
static void CreationVoid(AcDb3dSolid* extrusion, ObjectVoid Void, CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, int nbPolylineComposite, bool isMappedItem, Matrix4 transformationOperator3D);
static void CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid Void, CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, int nbPolylineComposite, bool isMappedItem, Matrix4 transformationOperator3D);
static void CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid Void, CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, int nbPolylineComposite, bool isMappedItem, Matrix4 transformationOperator3D);

//profilDef
void createSolid3dProfilIPE(I_profilDef IprofilDef, Style styleDessin);
void createSolid3dProfilIPN(I_profilDef IprofilDef, Style styleDessin);
void createSolid3dProfilL8(L_profilDef LprofilDef, Style styleDessin);
void createSolid3dProfilL9(L_profilDef LprofilDef, Style styleDessin);
void createSolid3dProfilT10(T_profilDef TprofilDef, Style styleDessin);
void createSolid3dProfilT12(T_profilDef TprofilDef, Style styleDessin);
void createSolid3dProfilUPE(U_profilDef UprofilDef, Style styleDessin);
void createSolid3dProfilUPN(U_profilDef UprofilDef, Style styleDessin);
void createSolid3dProfilC(C_profilDef CprofilDef, Style styleDessin);
void createSolid3dProfilZ(Z_profilDef ZprofilDef, Style styleDessin);
void createSolid3dProfilAsyI(AsymmetricI_profilDef AsymmetricIprofilDef, Style styleDessin);
void createSolid3dProfilCircHollow(CircleHollow_profilDef CircleHollowprofilDef, Style styleDessin);
void createSolid3dProfilRectHollow(RectangleHollow_profilDef RectangleHollowprofilDef, Style styleDessin);
void createSolid3dProfilCircle(Circle_profilDef CircleprofilDef, Style styleDessin);
void createSolid3dProfilRectangle(Rectangle_profilDef RectangleprofilDef, Style styleDessin);
void createFaceSolid(std::string entity, IFCObject& object, std::map<int, Style>* listStyles);
AcDbRegion* createCompositeCurve(CompositeCurveSegment _compositeCurveSegment, Matrix4 transform, bool isMappedItem, Matrix4 transformationOperator3D);
float roundoff(float value, unsigned char prec);