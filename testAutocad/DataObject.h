#pragma once
#include <list>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>
#include <vector>
#include <string>

typedef mathfu::Vector<float, 3> Vec3;
typedef mathfu::Matrix<float, 4> Matrix4;

struct BaseObject
{
	int Key;
	std::string Type;
};

struct ElementToConstruct : public BaseObject
{
	std::list<Vec3> Points;
	std::vector<int> Args;
	std::list<Matrix4> Plans;
	std::list<Matrix4> LocationsPolygonal;
	std::vector<bool> AgreementHalf;
	std::vector<bool> AgreementPolygonal;
	std::vector<std::string> EntitiesHalf;
	std::vector<std::string> EntitiesPolygonal;
};

struct ObjectToConstruct : public BaseObject
{
	Vec3 VecteurExtrusion;
	Matrix4 Transform;
	std::vector<ElementToConstruct> ElementsToConstruct;
};

struct ObjectVoid : ObjectToConstruct
{
	std::string NameProfilDef;
	float XDim;
	float YDim;
	float Radius;
};

struct TrimmedCurve
{
	Vec3 centreCircle;
	float radius;
	int trim1;
	int trim2;
	bool senseArgreement;
};

struct CompositeCurveSegment
{
	std::vector<std::vector<Vec3>> listPolyligne;
	std::vector<TrimmedCurve> listTrimmedCurve;
	std::vector<std::string> listParentCurve;
};