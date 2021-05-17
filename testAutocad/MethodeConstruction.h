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

void createSolid3d(std::list<Vec3> points1, std::vector<int> nbArg, Vec3 VecteurExtrusion, Matrix4 tranform1, std::list<Matrix4> listPlan, std::list<Matrix4> listLocationPolygonal, std::vector<bool> AgreementHalf, std::vector<bool> AgreementPolygonal, std::vector<std::string> listEntityHalf, std::vector<std::string> listEntityPolygonal);
static void DeplacementObjet3D(AcDb3dSolid* pSolid, Matrix4 transform1);
static void CreationSection(AcDb3dSolid* extrusion, Vec3 VecteurExtrusion, std::list<Vec3> points1, std::vector<int> nbArg, std::list<Matrix4> listPlan, std::list<Matrix4> listLocationPolygonal, std::vector<bool> AgreementHalf, std::vector<bool> AgreementPolygonal, std::vector<std::string> listEntityHalf, std::vector<std::string> listEntityPolygonal);