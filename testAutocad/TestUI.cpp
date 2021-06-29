#include "tchar.h"
#include <aced.h>
#include <rxregsvc.h> 

#include <ifc2x3/SPFReader.h>
#include <ifc2x3/SPFWriter.h>
#include <ifc2x3/ExpressDataSet.h>
#include <ifc2x3/IfcProject.h>
#include <ifc2x3/IfcLocalPlacement.h>
#include <ifc2x3/IfcAxis2Placement.h>
#include <ifc2x3/IfcAxis2Placement2D.h>
#include <ifc2x3/IfcAxis2Placement3D.h>

#include <Step/CallBack.h>

#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

#include "CreateConstructionPointVisitor.h"
#include "ComputePlacementVisitor.h"
#include "MethodeConstruction.h"
#include "tests.h"
#include <adscodes.h>

#include <iostream>

#define DISTANCE_TOLERANCE 0.001

bool equals(const Vec3& lhs, const Vec3& rhs)
{
    return (lhs - rhs).Length() < DISTANCE_TOLERANCE;
}

std::ostream& operator<<(std::ostream& os, const Vec3& v)
{
    os << "[ " << v.x() << ", "
        << v.y() << ", "
        << v.z() << " ]";
    return os;
}

#define PRINT_VALUE(x) \
    Log("    %s = %s\n", #x, x)

class ConsoleCallBack : public Step::CallBack
{
public:
    ConsoleCallBack() : _max(1) {}
    virtual void setMaximum(size_t max) { _max = max; }
    virtual void setProgress(size_t progress) { std::cerr << double(progress) / double(_max) * 100.0 << "%" << std::endl; }
    virtual bool stop() const { return false; }

protected:
    size_t _max;
};

void initApp();
void unloadApp();

const wchar_t* GetWC(const char* c, ...)
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);

    return wc;
}

#define Log(x, ...) \
    acutPrintf(_T(x), ...)

#define Log(x) \
    acutPrintf(_T(x))

