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

void dessinProfilDef(Object object, const std::string& entity, CreateConstructionPointVisitor& visitor, int index, Style styleDessin);

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
            _objectVoid.radius = (static_cast<Circle_profilDef*>(visitor1.getProfilDef().get()))->Radius;
        }
        else if (_objectVoid.NameProfilDef == "IfcRectangleProfileDef")
        {
            _objectVoid.XDim = (static_cast<Rectangle_profilDef*>(visitor1.getProfilDef().get()))->XDim;
            _objectVoid.YDim = (static_cast<Rectangle_profilDef*>(visitor1.getProfilDef().get()))->YDim;
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
        listStyle.emplace(std::make_pair(style.keyItem, style));
    }

    for (auto& site : expressDataSet->getAllIfcSite())
    {
        int key = (int)site.getKey();
        std::string entity = site.getType().getName();

        CreateConstructionPointVisitor visitor1;
        site.acceptVisitor(&visitor1);

        Object obj = visitor1.GetObjectData();

        Box box = visitor1.getBox();

        site.acceptVisitor(&placementVisitor);
        Matrix4 transformation = placementVisitor.getTransformation();
        obj.Transform = transformation * obj.Transform;

        if (obj.KeyItems.size() > 0)
        {
            if (listStyle.count(obj.KeyItems[0]))
            {
                styleDessin = listStyle[obj.KeyItems[0]];
                listStyle.erase(obj.KeyItems[0]);
            }
        }

        for (int i = 0; i < obj.NameItems.size(); i++)
        {
            std::string NameProfilDef = visitor1.getNameProfildef();
            if (obj.NameItems[i] == "IfcExtrudedAreaSolid")
            {
                if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                {
                    dessinProfilDef(obj, entity, visitor1, i, styleDessin);
                }
                else
                    extrusion(key, entity, obj, listVoid);
            }
            else if (obj.NameItems[i] == "IfcBooleanClippingResult")
            {
                extrusion(key, entity, obj, listVoid);
            }
            else if (obj.NameItems[i] == "IfcFacetedBrep" || obj.NameItems[i] == "IfcFaceBasedSurfaceModel" || obj.NameItems[i] == "IfcShellBasedSurfaceModel")
            {
                std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                bool orientation = visitor1.getOrientatationFace();
                createFaceSolid(entity, obj, styleDessin);
            }
            else if (obj.NameItems[i] == "IfcBoundingBox")
            {
                createBoundingBox(box, entity, styleDessin);
            }
        }
    }

    int count2 = 0;
    /*for (auto& buildingElement : expressDataSet->getAllIfcBuildingElement())
    {
        count2++;
        int key = (int)buildingElement.getKey();
        std::string entity = buildingElement.getType().getName();

        //if (key != 18016) continue;
        //if (key != 4574) continue;
        //if (entity != "IfcSlab") continue;
        if (key != 5440) continue;
        acutPrintf(_T("    => Element %d\n"), count2);

        CreateConstructionPointVisitor visitor1;
        acutPrintf(_T("Index : %i\n"), key);

        buildingElement.acceptVisitor(&visitor1);
        Object obj = visitor1.GetObjectData();

        Box box = visitor1.getBox();

        buildingElement.acceptVisitor(&placementVisitor);
        obj.TransformFace = placementVisitor.getTransformation();
        obj.Transform = obj.TransformFace * obj.Transform;

        //if (mappedItem = true)
        //{
        //    transform1 = visitor1.getTransformationOperator3D();
        //}
        
        for (int i = 0; i < obj.NameItems.size(); i++)
        {
            if (obj.KeyItems.size() > 0)
            {
                if (listStyle.count(obj.KeyItems[0]))
                {
                    styleDessin = listStyle[obj.KeyItems[0]];
                    listStyle.erase(obj.KeyItems[0]);
                }
            }

            if (entity != "IfcColumn" && entity != "IfcBeam")
            {
                if (obj.NameItems[i] == "IfcExtrudedAreaSolid")
                {
                    if (obj.NameProfilDef != "IfcArbitraryClosedProfileDef")
                    {
                        dessinProfilDef(obj, entity, visitor1, i, styleDessin);
                    }
                    else
                        extrusion(key, entity, obj, listVoid);
                }
                else if (obj.NameItems[i] == "IfcBooleanClippingResult")
                {
                    extrusion(key, entity, obj, listVoid);
                }
                else if (obj.NameItems[i] == "IfcFacetedBrep" || obj.NameItems[i] == "IfcFaceBasedSurfaceModel"  || obj.NameItems[i] == "IfcShellBasedSurfaceModel")
                {
                    std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                    bool orientation = visitor1.getOrientatationFace();
                    createFaceSolid(entity, obj, styleDessin);
                }
                else if (obj.NameItems[i] == "IfcBoundingBox")
                {
                    createBoundingBox(box, entity, styleDessin);
                }

            }
            else if (entity == "IfcColumn" || entity == "IfcBeam")
            {
                dessinProfilDef(obj, entity, visitor1, i, styleDessin);
            }
        }
    }*/

    for (auto& mappedItem : expressDataSet->getAllIfcMappedItem())
    {
        count2++;
        int key = (int)mappedItem.getKey();
        std::string entity = mappedItem.getType().getName();

        //if (key != 18016) continue;
        //if (key != 4574) continue;
        //if (entity != "IfcSlab") continue;
        acutPrintf(_T("    => Element %d\n"), count2);

        CreateConstructionPointVisitor visitor1;
        acutPrintf(_T("Index : %i\n"), key);

        mappedItem.acceptVisitor(&visitor1);

        Object obj = visitor1.GetObjectData();

        Box box = visitor1.getBox();

        mappedItem.acceptVisitor(&placementVisitor);
        obj.TransformFace = placementVisitor.getTransformation();
        obj.Transform = obj.TransformFace * obj.Transform;

        /*if (mappedItem = true)
        {
            transform1 = visitor1.getTransformationOperator3D();
        }*/

        if (obj.KeyItems.size() > 0)
        {
            if (listStyle.count(obj.KeyItems[0]))
            {
                styleDessin = listStyle[obj.KeyItems[0]];
                listStyle.erase(obj.KeyItems[0]);
            }
        }

        for (int i = 0; i < obj.NameItems.size(); i++)
        {
            if (entity != "IfcColumn" && entity != "IfcBeam")
            {
                std::string NameProfilDef = visitor1.getNameProfildef();
                if (obj.NameItems[i] == "IfcExtrudedAreaSolid")
                {
                    if (NameProfilDef != "IfcArbitraryClosedProfileDef")
                    {
                        dessinProfilDef(obj, entity, visitor1, i, styleDessin);
                    }
                    else
                        extrusion(key, entity, obj, listVoid);
                }
                else if (obj.NameItems[i] == "IfcBooleanClippingResult")
                {
                    extrusion(key, entity, obj, listVoid);
                }
                else if (obj.NameItems[i] == "IfcFacetedBrep" || obj.NameItems[i] == "IfcFaceBasedSurfaceModel" || obj.NameItems[i] == "IfcShellBasedSurfaceModel")
                {
                    std::vector<int> ListNbArg = visitor1.getListNbArgFace();
                    bool orientation = visitor1.getOrientatationFace();
                    createFaceSolid(entity, obj, styleDessin);
                }
                else if (obj.NameItems[i] == "IfcBoundingBox")
                {
                    createBoundingBox(box, entity, styleDessin);
                }
            }
            else if (entity == "IfcColumn" || entity == "IfcBeam")
            {
                dessinProfilDef(obj, entity, visitor1, i, styleDessin);
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

void dessinProfilDef(Object object, const std::string& entity, CreateConstructionPointVisitor& visitor, int index, Style styleDessin)
{
    if (object.NameProfilDef.find("ProfileDef") != std::string::npos)
    {
        auto profilDef = visitor.getProfilDef();
        profilDef->createSolid3dProfil(styleDessin);
    }
    else if (object.NameItems[index] == "IfcFacetedBrep")
    {
        //if (key != 20924) continue;
        std::vector<int> ListNbArg = visitor.getListNbArgFace();
        bool orientation = visitor.getOrientatationFace();
        createFaceSolid(entity, object, styleDessin);
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
