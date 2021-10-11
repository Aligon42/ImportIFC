#include "tchar.h"
#include <aced.h>
#include <rxregsvc.h> 
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <ctime>
#include <thread>

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
#include "InsertionBlock.h"
#include <adscodes.h>

#include <iostream>
#include "Export.h"



void initApp()
{
    // register a command with the AutoCAD command mechanism
    acedRegCmds->addCommand(_T("IMPORT_COMMANDS"),
        _T("ImportIFC"),
        _T("ImportIFC"),
        ACRX_CMD_TRANSPARENT,
        test);
}

void initAppEx()
{
    // register a command with the AutoCAD command mechanism
    acedRegCmds->addCommand(_T("EXPORT_COMMANDS"),
        _T("ExportIFC"),
        _T("ExportIFC"),
        ACRX_CMD_TRANSPARENT,
        ExportIFC);
}

//void initAppInsertionBlock()
//{
//    // register a command with the AutoCAD command mechanism
//    acedRegCmds->addCommand(_T("INSERTION_COMMANDS"),
//        _T("ImportIFC"),
//        _T("ImportIFC"),
//        ACRX_CMD_TRANSPARENT,
//        test);
//}


void unloadApp()
{
    acedRegCmds->removeGroup(_T("IMPORT_COMMANDS"));
}

void unloadAppEx()
{
    acedRegCmds->removeGroup(_T("EXPORT_COMMANDS"));
}


extern "C" AcRx::AppRetCode
acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
    switch (msg)
    {

    case AcRx::kInitAppMsg:
        acrxDynamicLinker->unlockApplication(pkt);
        acrxRegisterAppMDIAware(pkt);
        acrxDynamicLinker->unlockApplication(pkt);
        acrxRegisterAppMDIAware(pkt);
        acedDefun(_T("_insertionBlockSimple"), 1);
        acedDefun(_T("_ajusterBlocAvecContour"), 2);
        acedRegFunc(block, 1);
        acedRegFunc(ajusterBlocAvecContour, 2);
        initApp();
        initAppEx();
        acutPrintf(_T("Chargement des fonctions arx"));
        
        break;
    case AcRx::kUnloadAppMsg:
        acedUndef(_T("_insertionBlockSimple"), 1);
        acedUndef(_T("_ajusterBlocAvecContour"), 2);
        unloadApp();
        unloadAppEx();
        acutPrintf(_T("Déchargement des fonctions arx"));
       
        break;
    default:
        break;

    }

    return AcRx::kRetOK;

}