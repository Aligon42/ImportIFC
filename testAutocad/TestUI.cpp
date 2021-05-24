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
#include "Core.h"

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

void ExploreElement(std::map<Step::Id, Step::BaseObjectPtr>* elements, std::vector<Ref<BaseObject>>& objects)
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

        CreateConstructionPointVisitor visitor1;

        //if (key != 17109) continue;

        buildingElement.acceptVisitor(&visitor1);

        auto obj = visitor1.getObjectToConstructs();
        obj->Key = it->first;
        obj->Type = entity;

        buildingElement.acceptVisitor(&placementVisitor);

        obj->Transform = placementVisitor.getTransformation() * obj->Transform;

        if (entity == "IfcWallStandardCase" || entity == "IfcSlab")
        {
            if (obj->ElementsToConstruct.size() > 0)
            {
                objects.push_back(obj);
            }
        }
        else if (entity == "IfcColumn" || entity == "IfcBeam")
        {
            auto profilDef = visitor1.GetProfilDef();

            if (profilDef != nullptr)
            {
                profilDef->Key = it->first;
                profilDef->Type = entity;
                profilDef->Transform = obj->Transform;
                profilDef->VecteurExtrusion = obj->VecteurExtrusion;

                objects.push_back(profilDef);
            }
        }

        it++;
    }
}

void LoadIfc()
{
    try
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

        auto start = std::chrono::high_resolution_clock::now();

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
        MethodeConstruction methodeConstruction;
        std::vector<std::thread> threads;

        threads.push_back(std::thread([&]()
        {
            int count = 0;
            for (auto& voids : expressDataSet->getAllIfcRelVoidsElement())
            {
                count++;

                CreateConstructionPointVisitor visitor1;
                int key = (int)voids.getKey();

                voids.acceptVisitor(&visitor1);

                ObjectVoid objectVoid = visitor1.getObjectVoid();
                voids.acceptVisitor(&placementVisitor);
                objectVoid.Transform = placementVisitor.getTransformation() * objectVoid.Transform;
                methodeConstruction.AddObjectVoid(objectVoid);
            }
        }));

        std::map<std::string, std::vector<Ref<BaseObject>>> objects;

        for (auto buildingElement : expressDataSet->getAllIfcBuildingElement().m_refList)
        {
            if (buildingElement->size() > 0)
            {
                std::vector<Ref<BaseObject>> vector;
                std::string type = (*buildingElement->begin()).second->type();

                if (type != "IfcWallStandardCase" && type != "IfcSlab" && type != "IfcColumn" && type != "IfcBeam") continue;

                objects.emplace(std::make_pair(type, vector));

                threads.push_back(std::thread(ExploreElement, buildingElement, std::ref(objects[type])));
            }
        }

        for (auto& thread : threads)
        {
            thread.join();
        }

        for (auto& type : objects)
        {
            for (auto& el : type.second)
            {
                if (type.first == "IfcWallStandardCase" || type.first == "IfcSlab")
                {
                    methodeConstruction.createSolid3d(std::static_pointer_cast<ObjectToConstruct>(el));
                }
                else if (type.first == "IfcColumn" || type.first == "IfcBeam")
                {
                    methodeConstruction.createSolid3dProfil(std::static_pointer_cast<BaseProfilDef>(el));
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

        objects.clear();
        delete expressDataSet;

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        acutPrintf(_T("Durée: %d\n"), (int)duration.count());
    }
    catch (const std::exception& ex)
    {
        acutPrintf(_T("%s"), ex.what());
    }
}

void initApp()
{
    // register a command with the AutoCAD command mechanism
    acedRegCmds->addCommand(_T("IMPORT_COMMANDS"),
        _T("Import"),
        _T("Import"),
        ACRX_CMD_TRANSPARENT,
        LoadIfc);
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
