#include <iostream>
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
#include "acedCmdNF.h"
#include "dbSubD.h"
#include "geassign.h"
#include "adscodes.h"
#include "dbtrans.h"
#include <wchar.h>
#include <math.h>  

static int failure_results=0;
static int success_results=0;

#define TEST_VALIDITY(X) \
	if (!X.valid()) {\
	std::cerr << std::endl << "   Failure: " << #X << " is not valid" << std::endl << std::endl; \
	++failure_results; \
	} else {\
	std::cerr << "   Success: " << #X << " is valid" << std::endl; \
	++success_results; \
	}

#define TEST_FAILURE(X) \
{ \
	std::cerr << std::endl << "   Failure: " << X << std::endl << std::endl; \
	return 1; \
}

#define TEST_ASSERT(X) \
	if (!(X)) {\
	std::cerr << std::endl << "   Failure : " << #X << std::endl << std::endl; \
	++failure_results; \
	} else {\
	std::cerr << "   Success : " << #X << std::endl; \
	++success_results; \
	}

#define TEST_EXCEPTION(X,E) \
	try {\
	X; \
	std::cerr  << std::endl << "   Failure : exception " << #E << " not caught" << std::endl << std::endl; \
	++failure_results; \
} catch (E){\
	std::cerr << "   Success : exception " << #E << " caught" << std::endl; \
	++success_results; \
}

void test();

void dessinProfilDef(int key, std::string & NameProfilDef, std::string & entity, Vec3 & VecteurExtrusion, float hauteurExtrusion, Matrix4 & transform1, Matrix4 & transformation, Matrix4 & transformation2D, CreateConstructionPointVisitor & visitor1, std::list<Vec3>&points1, Matrix4 & transformFace, std::vector<std::string>&nameItems, int keyItem, std::vector<int>&keyItems, std::string & outerCurveName, std::vector<int>&ListNbArg, std::list<Matrix4>&listPlan, std::list<Matrix4>&listLocationPolygonal, std::vector<Step::Boolean>&AgreementHalf, std::vector<Step::Boolean>&AgreementPolygonal, std::vector<std::string>&listEntityHalf, std::vector<std::string>&listEntityPolygonal, std::vector<ObjectVoid>&listVoid, CompositeCurveSegment & _compositeCurveSegment, int nbPolylineComposite, int nbCompositeCurve, int i, std::map<int, Style>&vectorStyle, bool isMappedItem, Matrix4 & transformationOperator3D, double scale, std::vector<Vec3>&VecteurExtrusionBool, std::vector<float>&hauteurExtrusionBool, std::vector<Matrix4>&transformationBoolExtrud, std::vector<std::string>&NameProfilDefBool, std::vector<Rectangle_profilDef>&RectangleProfilDefBool);