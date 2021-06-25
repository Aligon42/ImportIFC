#pragma once

#include <string>
#include <vector>
#include <list>

#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

typedef mathfu::Vector<float, 3> Vec3;
typedef mathfu::Matrix<float, 4> Matrix4;

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

struct Style
{
    int keyItem;
    double red;
    double green;
    double blue;
    double transparence;
};

struct Box
{
    Vec3 Corner;
    int XDimBox;
    int YDimBox;
    int ZDimBox;
};

struct Object
{
	std::string NameProfilDef;
	Vec3 VecteurExtrusion;
	float HauteurExtrusion;
	Matrix4 Transform;
	std::list<Vec3> Points;
	Matrix4 TransformFace;
	std::vector<std::string> NameItems;
	Style StyleDessin; 
	bool IsMappedItem; 
	Matrix4 TransformationOperator3D;
    std::vector<int> KeyItems;
    std::string OuterCurveName;
    std::vector<int> ListNbArg;
    std::list<Matrix4> ListPlan;
    std::list<Matrix4> ListLocationPolygonal;
    std::vector<bool> AgreementHalf;
    std::vector<bool> AgreementPolygonal;
    std::vector<std::string> ListEntityHalf;
    std::vector<std::string> ListEntityPolygonal;
    CompositeCurveSegment CompositeCurveSegment;
    int NbPolylineComposite;
};