void test()
{
    AcDbObjectId transId;
    TCHAR* fname;

    struct resbuf* rb;
    // Get a ifc file from the user.
    //
    rb = acutNewRb(RTSTR);
    int stat = acedGetFileD(_T("Pick a IFC file"), NULL, _T("ifc"), 0, rb);
    if ((stat != RTNORM) || (rb == NULL))
    {
        acutPrintf(_T("\nYou must pick a ifc file."));
        return;
    }
    fname = (TCHAR*)acad_malloc((_tcslen(rb->resval.rstring) + 1) * sizeof(TCHAR));
    _tcscpy(fname, rb->resval.rstring);
    acutRelRb(rb);

    Log("Simple read/write example of Ifc2x3 SDK\n");
    bool shouldWrite = false;
    bool inMemory = false;

    // ** open, load, close the file
    std::ifstream ifcFile;
    ifcFile.open(fname);

    ifc2x3::SPFReader reader;
    ConsoleCallBack cb;
    //    reader.setCallBack(&cb);

    if (ifcFile.is_open())
    {
        acutPrintf(_T("\nreading file ..."));
    }
    else
    {
        acutPrintf(_T("ERROR: failed to open <%s>\n"), fname);
    }

    // get length of file
    ifcFile.seekg(0, ifcFile.end);
    std::ifstream::pos_type length = ifcFile.tellg();
    ifcFile.seekg(0, ifcFile.beg);

    bool result = reader.read(ifcFile, inMemory ? length : (std::ifstream::pos_type)0);
    ifcFile.close();

    if (result)
        acutPrintf(_T("OK!!\n"));
    else
    {
        Log("Ho no, there is a PROBLEM!!\n");
        std::vector<std::string> errors = reader.errors();
        std::vector<std::string>::iterator it = errors.begin();
        while (it != errors.end())
        {
            acutPrintf(_T("%s\n"), *it);
            ++it;
        }
    }

    // ** get the model
    ifc2x3::ExpressDataSet* expressDataSet = dynamic_cast<ifc2x3::ExpressDataSet*>(reader.getExpressDataSet());

    if (expressDataSet == NULL)
    {
        acutPrintf(_T("Ho no ... there is no ExpressDataSet.\n"));
    }

    if (shouldWrite)
    {
        // ** Instantiate the model if we want to write it
        expressDataSet->instantiateAll(/*&cb*/);
    }

    // ** Check the root of the model
    Step::RefLinkedList< ifc2x3::IfcProject > projects = expressDataSet->getAllIfcProject();
    if (projects.size() == 0) {
        acutPrintf(_T("Strange ... there is no IfcProject\n"));
    }
    else if (projects.size() > 1) {
        acutPrintf(_T("Strange ... there more than one IfcProject\n"));
    }
    else {
        Step::RefPtr< ifc2x3::IfcProject > project = &*(projects.begin());
        //Log("Project name is: %s\n", GetWC(project->getName().toISO_8859(Step::String::Western_European)));
        acutPrintf(_T("Project name is: %s\n"), GetWC((project->getName().toISO_8859(Step::String::Western_European)).c_str()));
        if (Step::isUnset(project->getLongName().toISO_8859(Step::String::Western_European))) {
            project->setLongName("Je lui donne le nom que je veux");
        }
        acutPrintf(_T("Project long name is: %s\n"), GetWC((project->getLongName().toISO_8859(Step::String::Western_European)).c_str()));
    }

    //TEST_ASSERT(expressDataSet != nullptr);
    expressDataSet->instantiateAll();
    ComputePlacementVisitor placementVisitor;

    for (auto& voids : expressDataSet->getAllIfcRelVoidsElement())
    {
        CreateConstructionPointVisitor visitor1;
        int key = (int)voids.getKey();
        
        voids.acceptVisitor(&visitor1);
        _objectVoid.keyForVoid = visitor1.getkeyForVoid();
        _objectVoid.NameProfilDef = visitor1.getNameProfildef();
        if (_objectVoid.NameProfilDef == "IfcArbitraryClosedProfileDef")
        {
            _objectVoid.points1 = visitor1.getPoints();
            _objectVoid.nbArg = visitor1.getNbArgPolyline();
        }
        else if (_objectVoid.NameProfilDef == "IfcCircleProfileDef")
        {
            _objectVoid.radius = visitor1.getCircleprofilDef().Radius;
        }
        else if (_objectVoid.NameProfilDef == "IfcRectangleProfileDef")
        {
            _objectVoid.XDim = visitor1.getRectangleprofilDef().XDim;
            _objectVoid.YDim = visitor1.getRectangleprofilDef().YDim;
        }
        _objectVoid.VecteurExtrusion = visitor1.getVectorDirection();
        _objectVoid.hauteurExtrusion = visitor1.getHauteurExtrusion();
        _objectVoid.listPlan = visitor1.getPlanPolygonal();
        _objectVoid.listLocationPolygonal = visitor1.getLocationPolygonal();
        _objectVoid.AgreementHalf = visitor1.getAgreementHalfBool();
        _objectVoid.AgreementPolygonal = visitor1.getAgreementPolygonalBool();
        _objectVoid.listEntityHalf = visitor1.getListEntityHalf();
        _objectVoid.listEntityPolygonal = visitor1.getListEntityPolygonal();

        voids.acceptVisitor(&placementVisitor);
        _objectVoid.transform1 = placementVisitor.getTransformation();
        Matrix4 transformation = visitor1.getTransformation();

        _objectVoid.transform1 *= transformation;

        listVoid.push_back(_objectVoid);

    }

    for (auto& styles : expressDataSet->getAllIfcStyledItem())
    {
        CreateConstructionPointVisitor visitor1;
        int key = styles.getKey();

        styles.acceptVisitor(&visitor1);
        style.keyItem = visitor1.getStyle().keyItem;
        style.red = visitor1.getStyle().red;
        style.green = visitor1.getStyle().green;
        style.blue = visitor1.getStyle().blue;
        style.transparence = visitor1.getStyle().transparence;
        listStyle.push_back(style);
    }

    for (auto& site : expressDataSet->getAllIfcSite())
    {
        int key = (int)site.getKey();
        std::string entity = site.getType().getName();
        CreateConstructionPointVisitor visitor1;
        site.acceptVisitor(&visitor1);

        std::vector<std::string> nameItems = visitor1.getNameItems();
        std::vector<int> keyItems = visitor1.getListKeyItem();
        std::list<Vec3> points1 = visitor1.getPoints();
        std::string outerCurveName = visitor1.getOuterCurveName();
        std::vector<int> ListNbArg = visitor1.getNbArgPolyline();
        Vec3 VecteurExtrusion = visitor1.getVectorDirection();
        float hauteurExtrusion = visitor1.getHauteurExtrusion();
        std::list<Matrix4> listPlan = visitor1.getPlanPolygonal();
        std::list<Matrix4> listLocationPolygonal = visitor1.getLocationPolygonal();
        std::vector<bool> AgreementHalf = visitor1.getAgreementHalfBool();
        std::vector<bool> AgreementPolygonal = visitor1.getAgreementPolygonalBool();
        std::vector<std::string> listEntityHalf = visitor1.getListEntityHalf();
        std::vector<std::string> listEntityPolygonal = visitor1.getListEntityPolygonal();
        CompositeCurveSegment _compositeCurveSegment = visitor1.getCompositeCurveSegment();
        int nbPolylineComposite = visitor1.getnbPolylineCompositeCurve();

        Box box = visitor1.getBox();

        site.acceptVisitor(&placementVisitor);
        Matrix4 transform1 = placementVisitor.getTransformation();
        Matrix4 transformation = visitor1.getTransformation();
        Matrix4 transformFace = transform1;

        transform1 *= transformation;

        bool isMappedItem = visitor1.getIsMappedItem();
        Matrix4 transformationOperator3D = visitor1.getTransformationOperator3D();


        if (keyItems.size() > 0)
        {
            for (int i = 0; i < listStyle.size(); i++)
            {
                if (keyItems[0] == listStyle[i].keyItem)
                {
                    styleDessin.red = listStyle[i].red;
                    styleDessin.green = listStyle[i].green;
                    styleDessin.blue = listStyle[i].blue;
                    styleDessin.transparence = listStyle[i].transparence;
                    listStyle.erase(listStyle.begin() + i);
                }
            }
        }

        for (int i = 0; i < nameItems.size(); i++)
        {
            std::string NameProfilDef = visitor1.getNameProfildef();
            if (nameItems[i] == "IfcExtrudedAreaSolid")
            {
                if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                {
                    dessinProfilDef(NameProfilDef,entity, VecteurExtrusion, hauteurExtrusion, transform1, visitor1, points1, transformFace, nameItems, i, styleDessin, isMappedItem, transformationOperator3D);
                }
                else
                    extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
            }
            else if (nameItems[i] == "IfcBooleanClippingResult")
            {
                extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
            }
            else if (nameItems[i] == "IfcFacetedBrep" || nameItems[i] == "IfcFaceBasedSurfaceModel" || nameItems[i] == "IfcShellBasedSurfaceModel")
            {
                std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                bool orientation = visitor1.getOrientatationFace();
                createFaceSolid(entity,points1, ListNbArg, orientation, transformFace, styleDessin, isMappedItem, transformationOperator3D);
            }
            else if (nameItems[i] == "IfcBoundingBox")
            {
                createBoundingBox(box, entity, styleDessin);
            }
        }
    }

    for (auto& mappedItems : expressDataSet->getAllIfcMappedItem())
    {
        int key = (int)mappedItems.getKey();
        std::string entity = mappedItems.getType().getName();

        //if (key != 18016) continue;
        //if (key != 4574) continue;

        CreateConstructionPointVisitor visitor1;

        mappedItems.acceptVisitor(&visitor1);
        mappedItems.acceptVisitor(&placementVisitor);

        MappedItem mappedItem;

        mappedItem.nameItemsMap = visitor1.getNameItems();
        mappedItem.keyItemsMap = visitor1.getListKeyItem();
        mappedItem.points1Map = visitor1.getPoints();
        mappedItem.outerCurveNameMap = visitor1.getOuterCurveName();
        mappedItem.ListNbArgMap = visitor1.getNbArgPolyline();
        mappedItem.VecteurExtrusionMap = visitor1.getVectorDirection();
        mappedItem.hauteurExtrusionMap = visitor1.getHauteurExtrusion();
        mappedItem.listPlanMap = visitor1.getPlanPolygonal();
        mappedItem.listLocationPolygonalMap = visitor1.getLocationPolygonal();
        mappedItem.AgreementHalfMap = visitor1.getAgreementHalfBool();
        mappedItem.AgreementPolygonalMap = visitor1.getAgreementPolygonalBool();
        mappedItem.listEntityHalfMap = visitor1.getListEntityHalf();
        mappedItem.listEntityPolygonalMap = visitor1.getListEntityPolygonal();
        mappedItem._compositeCurveMap = visitor1.getCompositeCurveSegment();
        mappedItem.nbPolylineCompositeMap = visitor1.getnbPolylineCompositeCurve();
        mappedItem.boxMap = visitor1.getBox();        
        mappedItem.transform1Map = placementVisitor.getTransformation();
        mappedItem.transformationMap = visitor1.getTransformation();
        mappedItem.transformFaceMap = mappedItem.transform1Map;
        mappedItem.isMappedItemMap = visitor1.getIsMappedItem();
        mappedItem.transformationOperator3DMap = visitor1.getTransformationOperator3D();

        std::vector<MappedItem> listMappedItem;

        listMappedItem.push_back(mappedItem);
    }

    for (auto& buildingElement : expressDataSet->getAllIfcBuildingElement())
    {
        
        int key = (int)buildingElement.getKey();
        std::string entity = buildingElement.getType().getName();

        if (entity != "IfcSlab") continue;
        acutPrintf(_T("    => Element %d\n"), count2);

        CreateConstructionPointVisitor visitor1;

        buildingElement.acceptVisitor(&visitor1);

        std::vector<std::string> nameItems = visitor1.getNameItems();
        std::vector<int> keyItems = visitor1.getListKeyItem();

        std::list<Vec3> points1 = visitor1.getPoints();
        std::string outerCurveName = visitor1.getOuterCurveName();
        std::vector<int> ListNbArg = visitor1.getNbArgPolyline();

        Vec3 VecteurExtrusion = visitor1.getVectorDirection();
        float hauteurExtrusion = visitor1.getHauteurExtrusion();

        std::list<Matrix4> listPlan = visitor1.getPlanPolygonal();

        std::list<Matrix4> listLocationPolygonal = visitor1.getLocationPolygonal();

        std::vector<bool> AgreementHalf = visitor1.getAgreementHalfBool();
        std::vector<bool> AgreementPolygonal = visitor1.getAgreementPolygonalBool();

        std::vector<std::string> listEntityHalf = visitor1.getListEntityHalf();
        std::vector<std::string> listEntityPolygonal = visitor1.getListEntityPolygonal();

        CompositeCurveSegment _compositeCurveSegment = visitor1.getCompositeCurveSegment();
        int nbPolylineComposite = visitor1.getnbPolylineCompositeCurve();

        Box box = visitor1.getBox();

        buildingElement.acceptVisitor(&placementVisitor);
        Matrix4 transform1 = placementVisitor.getTransformation();
        Matrix4 transformation = visitor1.getTransformation();
        Matrix4 transformFace = transform1;

        bool isMappedItem = visitor1.getIsMappedItem();
        Matrix4 transformationOperator3D = visitor1.getTransformationOperator3D();

        if (keyItems.size() > 0)
        {
            for (int i = 0; i < listStyle.size(); i++)
            {
                if (keyItems[0] == listStyle[i].keyItem)
                {
                    styleDessin.red = listStyle[i].red;
                    styleDessin.green = listStyle[i].green;
                    styleDessin.blue = listStyle[i].blue;
                    styleDessin.transparence = listStyle[i].transparence;
                    listStyle.erase(listStyle.begin() + i);
                }
            }
        }
        
        for (int i = 0; i < nameItems.size(); i++)
        {
            if (entity == "IfcBuildingElementProxy")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                if (nameItems[i] == "IfcExtrudedAreaSolid")
                {
                    transform1 *= transformation;

                    if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                    {
                        dessinProfilDef(NameProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, visitor1, points1, transformFace, nameItems, i, styleDessin, isMappedItem, transformationOperator3D);
                    }
                    else
                        extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcBooleanClippingResult")
                {
                    transform1 *= transformation;

                    extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcFacetedBrep" || nameItems[i] == "IfcFaceBasedSurfaceModel" || nameItems[i] == "IfcShellBasedSurfaceModel")
                {
                    transform1 *= transformation;

                    std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                    bool orientation = visitor1.getOrientatationFace();
                    createFaceSolid(entity, points1, ListNbArg, orientation, transformFace, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcBoundingBox")
                {
                    createBoundingBox(box, entity, styleDessin);
                }
                else if (nameItems[i] == "IfcMappedItem")
                {

                }
            }
            else if (entity != "IfcColumn" && entity != "IfcBeam")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                if (nameItems[i] == "IfcExtrudedAreaSolid")
                {
                    transform1 *= transformation;

                    if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                    {
                        dessinProfilDef(NameProfilDef,entity, VecteurExtrusion, hauteurExtrusion, transform1, visitor1, points1, transformFace, nameItems, i, styleDessin, isMappedItem, transformationOperator3D);
                    }
                    else
                    extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcBooleanClippingResult")
                {
                    transform1 *= transformation;

                    extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcFacetedBrep" || nameItems[i] == "IfcFaceBasedSurfaceModel"  || nameItems[i] == "IfcShellBasedSurfaceModel")
                {
                    transform1 *= transformation;

                    std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                    bool orientation = visitor1.getOrientatationFace();
                    createFaceSolid(entity,points1, ListNbArg, orientation, transformFace, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcBoundingBox")
                {
                    createBoundingBox(box, entity, styleDessin);
                }
            }
            else if (entity == "IfcColumn" || entity == "IfcBeam")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                dessinProfilDef(NameProfilDef,entity , VecteurExtrusion, hauteurExtrusion, transform1, visitor1, points1, transformFace, nameItems, i, styleDessin, isMappedItem, transformationOperator3D);

            }
        }
    }

    for (auto& mappedItem : expressDataSet->getAllIfcMappedItem())
    {
        
        int key = (int)mappedItem.getKey();
        std::string entity = mappedItem.getType().getName();


        CreateConstructionPointVisitor visitor1;

        mappedItem.acceptVisitor(&visitor1);

        std::vector<std::string> nameItems = visitor1.getNameItems();
        std::vector<int> keyItems = visitor1.getListKeyItem();

        std::list<Vec3> points1 = visitor1.getPoints();
        std::string outerCurveName = visitor1.getOuterCurveName();
        std::vector<int> ListNbArg = visitor1.getNbArgPolyline();

        Vec3 VecteurExtrusion = visitor1.getVectorDirection();
        float hauteurExtrusion = visitor1.getHauteurExtrusion();

        std::list<Matrix4> listPlan = visitor1.getPlanPolygonal();

        std::list<Matrix4> listLocationPolygonal = visitor1.getLocationPolygonal();

        std::vector<bool> AgreementHalf = visitor1.getAgreementHalfBool();
        std::vector<bool> AgreementPolygonal = visitor1.getAgreementPolygonalBool();

        std::vector<std::string> listEntityHalf = visitor1.getListEntityHalf();
        std::vector<std::string> listEntityPolygonal = visitor1.getListEntityPolygonal();

        CompositeCurveSegment _compositeCurveSegment = visitor1.getCompositeCurveSegment();
        int nbPolylineComposite = visitor1.getnbPolylineCompositeCurve();

        Box box = visitor1.getBox();

        mappedItem.acceptVisitor(&placementVisitor);
        Matrix4 transform1 = placementVisitor.getTransformation();
        Matrix4 transformationProxy = placementVisitor.getTransformationProxy();
        Matrix4 transformation = visitor1.getTransformation();
        Matrix4 transformFace = transform1;

        bool isMappedItem = visitor1.getIsMappedItem();
        Matrix4 transformationOperator3D = visitor1.getTransformationOperator3D();

        transform1 *= transformation;

        //transformationOperator3D *= transformationProxy;

        if (keyItems.size() > 0)
        {
            for (int i = 0; i < listStyle.size(); i++)
            {
                if (keyItems[0] == listStyle[i].keyItem)
                {
                    styleDessin.red = listStyle[i].red;
                    styleDessin.green = listStyle[i].green;
                    styleDessin.blue = listStyle[i].blue;
                    styleDessin.transparence = listStyle[i].transparence;
                    listStyle.erase(listStyle.begin() + i);
                }
            }
        }

        for (int i = 0; i < nameItems.size(); i++)
        {
            if (entity != "IfcColumn" && entity != "IfcBeam")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                if (nameItems[i] == "IfcExtrudedAreaSolid")
                {
                    if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                    {
                        dessinProfilDef(NameProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, visitor1, points1, transformFace, nameItems, i, styleDessin, isMappedItem, transformationOperator3D);
                    }
                    else
                        extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcBooleanClippingResult")
                {
                    extrusion(key, entity, nameItems, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcFacetedBrep" || nameItems[i] == "IfcFaceBasedSurfaceModel" || nameItems[i] == "IfcShellBasedSurfaceModel")
                {
                    std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                    bool orientation = visitor1.getOrientatationFace();
                    createFaceSolid(entity, points1, ListNbArg, orientation, transformFace, styleDessin, isMappedItem, transformationOperator3D);
                }
                else if (nameItems[i] == "IfcBoundingBox")
                {
                    createBoundingBox(box, entity, styleDessin);
                }

            }
            else if (entity == "IfcColumn" || entity == "IfcBeam")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                dessinProfilDef(NameProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, visitor1, points1, transformFace, nameItems, i, styleDessin, isMappedItem, transformationOperator3D);

            }
        }
    }

    acutPrintf(_T("\nFailure : %d\nSuccess : %d\n"), failure_results, success_results);

    //if (shouldWrite)
    //{
    //    // ** Write the file
    //    ifc2x3::SPFWriter writer(expressDataSet);
    //    std::ofstream filestream;
    //    filestream.open(writeFile);

    //    bool status = writer.write(filestream);
    //    filestream.close();
    //}
    listVoid.clear();
    listStyle.clear();
    delete expressDataSet;
}

void dessinProfilDef(std::string NameProfilDef, std::string entity, Vec3 VecteurExtrusion, float hauteurExtrusion, Matrix4 transform1, CreateConstructionPointVisitor visitor1, std::list<Vec3> points1, Matrix4 transformFace, std::vector<std::string> nameItems, int i, Style styleDessin, bool isMappedItem, Matrix4 transformationOperator3D)
{
    if (NameProfilDef == "IfcIShapeProfileDef")
    {
        I_profilDef IprofilDef = visitor1.getIprofilDef();
        if (IprofilDef.nbArg == 5)
        {
            createSolid3dProfilIPE(IprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
        }
    }
    else if (NameProfilDef == "IfcLShapeProfileDef")
    {
        L_profilDef LprofilDef = visitor1.getLprofilDef();
        if (LprofilDef.nbArg == 5)
        {
            createSolid3dProfilL8(LprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
        }
        else
        {
            createSolid3dProfilL9(LprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
        }
    }
    else if (NameProfilDef == "IfcTShapeProfileDef")
    {
        T_profilDef TprofilDef = visitor1.getTprofilDef();
        if (TprofilDef.nbArg == 7)
        {
            createSolid3dProfilT10(TprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
        }
        else
        {
            createSolid3dProfilT12(TprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
        }
    }
    else if (NameProfilDef == "IfcUShapeProfileDef")
    {
        U_profilDef UprofilDef = visitor1.getUprofilDef();
        if (UprofilDef.nbArg == 5)
        {
            createSolid3dProfilUPE(UprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
        }
        else
        {
            createSolid3dProfilUPN(UprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
        }
    }
    else if (NameProfilDef == "IfcCShapeProfileDef")
    {
        C_profilDef CprofilDef = visitor1.getCprofilDef();
        createSolid3dProfilC(CprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
    }
    else if (NameProfilDef == "IfcZShapeProfileDef")
    {
        Z_profilDef ZprofilDef = visitor1.getZprofilDef();
        createSolid3dProfilZ(ZprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
    }
    else if (NameProfilDef == "IfcAsymmetricIShapeProfileDef")
    {
        AsymmetricI_profilDef AsymmetricIprofilDef = visitor1.getAsymmetricIprofilDef();
        createSolid3dProfilAsyI(AsymmetricIprofilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
    }
    else if (NameProfilDef == "IfcCircleHollowProfileDef")
    {
        CircleHollow_profilDef CircleHollowProfilDef = visitor1.getCircleHollowprofilDef();
        createSolid3dProfilCircHollow(CircleHollowProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
    }
    else if (NameProfilDef == "IfcRectangleHollowProfileDef")
    {
        RectangleHollow_profilDef RectangleHollowProfilDef = visitor1.getRectangleHollowprofilDef();
        createSolid3dProfilRectHollow(RectangleHollowProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
    }
    else if (NameProfilDef == "IfcCircleProfileDef")
    {
        Circle_profilDef CircleProfilDef = visitor1.getCircleprofilDef();
        createSolid3dProfilCircle(CircleProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
    }
    else if (NameProfilDef == "IfcRectangleProfileDef")
    {
        Rectangle_profilDef RectangleProfilDef = visitor1.getRectangleprofilDef();
        createSolid3dProfilRectangle(RectangleProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, styleDessin, isMappedItem, transformationOperator3D);
    }
    else if (nameItems[i] == "IfcFacetedBrep")
    {
        //if (key != 20924) continue;
        std::vector<int> ListNbArg = visitor1.getListNbArgFace();
        bool orientation = visitor1.getOrientatationFace();
        createFaceSolid(entity,points1, ListNbArg, orientation, transformFace, styleDessin, isMappedItem, transformationOperator3D);
    }
}

void initApp()
{
    // register a command with the AutoCAD command mechanism
    acedRegCmds->addCommand(_T("IMPORT_COMMANDS"),
        _T("Import"),
        _T("Import"),
        ACRX_CMD_TRANSPARENT,
        test);
}

void unloadApp()
{
    acedRegCmds->removeGroup(_T("IMPORT_COMMANDS"));
}

extern "C" AcRx::AppRetCode
acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
    switch (msg)
    {

    case AcRx::kInitAppMsg:
        acrxDynamicLinker->unlockApplication(pkt);
        acrxRegisterAppMDIAware(pkt);
        initApp();
        break;
    case AcRx::kUnloadAppMsg:
        unloadApp();
        break;
    default:
        break;

    }

    return AcRx::kRetOK;

}
