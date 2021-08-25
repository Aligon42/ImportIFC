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
#include "dbSubD.h"

#include <chrono>

#include "Object.h"

#define PI 3.141592653589793

class Timer
{
public:
	Timer(int key, const std::string& entity, const std::string& name);
	~Timer();

private:
	std::chrono::steady_clock::time_point m_StartTime;
	std::string m_Name, m_Entity;
	int m_Key;
};

class Construction
{
public:
	Construction();
	Construction(std::shared_ptr<IFCObject> ifcObject);
	~Construction();

	void Extrusion();
	void CreationFaceSolid();

	// ProfilDef
	void CreateSolid3dProfilIPE(I_profilDef& profilDef);
	void CreateSolid3dProfilIPN(I_profilDef& profilDef);
	void CreateSolid3dProfilL8(L_profilDef& profilDef);
	void CreateSolid3dProfilL9(L_profilDef& profilDef);
	void CreateSolid3dProfilT10(T_profilDef& profilDef);
	void CreateSolid3dProfilT12(T_profilDef& profilDef);
	void CreateSolid3dProfilUPE(U_profilDef& profilDef);
	void CreateSolid3dProfilUPN(U_profilDef& profilDef);
	void CreateSolid3dProfilC(C_profilDef& profilDef);
	void CreateSolid3dProfilZ(Z_profilDef& profilDef);
	void CreateSolid3dProfilAsyI(AsymmetricI_profilDef& profilDef);
	void CreateSolid3dProfilCircHollow(CircleHollow_profilDef& profilDef);
	void CreateSolid3dProfilRectHollow(RectangleHollow_profilDef& profilDef);
	void CreateSolid3dProfilCircle(Circle_profilDef& profilDef);
	void CreateSolid3dProfilRectangle(Rectangle_profilDef& profilDef);

public:
	static std::string s_LogPath;
	static std::map<std::string, std::vector<std::string>> s_Logs;
	static std::map<int, std::vector<std::shared_ptr<IFCObject>>> s_ObjectVoids;
	static std::map<int, Style> s_Styles;

private:
	Acad::ErrorStatus InitLayerTable();
	Acad::ErrorStatus AddTableRecord(const ACHAR* layerName);
	void DrawElement(const ACHAR* layerName, AcDb3dSolid* pSolid, Acad::ErrorStatus& es);
	void DrawElement(const ACHAR* layerName, AcDbSubDMesh* pSubDMesh, AcArray<AcCmEntityColor>& clrArray, Acad::ErrorStatus& es);

	void DrawExtrusion(const ACHAR* layerName);
	void DrawFaces(const ACHAR* layerName);
	void DrawFacesMappedItem(IFCShapeRepresentation& shape, IFCShapeRepresentation& mappedShape);
	void DrawVoids(AcDb3dSolid* solid);

	void HandleDeplacements(AcDb3dSolid* solid, IFCShapeRepresentation& shape, bool move2D = false);
	void HandleDeplacements(AcDbSubDMesh* pSubDMesh, IFCShapeRepresentation& shape, bool move2D = false);
	void HandleDeplacements(AcDb3dSolid* solid, bool move2D = false, ProfilDef* profilDef = nullptr);
	void HandleDeplacements(AcDbSubDMesh* pSubDMesh, bool move2D = false, ProfilDef* profilDef = nullptr);
	void DeplacementObjet3DMappedItem(AcDb3dSolid* solid, const Matrix4& transform);
	void DeplacementObjet3DMappedItem(AcDbSubDMesh* pSubDMesh, const Matrix4& transform);
	void DeplacementObjet(AcDb3dSolid* solid, const Matrix4& transform);
	void DeplacementObjet(AcDbSubDMesh* pSubDMesh, const Matrix4& transform);

	void CreationVoid(AcDb3dSolid* extrusion, std::shared_ptr<IFCObject> objectVoid);
	void CreationVoidCircle(AcDb3dSolid* extrusion, std::shared_ptr<IFCObject> objectVoid);
	void CreationVoidRectangle(AcDb3dSolid* extrusion, std::shared_ptr<IFCObject> objectVoid);
	void CreationSection(AcDb3dSolid* extrusion, IFCShapeRepresentation& shapeRepresentation);
	void CreationProfilDef(AcDbRegion* pRegion, AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions, const AcGeVector3d& vecExtru, ProfilDef* profilDef = nullptr);

	AcDbRegion* CreateCompositeCurve(const std::vector<CompositeCurveSegment>& compositeCurve, const Matrix4& transform);
	AcDb2dPolyline* CreatePolyline(AcGePoint3dArray& ptArr, AcDbVoidPtrArray& lines, bool shouldTransform = false, AcGeMatrix3d* matrix3d = nullptr, AcGeVector3d* acVec3d = nullptr);
	AcDbCircle* CreateCircle(AcGePoint3d& center, double radius, AcDbVoidPtrArray& lines, bool shouldTransform = false, AcGeMatrix3d* matrix3d = nullptr, AcGeVector3d* acVec3d = nullptr);
	AcDbRegion* CreateRegion(AcDb2dPolyline* pNewPline, AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions);
	AcDbRegion* CreateRegion(AcDbCircle* pCircle, AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions);
	AcDbRegion* CreateRegion(AcDbVoidPtrArray& lines, AcDbVoidPtrArray& regions);

	void SetColor(AcDb3dSolid* solid, int colorIndex);
	void SetColor(AcDbSubDMesh* pSubDMesh, int colorIndex);

	const ACHAR* GetLayerName(std::string& entity);
	const wchar_t* ConvertToWideChar(const char* c, ...);
	AcGeVector3d GetExtrusionVector(const Vec3& vector, double height);
	inline bool StepBoolToBoolean(Step::Boolean other) { return other - 1; }

private:
	std::chrono::steady_clock::time_point m_StartTime;

	AcDbDatabase* m_Database;
	AcDbLayerTable* m_LayerTable;
	std::shared_ptr<IFCObject> m_IfcObject;
};