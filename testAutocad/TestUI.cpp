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
#include <adscodes.h>

#include <iostream>
#include "tests.h"

#include <thread>
#include <map>
#include <functional>
#include <chrono>

#include "DataObject.h"

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

void DoYourThing(std::map<Step::Id, Step::BaseObjectPtr>* elements, std::vector<BaseObject*>& objects)
{
    ComputePlacementVisitor placementVisitor;
    int count = 0;

    auto it = elements->begin();

    while (it != elements->end())
    {
        auto& buildingElement = *(it->second);

        count++;
        int key = it->first;
        std::string entity = buildingElement.getType().getName();

        //acutPrintf(_T("    => Element %d\n"), count);

        CreateConstructionPointVisitor visitor1;
        //acutPrintf(_T("Index : %i\n"), key);
        if (key == 542)
        {
            int PA = 0;
        }

        buildingElement.acceptVisitor(&visitor1);

        std::list<Vec3> points1 = visitor1.getPoints();
        /*for (const auto& point : points1)
        {
            acutPrintf(_T("[ %f, %f, %f ]\n"), point.x(), point.y(), point.z());
        }*/

        std::vector<int> ListNbArg = visitor1.getNbArgPolyline();

        Vec3 VecteurExtrusion = visitor1.getVectorDirection();
        //acutPrintf(_T("Vecteur extrusion : [ %f, %f , %f]\n"), VecteurExtrusion.x(), VecteurExtrusion.y(), VecteurExtrusion.z());

        std::list<Matrix4> listPlan = visitor1.getPlanPolygonal();

        std::list<Matrix4> listLocationPolygonal = visitor1.getLocationPolygonal();

        std::vector<bool> AgreementHalf = visitor1.getAgreementHalfBool();
        std::vector<bool> AgreementPolygonal = visitor1.getAgreementPolygonalBool();

        std::vector<std::string> listEntityHalf = visitor1.getListEntityHalf();
        std::vector<std::string> listEntityPolygonal = visitor1.getListEntityPolygonal();

        buildingElement.acceptVisitor(&placementVisitor);
        Matrix4 transform1 = placementVisitor.getTransformation();
        Matrix4 transformation = visitor1.getTransformation();

        transform1 *= transformation;

        if (entity == "IfcWallStandardCase" || entity == "IfcSlab")
        {
            if (points1.size() > 0 && ListNbArg.size() > 0)
            {
                BaseObject* obj = new DataObject;
                obj->EntityType = entity;
                ((DataObject*)obj)->Key = key;
                ((DataObject*)obj)->Points = points1;
                ((DataObject*)obj)->NbArgs = ListNbArg;
                ((DataObject*)obj)->VecteurExtrusion = VecteurExtrusion;
                ((DataObject*)obj)->Transformation = transform1;
                ((DataObject*)obj)->ListePlan = listPlan;
                ((DataObject*)obj)->ListeLocationPolygonal = listLocationPolygonal;
                ((DataObject*)obj)->AgreementHalf = AgreementHalf;
                ((DataObject*)obj)->AgreementPolygonal = AgreementPolygonal;
                ((DataObject*)obj)->ListEntityHalf = listEntityHalf;
                ((DataObject*)obj)->ListEntityPolygonal = listEntityPolygonal;

                objects.push_back(obj);
            }
        }
        else if (entity == "IfcColumn" || entity == "IfcBeam")
        {
            auto profilDef = visitor1.GetProfilDef();

            if (profilDef != nullptr)
            {
                profilDef->Name = visitor1.getNameProfildef();
                profilDef->EntityType = entity;
                profilDef->Transformation = transform1;
                profilDef->VecteurExtrusion = VecteurExtrusion;

                objects.push_back(profilDef);
            }
        }

        it++;
    }
}

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

    auto start = std::chrono::high_resolution_clock::now();

    int count = 0;
    for (auto& voids : expressDataSet->getAllIfcRelVoidsElement())
    {
        count++;

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
            _objectVoid.radius = ((Circle_profilDef*)visitor1.GetProfilDef())->Radius;
        }
        else if (_objectVoid.NameProfilDef == "IfcRectangleProfileDef")
        {
            _objectVoid.XDim = ((Rectangle_profilDef*)visitor1.GetProfilDef())->XDim;
            _objectVoid.YDim = ((Rectangle_profilDef*)visitor1.GetProfilDef())->YDim;
        }
        _objectVoid.VecteurExtrusion = visitor1.getVectorDirection();
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

    std::vector<std::thread*> threads;
    std::map<std::string, std::vector<BaseObject*>> objects;

    for (auto buildingElement : expressDataSet->getAllIfcBuildingElement().m_refList)
    {
        if (buildingElement->size() > 0)
        {
            std::vector<BaseObject*> vector;
            std::string type = (*buildingElement->begin()).second->type();
            objects.emplace(std::make_pair(type, vector));

            threads.push_back(new std::thread(DoYourThing, buildingElement, std::ref(objects[type])));
        }
    }

    for (auto thread : threads)
    {
        thread->join();
    }

    for (auto& thread : threads)
    {
        delete thread;
    }

    for (auto& type : objects)
    {
        for (auto& el : type.second)
        {
            if (type.first == "IfcWallStandardCase" || type.first == "IfcSlab")
            {
                auto type = (DataObject*)el;

                createSolid3d(type->Key, type->Points, type->NbArgs, type->VecteurExtrusion, type->Transformation, type->ListePlan, type->ListeLocationPolygonal, type->AgreementHalf, type->AgreementPolygonal, type->ListEntityHalf, type->ListEntityPolygonal, listVoid);
            }
            else if (type.first == "IfcColumn" || type.first == "IfcBeam")
            {
                createSolid3dProfil((BaseProfilDef*)el, el->VecteurExtrusion, el->Transformation);
            }

            delete el;
            el = nullptr;
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

    objects.clear();
    delete expressDataSet;

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    acutPrintf(_T("Dur�e: %d\n"), (int)duration.count());
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
