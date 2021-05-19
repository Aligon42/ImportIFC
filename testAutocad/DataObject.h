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
	std::string EntityType;
	Vec3 VecteurExtrusion;
	Matrix4 Transformation;
};

struct DataObject : public BaseObject
{
	int Key;
	std::list<Vec3> Points;
	std::vector<int> NbArgs;
	std::list<Matrix4> ListePlan;
	std::list<Matrix4> ListeLocationPolygonal;
	std::vector<bool> AgreementHalf;
	std::vector<bool> AgreementPolygonal;
	std::vector<std::string> ListEntityHalf;
	std::vector<std::string> ListEntityPolygonal;
};