#ifndef CREATEGEOMETRICREPRESENTATIONVISITOR_H_
#define CREATEGEOMETRICREPRESENTATIONVISITOR_H_

#include <ifc2x3/InheritVisitor.h>
#include <ifc2x3/ExpressDataSet.h>

#include <vector>

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

struct CompositeCurveSegmentEx
{
	ifc2x3::IfcTransitionCode transition;
	Step::Boolean sameSense;
};

struct TrimmedCurveEx
{
	double trim1;
	double trim2;
	Step::Boolean senseAgreement;
	ifc2x3::IfcTrimmingPreference preference;
};

struct Circle
{
	AcGePoint3d centre;
	ifc2x3::IfcPositiveLengthMeasure rayon;
};

struct FaceParCompositeCurve
{
	std::vector<CompositeCurveSegmentEx> listCompositeCurveSegmentTrim;
	std::vector<TrimmedCurveEx> listTrimmedCurve;
	std::vector<Circle> listCircle;
	std::vector<std::string> listTypeCompositeCurveSegment;
};




class CreateGeometricRepresentationVisitor : public ifc2x3::InheritVisitor
{
public:

	
	typedef enum { UNDEF_GEOM = 0, FOOTPRINT_CURVE2D, AXIS_CURVE2D, BODY_SWEPTSOLID } GeometryType;
	typedef enum { UNDEF_LOC = 0, POSITION, LOCAL_PLACEMENT } LocationType;
	//! Constructor
	CreateGeometricRepresentationVisitor(ifc2x3::ExpressDataSet* eds);

	virtual bool visitIfcBuildingElementPart(ifc2x3::IfcBuildingElementPart* value);
	virtual bool visitIfcWallStandardCase(ifc2x3::IfcWallStandardCase* value);
	virtual bool visitIfcCovering(ifc2x3::IfcCovering* value);
	virtual bool visitIfcPlate(ifc2x3::IfcPlate* value);
	virtual bool visitIfcSpatialStructureElement(ifc2x3::IfcSpatialStructureElement* value);
	virtual bool visitIfcSlab(ifc2x3::IfcSlab* value);
	virtual bool visitIfcBeam(ifc2x3::IfcBeam* value);
	virtual bool visitIfcSpace(ifc2x3::IfcSpace* value);
	virtual bool visitIfcOpeningElement(ifc2x3::IfcOpeningElement* value);
	virtual bool visitIfcWindow(ifc2x3::IfcWindow* value);
	virtual bool visitIfcProduct(ifc2x3::IfcProduct* value);
	virtual bool visitIfcProductRepresentation(ifc2x3::IfcProductRepresentation* value);
	virtual bool visitIfcRepresentation(ifc2x3::IfcRepresentation* value);
	virtual bool visitIfcFacetedBrep(ifc2x3::IfcFacetedBrep* value);
	virtual bool visitIfcClosedShell(ifc2x3::IfcClosedShell* value);
	virtual bool visitIfcFace(ifc2x3::IfcFace* value);
	virtual bool visitIfcFaceOuterBound(ifc2x3::IfcFaceOuterBound* value);
	virtual bool visitIfcSweptAreaSolid(ifc2x3::IfcSweptAreaSolid* value);
	virtual bool visitIfcExtrudedAreaSolid(ifc2x3::IfcExtrudedAreaSolid* value);
	virtual bool visitIfcPlacement(ifc2x3::IfcPlacement* value);
	virtual bool visitIfcLocalPlacement(ifc2x3::IfcLocalPlacement* value);
	virtual bool visitIfcArbitraryClosedProfileDef(ifc2x3::IfcArbitraryClosedProfileDef* value);
	virtual bool visitIfcCompositeCurve(ifc2x3::IfcCompositeCurve* value);
	virtual bool visitIfcCompositeCurveSegment(ifc2x3::IfcCompositeCurveSegment* value);
	virtual bool visitIfcTrimmedCurve(ifc2x3::IfcTrimmedCurve* value);
	virtual bool visitIfcTrimmingSelect(ifc2x3::IfcTrimmingSelect* value);
	virtual bool visitIfcPolyline(ifc2x3::IfcPolyline* value);
	virtual bool visitIfcPolyLoop(ifc2x3::IfcPolyLoop* value);
	virtual bool visitIfcEdgeLoop(ifc2x3::IfcEdgeLoop* value);
	virtual bool visitIfcOrientedEdge(ifc2x3::IfcOrientedEdge* value);
	virtual bool visitIfcEdgeCurve(ifc2x3::IfcEdgeCurve* value);
	virtual bool visitIfcVertexPoint(ifc2x3::IfcVertexPoint* value);
	virtual bool visitIfcCircle(ifc2x3::IfcCircle* value);
	virtual bool visitIfcAxis2Placement3D(ifc2x3::IfcAxis2Placement3D* value);

	virtual bool visitIfcCoveringType(ifc2x3::IfcCoveringType* value);
	virtual bool visitIfcPropertySet(ifc2x3::IfcPropertySet* value);

