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

struct Face
{
	std::string TypeFace;

	Face(const std::string& type) : TypeFace(type) {}
	virtual ~Face() {}
};

struct Point2D
{
	double X, Y;

	Point2D(double x, double y)
		: X(x), Y(y) {}
};

struct Point3D
{
	double X, Y, Z;

	Point3D(double x, double y, double z) 
		: X(x), Y(y), Z(z) {}
};

struct PolylineEx : public Face
{
	std::vector<Point3D> Points;

	PolylineEx() : Face("polyline") {}
	~PolylineEx() {}
};

struct CompositeCurveSegmentEx
{
	std::string TypeSegment;
	ifc2x3::IfcTransitionCode Transition;
	Step::Boolean SameSense;

	CompositeCurveSegmentEx() {}
	CompositeCurveSegmentEx(const std::string& type) 
		: TypeSegment(type) {}
};

struct CompositeCurve : public Face
{
	std::vector<std::shared_ptr<CompositeCurveSegmentEx>> CompositeCurveSegments;

	CompositeCurve() : Face("compositeCurve") {}
	~CompositeCurve();
};

struct CompositeCurveSegmentPolyline : CompositeCurveSegmentEx
{
	std::vector<Point3D> Points;

	CompositeCurveSegmentPolyline() : CompositeCurveSegmentEx("polyline") {}
};

struct Circle
{
	AcGePoint3d Centre;
	ifc2x3::IfcPositiveLengthMeasure Rayon;
};

struct TrimmedCurveEx : CompositeCurveSegmentEx
{
	Circle Circle;
	double Trim1;
	double Trim2;
	Step::Boolean SenseAgreement;
	ifc2x3::IfcTrimmingPreference Preference;

	TrimmedCurveEx() : CompositeCurveSegmentEx("trimmedCurve") {}
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
	void set3DPolyline(std::vector<double>& poly) {
		for (int i = 0; i < poly.size(); i++)
		{
			m3DPolyline.push_back(poly[i]);
		} mUpdateGeometry = true; }
	void setPolyloop(std::vector<double>& poly) {
		for (int i = 0; i < poly.size(); i++)
		{
			mPolyloop.push_back(poly[i]);
		} 
		mUpdateGeometry = true;
	}
	void setPosition(std::vector<double>& vec) { mPosition = vec; }
	void setLocalPlacement(std::vector<double>& vec) { mLocalPlacement = vec; }
	void setExtrusionDirection(std::vector<double>& vec) { mExtrusionDirection = vec; }
	void setExtrusionDepth(double depth) { mExtrusionDepth = depth; }
	void init();

	inline void AjoutFace(const std::vector<std::shared_ptr<Face>>& faces) { mFaces = faces; }

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

	bool vertexS = false;
	bool vertexE = false;

	std::vector<std::shared_ptr<Face>> mFaces;
	int mCurrentFaceIndex = 0;
	int mCurrentCompositeCurveIndex = 0;
	bool mIsCompositeCurve = false;
};

#endif // ** CREATEGEOMETRICREPRESENTATIONVISITOR_H_ ** //