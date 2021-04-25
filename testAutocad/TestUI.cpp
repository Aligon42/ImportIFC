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

#include "CreateConstructionPointVisitor.h"
#include "ComputePlacementVisitor.h"
#include "MethodeConstructionr.h"
#include <adscodes.h>

#include <iostream>
#include "tests.h"

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

    int count = 0;
    for (auto& wall : expressDataSet->getAllIfcWall())
    {
        count++;

        acutPrintf(_T("    => Wall %d\n"), count);
        CreateConstructionPointVisitor visitor1;

        wall.acceptVisitor(&visitor1);

        std::list<Vec3> points1 = visitor1.getPoints();

        for (const auto& point : points1)
        {
            acutPrintf(_T("[ %f, %f, %f ]\n"),  point.x(), point.y(), point.z());
        }

        Vec3 VecteurExtrusion = visitor1.getVectorDirection();

        acutPrintf(_T("Vecteur extrusion : [ %f, %f , %f]\n"), VecteurExtrusion.x(), VecteurExtrusion.y(), VecteurExtrusion.z());
       
        wall.acceptVisitor(&placementVisitor);
        Matrix4 transform1 = placementVisitor.getTransformation();

        createSolid3d(points1, VecteurExtrusion);
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

    delete expressDataSet;
}

static void build()
{
    Acad::ErrorStatus es;

    ads_name polyName;

    ads_point ptres;

    AcGePoint3dArray ptArr;
    ptArr.setLogicalLength(4);

    ptArr[0].set(0, 0, 0.0);
    ptArr[1].set(10, 0, 0.0);
    ptArr[2].set(10, 10, 0.0);
    ptArr[3].set(0, 10, 0.0);

    AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
        AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
    pNewPline->setColorIndex(3);

    //get the boundary curves of the polyline
    AcDbEntity* pEntity = NULL;
    if (pNewPline == NULL)
    {
        pEntity->close();
        return;
    }
    AcDbVoidPtrArray lines;
    pNewPline->explode(lines);
    pNewPline->close();

    // Create a region from the set of lines.
    AcDbVoidPtrArray regions;
    es = AcDbRegion::createFromCurves(lines, regions);
    if (Acad::eOk != es)
    {
        pNewPline->close();
        acutPrintf(L"\nFailed to create region\n");
        return;
    }
    AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);
    // Extrude the region to create a solid.
    AcDb3dSolid* pSolid = new AcDb3dSolid();
    es = pSolid->extrude(pRegion, 10.0, 0.0);

    for (int i = 0; i < lines.length(); i++)
    {
        delete (AcRxObject*)lines[i];
    }

    for (int ii = 0; ii < regions.length(); ii++)
    {
        delete (AcRxObject*)regions[ii];
    }

    //DeplacementObjet3D(pSolid);

    AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
    if (Acad::eOk == es)
    {
        AcDbDatabase* pDb = curDoc()->database();
        AcDbObjectId modelId;
        modelId = acdbSymUtil()->blockModelSpaceId(pDb);
        AcDbBlockTableRecord* pBlockTableRecord;
        acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
        pBlockTableRecord->appendAcDbEntity(pSolid);
        pBlockTableRecord->close();
        pSolid->close();
    }
    else
    {
        delete pSolid;
        acutPrintf(_T("Je ne fais rien du tout"));
        return;
    }
}

void initApp()
{
    // register a command with the AutoCAD command mechanism
    acedRegCmds->addCommand(_T("TEST_COMMANDS"),
        _T("Test"),
        _T("Test"),
        ACRX_CMD_TRANSPARENT,
        test);

    acedRegCmds->addCommand(_T("TESTBUILD_COMMANDS"),
        _T("build"),
        _T("build"),
        ACRX_CMD_TRANSPARENT,
        build);
}
void unloadApp()
{
    acedRegCmds->removeGroup(_T("TEST_COMMANDS"));
    acedRegCmds->removeGroup(_T("TESTBUILD_COMMANDS"));
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
