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

class Construction
{
public:
	Construction(IFCObjTemp* ifcObject);

	void Extrusion();

public:
	static std::map<int, ObjectVoid> s_ObjectVoids;

private:
	Acad::ErrorStatus InitLayerTable();
	Acad::ErrorStatus AddTableRecord(const ACHAR* layerName);
	void DrawElement(const ACHAR* layerName, AcDb3dSolid* pSolid, Acad::ErrorStatus es);

	void DrawExtrusion(const ACHAR* layerName);
	void DrawVoids(AcDb3dSolid* solid);

	void HandleDeplacements(AcDb3dSolid* solid, bool move2D = false);
	void DeplacementObjet3DMappedItem(AcDb3dSolid* solid, const Matrix4& transform);
	void DeplacementObjet3D(AcDb3dSolid* solid, const Matrix4& transform);
	void DeplacementObjet2D(AcDb3dSolid* solid, const Matrix4& transform);

	void CreationVoid(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);
	void CreationVoidCircle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);
	void CreationVoidRectangle(AcDb3dSolid* extrusion, ObjectVoid& objectVoid);
	void CreationSection(AcDb3dSolid* extrusion, IFCShapeRepresentation& shapeRepresentation);

	AcDbRegion* CreateCompositeCurve(const std::vector<CompositeCurveSegment>& compositeCurve, const Matrix4& transform);
	AcDb2dPolyline* CreatePolyline(AcGePoint3dArray& ptArr, AcDbVoidPtrArray& lines);
	AcDbRegion* CreateRegion(AcDb2dPolyline* pNewPline, AcDbVoidPtrArray lines, AcDbVoidPtrArray& regions);
	AcDbRegion* CreateRegion(AcDbVoidPtrArray lines, AcDbVoidPtrArray& regions);

	void SetColor(AcDb3dSolid* solid);

	const ACHAR* GetLayerName();
	const wchar_t* ConvertToWideChar(const char* c, ...);
	AcGeVector3d GetExtrusionVector(const Vec3& vector, double height);
	inline bool StepBoolToBoolean(Step::Boolean other) { return other - 1; }

private:
	AcDbDatabase* m_Database;
	AcDbLayerTable* m_LayerTable;
	IFCObjTemp* m_IfcObject;
	Style m_Style;
};