	void set2DPolyline(std::vector<double>& poly) { m2DPolyline = poly; mUpdateGeometry = true; }
	void set3DPolyline(std::vector<double>& poly) { m3DPolyline = poly; mUpdateGeometry = true; }
	void setPolyloop(std::vector<double>& poly) { mPolyloop = poly; mUpdateGeometry = true; }
	void setPosition(std::vector<double>& vec) { mPosition = vec; }
	void setLocalPlacement(std::vector<double>& vec) { mLocalPlacement = vec; }
	void setExtrusionDirection(std::vector<double>& vec) { mExtrusionDirection = vec; }
	void setExtrusionDepth(double depth) { mExtrusionDepth = depth; }
	void init();

	inline void setElement(const std::vector<int>& elements) { mElements = elements; }

	inline void setCompositeCurveSegment(CompositeCurveSegmentEx& compositeCurveSegment, TrimmedCurveEx& trimmedCurve, Circle& circle)
	{
		mCompositeCurveSegment.sameSense = compositeCurveSegment.sameSense; mCompositeCurveSegment.transition = compositeCurveSegment.transition; mTrimmedCurve = trimmedCurve; mCircle = circle;
	}

	inline void setListCompositeCurveSegmentTrim(std::vector<CompositeCurveSegmentEx>& listCompositeCurveSegment) { mListCompositeCurveSegmentTrim = listCompositeCurveSegment; }

	inline void setListTrimmedCurve(std::vector<TrimmedCurveEx>& listTrimmedCurve) { mListTrimmedCurve = listTrimmedCurve; }

	inline void setListCCircle(std::vector<Circle>& listCircle) { mListCircle = listCircle; }

	inline void setListTypeCompositeCurveSegment(std::vector<std::string>& listTypeCompositeCurveSegment) { mListTypeCompositeCurveSegment = listTypeCompositeCurveSegment; }

	inline void setListNbPointsPolylineCompositeCurveSegment(std::vector<int>& listNbPointsPolylineCompositeCurveSegment) { mListNbPointsPolylineCompositeCurveSegment = listNbPointsPolylineCompositeCurveSegment; }

	inline void setFaceParCompositeCurve(std::vector<FaceParCompositeCurve>& listFaceCompositeCurve)
	{
		mListFaceCompositeCurve = listFaceCompositeCurve;
	}

	inline void ajoutTypeLoop_type(const std::string& type) { typeLoop.push_back(type); }

protected:
	GeometryType mGeomType;
	LocationType mLocationType;
	Step::RefPtr< ifc2x3::ExpressDataSet > mDataSet;
	bool mPolyloopMustBeClosed;
	std::vector<double> m2DPolyline;
	std::vector<double> m3DPolyline;
	std::vector<double> mPosition;
	std::vector<double> mLocalPlacement;
	std::vector<double> mExtrusionDirection;
	std::vector<double> mPolyloop;
	std::vector< ifc2x3::IfcPositiveLengthMeasure> mRayon;
	double mExtrusionDepth;
	bool mUpdateGeometry;
	Step::RefPtr< ifc2x3::IfcPolyline > mPolyline;
	int mOffset = 0;

	std::vector<int> mElements;
	int mFaceIndex = 0;

	bool vertexS = false;
	bool vertexE = false;

	std::vector<std::string> mListTypeCompositeCurveSegment;
	std::vector<int> mListNbPointsPolylineCompositeCurveSegment;
	std::vector<CompositeCurveSegmentEx> mListCompositeCurveSegmentTrim;
	std::vector<TrimmedCurveEx> mListTrimmedCurve;
	std::vector<Circle> mListCircle;
	CompositeCurveSegmentEx mCompositeCurveSegment;
	TrimmedCurveEx mTrimmedCurve;
	Circle mCircle;

	std::vector<FaceParCompositeCurve> mListFaceCompositeCurve;
	int indexListCompositeCurveSegment = 0;
	int indexCompositeCurvePoly = 0;
	int indexFace;
	int indexTypeLoop;
	int indexListTrimmedCurve = 0;
	int indexListCircle = 0;

	std::vector<std::string> typeLoop;


	//  		4  3
	//		 |  |  |
	int test[8][8][8];

	// triple tableau
	// tableau: 4, 4, 4, 8, 9, 
};

#endif // ** CREATEGEOMETRICREPRESENTATIONVISITOR_H_ ** //


Step::RefPtr<ifc2x3::IfcFaceOuterBound> faceOuterBound;
ifc2x3::Set_IfcFaceBound_1_n faceBound_1_n;

for (size_t i = 0; i < 1; i++)
{
	faceOuterBound = mDataSet->createIfcFaceOuterBound();
	result &= faceOuterBound->acceptVisitor(this);
	faceBound_1_n.emplace(faceOuterBound);
}

indexListCompositeCurveSegment = 0;
rpValue->setBounds(faceBound_1_n);