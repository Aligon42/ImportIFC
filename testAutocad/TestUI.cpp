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
        listStyle.insert(std::pair<int, Style>(style.keyItem, style));
    }

    for (auto& site : expressDataSet->getAllIfcSite())
    {
        
        int key = (int)site.getKey();
        if (key != 1) continue;
        std::string entity = site.getType().getName();
        CreateConstructionPointVisitor visitor1;
        site.acceptVisitor(&visitor1);

        std::vector<std::string> nameItems = visitor1.getNameItems();
        std::vector<int> keyItems = visitor1.getListKeyItem();
        std::list<Vec3> points1 = visitor1.getPoints();
        std::string outerCurveName = visitor1.getOuterCurveName();
        std::vector<int> ListNbArg = visitor1.getNbArgPolyline();
        Vec3 VecteurExtrusion = visitor1.getVectorDirection();
        std::vector<Vec3> VecteurExtrusionBool = visitor1.getVectorDirectionBool();
        if (VecteurExtrusionBool.size() > 0)
        {
            VecteurExtrusionBool.erase(VecteurExtrusionBool.begin());
        }
        float hauteurExtrusion = visitor1.getHauteurExtrusion();
        std::vector<float> hauteurExtrusionBool = visitor1.getHauteurExtrusionBool();
        if (hauteurExtrusionBool.size() > 0)
        {
            hauteurExtrusionBool.erase(hauteurExtrusionBool.begin());
        }
        std::list<Matrix4> listPlan = visitor1.getPlanPolygonal();
        std::list<Matrix4> listLocationPolygonal = visitor1.getLocationPolygonal();
        std::vector<Step::Boolean> AgreementHalf = visitor1.getAgreementHalfBool();
        std::vector<Step::Boolean> AgreementPolygonal = visitor1.getAgreementPolygonalBool();
        std::vector<std::string> listEntityHalf = visitor1.getListEntityHalf();
        std::vector<std::string> listEntityPolygonal = visitor1.getListEntityPolygonal();
        CompositeCurveSegment _compositeCurveSegment = visitor1.getCompositeCurveSegment();
        int nbPolylineComposite = visitor1.getnbPolylineCompositeCurve();
        int nbCompositeCurve = visitor1.getNbCompositeCurve();
        int scale = visitor1.getScale();

        Box box = visitor1.getBox();

        std::vector<Rectangle_profilDef> RectangleProfilDefBool = visitor1.getRectangleprofilDefBool();

        site.acceptVisitor(&placementVisitor);
        Matrix4 transform1 = placementVisitor.getTransformation();
        Matrix4 transformation = visitor1.getTransformation();
        Matrix4 transformation2D = visitor1.getTransformation2D();
        std::vector<Matrix4> transformationBoolExtrud = visitor1.getTransformationBoolExtrud();
        if (transformationBoolExtrud.size() > 1)
        {
            transformationBoolExtrud.erase(transformationBoolExtrud.begin());
        }
        Matrix4 transformFace = transform1;

        bool isMappedItemMethode = visitor1.getIsMappedItemMethode();
        bool isMappedItem = visitor1.getIsMappedItem();
        Matrix4 transformationOperator3D = visitor1.getTransformationOperator3D();

        std::vector<Style> vectorStyle;
        if (keyItems.size() > 0)
        {
            for (int i = 0; i < keyItems.size(); i++)
            {
                vectorStyle.push_back(listStyle[keyItems[i]]);
            }
        }

        for (int i = 0; i < nameItems.size(); i++)
        {
            std::string NameProfilDef = visitor1.getNameProfildef();
            std::vector<std::string> NameProfilDefBool = visitor1.getNameProfildefBool();
            if (nameItems[i] == "IfcExtrudedAreaSolid")
            {
                

                if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                {
                    dessinProfilDef(NameProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, visitor1, points1, transformFace, nameItems, keyItems[i], keyItems, outerCurveName, ListNbArg, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, i, vectorStyle, isMappedItemMethode, transformationOperator3D, scale, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                }
                else
                {
                    transform1 *= transformation;
                    extrusion(key, entity, nameItems, keyItems[i], outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                }
                    
            }
            else if (nameItems[i] == "IfcBooleanClippingResult")
            {
                //transform1 *= transformation;

                extrusion(key, entity, nameItems, keyItems[i], outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
            }
            else if (nameItems[i] == "IfcFacetedBrep" || nameItems[i] == "IfcFaceBasedSurfaceModel" || nameItems[i] == "IfcShellBasedSurfaceModel")
            {
                std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                bool orientation = visitor1.getOrientatationFace();
                createFaceSolid(entity, keyItems, points1, ListNbArg, orientation, transformFace, transform1, transformation, vectorStyle, isMappedItem, transformationOperator3D, scale);
            }
            else if (nameItems[i] == "IfcBoundingBox")
            {
                createBoundingBox(box, entity, keyItems[i], vectorStyle);
            }
        }
    }

    std::map<int, MappedItem> dicoMappedItem;

    for (auto& mappedItems : expressDataSet->getAllIfcMappedItem())
    {
        int key = (int)mappedItems.getKey();
        std::string entity = mappedItems.getType().getName();

        //if (key != 1) continue;
        //if (key != 5454) continue;

        CreateConstructionPointVisitor visitor1;

        mappedItems.acceptVisitor(&visitor1);
        mappedItems.acceptVisitor(&placementVisitor);

        MappedItem mappedItem;

        mappedItem.nameItemsMap = visitor1.getNameItemsMap();
        mappedItem.NameProfilDefMap = visitor1.getNameProfildef();
        mappedItem.keyItemsMap = visitor1.getListKeyItem();
        mappedItem.points1Map = visitor1.getPoints();
        mappedItem.outerCurveNameMap = visitor1.getOuterCurveName();
        mappedItem.ListNbArgMap = visitor1.getNbArgPolyline();
        mappedItem.listNbArgFaceMap = visitor1.getListNbArgFace();
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
        mappedItem.transformation2DMap = visitor1.getTransformation2D();
        mappedItem.transformFaceMap = mappedItem.transform1Map;
        mappedItem.isMappedItemMap = visitor1.getIsMappedItem();
        mappedItem.transformationOperator3DMap = visitor1.getTransformationOperator3D();
        mappedItem.scale = visitor1.getScale();

        dicoMappedItem.insert(std::pair<int, MappedItem>(key, mappedItem));

    }

    for (auto& buildingElement : expressDataSet->getAllIfcElement())
    {
        
        int key = (int)buildingElement.getKey();
        std::string entity = buildingElement.getType().getName();
        //if (key != 108852 /*&& key != 110815 && key != 108907 && key != 110867*/) continue;
        //if (key != 8513) continue;
        //if (key != 59641) continue;
        //if (key != 1309) continue;
        if (key != 77564) continue;

        CreateConstructionPointVisitor visitor1;

        buildingElement.acceptVisitor(&visitor1);

        std::vector<std::string> nameItems = visitor1.getNameItems();
        std::vector<int> keyItems = visitor1.getListKeyItem();

        std::list<Vec3> points1 = visitor1.getPoints();
        std::string outerCurveName = visitor1.getOuterCurveName();
        std::vector<int> ListNbArg = visitor1.getNbArgPolyline();
        std::vector<int> listNbArgFace = visitor1.getListNbArgFace();

        Vec3 VecteurExtrusion = visitor1.getVectorDirection();
        std::vector<Vec3> VecteurExtrusionBool = visitor1.getVectorDirectionBool();
        if (VecteurExtrusionBool.size() > 1)
        {
            VecteurExtrusionBool.erase(VecteurExtrusionBool.begin());
        }
        float hauteurExtrusion = visitor1.getHauteurExtrusion();
        std::vector<float> hauteurExtrusionBool = visitor1.getHauteurExtrusionBool();
        if (hauteurExtrusionBool.size() > 1)
        {
            hauteurExtrusionBool.erase(hauteurExtrusionBool.begin());
        }
        std::list<Matrix4> listPlan = visitor1.getPlanPolygonal();

        std::list<Matrix4> listLocationPolygonal = visitor1.getLocationPolygonal();

        std::vector<Step::Boolean> AgreementHalf = visitor1.getAgreementHalfBool();
        std::vector<Step::Boolean> AgreementPolygonal = visitor1.getAgreementPolygonalBool();

        std::vector<std::string> listEntityHalf = visitor1.getListEntityHalf();
        std::vector<std::string> listEntityPolygonal = visitor1.getListEntityPolygonal();

        CompositeCurveSegment _compositeCurveSegment = visitor1.getCompositeCurveSegment();
        int nbPolylineComposite = visitor1.getnbPolylineCompositeCurve();
        int nbCompositeCurve = visitor1.getNbCompositeCurve();

        std::vector<int> keyMappedItem = visitor1.getKeyMappedItem();       
        std::vector<int> keyShapeMap = visitor1.getkeyShapeMap();  
        double scale = visitor1.getScale();

        Box box = visitor1.getBox();

        std::vector<Rectangle_profilDef> RectangleProfilDefBool = visitor1.getRectangleprofilDefBool();

        buildingElement.acceptVisitor(&placementVisitor);
        Matrix4 transform1 = placementVisitor.getTransformation();
        Matrix4 transformation = visitor1.getTransformation();
        Matrix4 transformation2D = visitor1.getTransformation2D();
        std::vector<Matrix4> transformationBoolExtrud = visitor1.getTransformationBoolExtrud();
        if (transformationBoolExtrud.size() > 1)
        {
            transformationBoolExtrud.erase(transformationBoolExtrud.begin());
        }
        Matrix4 transformFace = transform1;

        //bool isMappedItem = visitor1.getIsMappedItem();
        bool isMappedItemMethode = visitor1.getIsMappedItemMethode();
        Matrix4 transformationOperator3D = visitor1.getTransformationOperator3D();

        std::vector<Style> vectorStyle;
        int sizeItem = keyItems.size();
        if (isMappedItemMethode)
        {
            sizeItem = sizeItem / 2;
        }
        if (sizeItem > 0)
        {
            for (int i = 0; i < keyItems.size(); i++)
            {
                if (isMappedItemMethode)
                {
                    if (keyShapeMap.size() < i)
                    {
                        break;
                    }
                    Style pushStyle = listStyle[keyShapeMap[i]];
                    vectorStyle.push_back(pushStyle);
                }
                else
                {
                    vectorStyle.push_back(listStyle[keyItems[i]]);
                }
                
            }
        }
        
        for (int i = 0; i < nameItems.size(); i++)
        {
            if (entity != "IfcColumn" && entity != "IfcBeam")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                std::vector<std::string> NameProfilDefBool = visitor1.getNameProfildefBool();
                if (nameItems[i] == "IfcExtrudedAreaSolid" && !isMappedItemMethode)
                {
                    if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                    {
                        dessinProfilDef(NameProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, visitor1, points1, transformFace, nameItems, keyItems[i], keyItems, outerCurveName, ListNbArg, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, i, vectorStyle, isMappedItemMethode, transformationOperator3D, scale, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                    }
                    else
                    {
                        transform1 *= transformation;
                        extrusion(key, entity, nameItems, keyItems[i], outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItemMethode, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                    }
                    
                }
                else if (nameItems[i] == "IfcBooleanClippingResult" || nameItems[i] == "IfcBooleanResult")
                {
                    //transform1 *= transformation;
                    if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                    {
                        dessinProfilDef(NameProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, visitor1, points1, transformFace, nameItems, keyItems[i], keyItems, outerCurveName, ListNbArg, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, i, vectorStyle, isMappedItemMethode, transformationOperator3D, scale, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                    }
                    else
                    extrusion(key, entity, nameItems, keyItems[i], outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItemMethode, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                }
                else if (nameItems[i] == "IfcFacetedBrep" || nameItems[i] == "IfcFaceBasedSurfaceModel"  || nameItems[i] == "IfcShellBasedSurfaceModel" && !isMappedItemMethode)
                {
                    std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                    bool orientation = visitor1.getOrientatationFace();
                    createFaceSolid(entity, keyItems, points1, listNbArgFace, orientation, transformFace, transform1, transformation, vectorStyle, isMappedItemMethode, transformationOperator3D, scale);
                }
                else if (nameItems[i] == "IfcBoundingBox" && !isMappedItemMethode)
                {
                    createBoundingBox(box, entity, keyItems[i], vectorStyle);
                }
                else if (nameItems[i] == "IfcMappedItem" && isMappedItemMethode)
                {
                    //isMappedItem = true;
                    MappedItem map;

                    for (int j = 0; j < keyMappedItem.size(); j++)
                    {
                        map = dicoMappedItem[keyMappedItem[j]];

                        if (map.nameItemsMap[0] == "IfcExtrudedAreaSolid")
                        {
                            map.transform1Map *= transform1;
                            map.transform1Map *= transformation;

                            if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                            {
                                dessinProfilDef(map.NameProfilDefMap, entity, map.VecteurExtrusionMap, map.hauteurExtrusionMap, transform1, transformation, transformation2D, visitor1, map.points1Map, transformFace, map.nameItemsMap, map.keyItemsMap[j], map.keyItemsMap, outerCurveName, ListNbArg, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, i, vectorStyle, map.isMappedItemMap, map.transformationOperator3DMap, scale, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                            }
                            else
                                extrusion(key, entity, map.nameItemsMap, map.keyItemsMap[j], map.outerCurveNameMap, map.points1Map, map.ListNbArgMap, map.VecteurExtrusionMap, map.hauteurExtrusionMap, map.transform1Map, map.listPlanMap, map.listLocationPolygonalMap, map.AgreementHalfMap, map.AgreementPolygonalMap, map.listEntityHalfMap, map.listEntityPolygonalMap, listVoid, map._compositeCurveMap, map.nbPolylineCompositeMap, nbCompositeCurve, vectorStyle, isMappedItemMethode, map.transformationOperator3DMap, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                        }
                        else if (map.nameItemsMap[0] == "IfcBooleanClippingResult")
                        {
                            //transform1 *= transformation;

                            extrusion(key, entity, map.nameItemsMap, map.keyItemsMap[j], map.outerCurveNameMap, map.points1Map, map.ListNbArgMap, map.VecteurExtrusionMap, map.hauteurExtrusionMap, map.transform1Map, map.transformationMap, map.listPlanMap, map.listLocationPolygonalMap, map.AgreementHalfMap, map.AgreementPolygonalMap, map.listEntityHalfMap, map.listEntityPolygonalMap, listVoid, map._compositeCurveMap, map.nbPolylineCompositeMap, nbCompositeCurve, vectorStyle, isMappedItemMethode, map.transformationOperator3DMap, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                        }
                        else if (map.nameItemsMap[0] == "IfcFacetedBrep" || map.nameItemsMap[0] == "IfcFaceBasedSurfaceModel" || map.nameItemsMap[0] == "IfcShellBasedSurfaceModel")
                        {
                            
                            std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                            bool orientation = visitor1.getOrientatationFace();
                            createFaceSolid(entity, map.keyItemsMap, map.points1Map, map.listNbArgFaceMap, orientation, map.transformFaceMap, transform1, transformation, vectorStyle, isMappedItemMethode, map.transformationOperator3DMap, map.scale);
                        }
                        else if (map.nameItemsMap[0] == "IfcBoundingBox")
                        {
                            createBoundingBox(map.boxMap, entity, map.keyItemsMap[j], vectorStyle);
                        }
                    }
                }
            }
            else if (entity == "IfcColumn" || entity == "IfcBeam")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                std::vector<std::string> NameProfilDefBool = visitor1.getNameProfildefBool();
                

                if (nameItems[i] == "IfcMappedItem" && isMappedItemMethode)
                {
                    MappedItem map;
                    for (int j = 0; j < keyMappedItem.size(); j++)
                    {
                        map = dicoMappedItem[keyMappedItem[j]];

                        dessinProfilDef(map.NameProfilDefMap, entity, map.VecteurExtrusionMap, map.hauteurExtrusionMap, transform1, transformation, transformation2D, visitor1, map.points1Map, transformFace, map.nameItemsMap, map.keyItemsMap[j], map.keyItemsMap, outerCurveName, ListNbArg, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, i, vectorStyle, map.isMappedItemMap, map.transformationOperator3DMap, scale, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                    }
                }
                else 
                {
                    dessinProfilDef(NameProfilDef, entity, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, visitor1, points1, transformFace, nameItems, keyItems[i], keyItems, outerCurveName, ListNbArg, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, i, vectorStyle, isMappedItemMethode, transformationOperator3D, scale, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
                }

            }
            if (isMappedItemMethode) break;
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

void dessinProfilDef(std::string NameProfilDef, std::string entity, Vec3 VecteurExtrusion, float hauteurExtrusion, Matrix4 transform1, Matrix4 transformation, Matrix4 transformation2D, CreateConstructionPointVisitor visitor1, std::list<Vec3> points1, Matrix4 transformFace, std::vector<std::string> nameItems, int keyItem, std::vector<int> keyItems, std::string outerCurveName, std::vector<int> ListNbArg, std::list<Matrix4> listPlan, std::list<Matrix4> listLocationPolygonal, std::vector<Step::Boolean> AgreementHalf, std::vector<Step::Boolean> AgreementPolygonal, std::vector<std::string> listEntityHalf, std::vector<std::string> listEntityPolygonal, std::vector<ObjectVoid> listVoid, CompositeCurveSegment _compositeCurveSegment, int nbPolylineComposite, int nbCompositeCurve, int i, std::vector<Style> vectorStyle, bool isMappedItem, Matrix4 transformationOperator3D, double scale, std::vector<Vec3> VecteurExtrusionBool, std::vector<float> hauteurExtrusionBool, std::vector<Matrix4> transformationBoolExtrud, std::vector<std::string> NameProfilDefBool, std::vector<Rectangle_profilDef> RectangleProfilDefBool)
{
   
    if (NameProfilDef == "IfcIShapeProfileDef")
    {
        I_profilDef IprofilDef = visitor1.getIprofilDef();
        if (IprofilDef.nbArg == 5)
        {
            createSolid3dProfilIPE(IprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
        }
    }
    else if (NameProfilDef == "IfcLShapeProfileDef")
    {
        //transform1 *= transformation;
        L_profilDef LprofilDef = visitor1.getLprofilDef();
        if (LprofilDef.nbArg == 5)
        {
            createSolid3dProfilL8(LprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
        }
        else
        {
            createSolid3dProfilL9(LprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
        }
    }
    else if (NameProfilDef == "IfcTShapeProfileDef")
    {
        //transform1 *= transformation;
        T_profilDef TprofilDef = visitor1.getTprofilDef();
        if (TprofilDef.nbArg == 7)
        {
            createSolid3dProfilT10(TprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
        }
        else
        {
            createSolid3dProfilT12(TprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
        }
    }
    else if (NameProfilDef == "IfcUShapeProfileDef")
    {
        //transform1 *= transformation;
        U_profilDef UprofilDef = visitor1.getUprofilDef();
        if (UprofilDef.nbArg == 5)
        {
            createSolid3dProfilUPE(UprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
        }
        else
        {
            createSolid3dProfilUPN(UprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
        }
    }
    else if (NameProfilDef == "IfcCShapeProfileDef")
    {
        //transform1 *= transformation;
        C_profilDef CprofilDef = visitor1.getCprofilDef();
        createSolid3dProfilC(CprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
    }
    else if (NameProfilDef == "IfcZShapeProfileDef")
    {
        //transform1 *= transformation;
        Z_profilDef ZprofilDef = visitor1.getZprofilDef();
        createSolid3dProfilZ(ZprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
    }
    else if (NameProfilDef == "IfcAsymmetricIShapeProfileDef")
    {
        //transform1 *= transformation;
        AsymmetricI_profilDef AsymmetricIprofilDef = visitor1.getAsymmetricIprofilDef();
        createSolid3dProfilAsyI(AsymmetricIprofilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
    }
    else if (NameProfilDef == "IfcCircleHollowProfileDef")
    {
        //transform1 *= transformation;
        CircleHollow_profilDef CircleHollowProfilDef = visitor1.getCircleHollowprofilDef();
        createSolid3dProfilCircHollow(CircleHollowProfilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
    }
    else if (NameProfilDef == "IfcRectangleHollowProfileDef")
    {
        //transform1 *= transformation;
        RectangleHollow_profilDef RectangleHollowProfilDef = visitor1.getRectangleHollowprofilDef();
        createSolid3dProfilRectHollow(RectangleHollowProfilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
    }
    else if (NameProfilDef == "IfcCircleProfileDef")
    {
        //transform1 *= transformation;
        Circle_profilDef CircleProfilDef = visitor1.getCircleprofilDef();
        createSolid3dProfilCircle(CircleProfilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
    }
    else if (NameProfilDef == "IfcRectangleProfileDef")
    {
        //transform1 *= transformation;
        Rectangle_profilDef RectangleProfilDef = visitor1.getRectangleprofilDef();
        if (RectangleProfilDefBool.size() > 1)
        {
            RectangleProfilDef = RectangleProfilDefBool.at(0);
            RectangleProfilDefBool.erase(RectangleProfilDefBool.begin());
        }
        createSolid3dProfilRectangle(RectangleProfilDef, entity, keyItem, outerCurveName, points1, ListNbArg, VecteurExtrusion, hauteurExtrusion, transform1, transformation, transformation2D, listPlan, listLocationPolygonal, AgreementHalf, AgreementPolygonal, listEntityHalf, listEntityPolygonal, listVoid, _compositeCurveSegment, nbPolylineComposite, nbCompositeCurve, vectorStyle, isMappedItem, transformationOperator3D, VecteurExtrusionBool, hauteurExtrusionBool, transformationBoolExtrud, NameProfilDefBool, RectangleProfilDefBool);
    }
    else if (nameItems[i] == "IfcFacetedBrep")
    {
        transform1 *= transformation;
        std::vector<int> ListNbArg = visitor1.getListNbArgFace();
        bool orientation = visitor1.getOrientatationFace();
        createFaceSolid(entity, keyItems, points1, ListNbArg, orientation, transformFace, transform1, transformation, vectorStyle, isMappedItem, transformationOperator3D, scale);
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
