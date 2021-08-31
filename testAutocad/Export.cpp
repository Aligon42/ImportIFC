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

#include <ifc2x3/IfcUnit.h>

#include <iostream>
#include <stdio.h>
#include <vector>

#ifdef WIN32
#include <time.h>
#endif

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

void initOwnerHistory(ifc2x3::IfcOwnerHistory* theOwnerHistory, std::string userID, std::string userFN, std::string userLN,std::string orgID, std::string orgName,std::string appVersion, std::string appID, std::string appName)
{
    theOwnerHistory->getOwningUser()->getThePerson()->setId(userID);
    theOwnerHistory->getOwningUser()->getThePerson()->setGivenName(userFN);
    theOwnerHistory->getOwningUser()->getThePerson()->setFamilyName(userLN);

    theOwnerHistory->getOwningUser()->getTheOrganization()->setId(orgID);
    theOwnerHistory->getOwningUser()->getTheOrganization()->setName(orgName);

    theOwnerHistory->getOwningApplication()->setVersion(appVersion);
    theOwnerHistory->getOwningApplication()->setApplicationIdentifier(appID);
    theOwnerHistory->getOwningApplication()->setApplicationFullName(appName);

#ifdef WIN32
    time_t ltime;
    time(&ltime);
    theOwnerHistory->setCreationDate((ifc2x3::IfcTimeStamp)ltime);
#endif
}

void ExportIFC()
{
    // ** First build an ExpressDataSet
    Step::RefPtr <ifc2x3::ExpressDataSet> expressDataSet = new ifc2x3::ExpressDataSet();

    // ** Geometric representation stuff
    std::vector<double> points, points2;
    std::vector<double> position, placement;

    //build the header
    Step::SPFHeader header;
    header.getFileDescription().description.push_back("Test pour l'exportIFC PROCAL");
    header.getFileName().name = "TEST";
    header.getFileName().author.push_back("Antoine Cacheux");
    header.getFileName().organization.push_back("PROCAL");
    header.getFileName().originatingSystem = "unknown";
    header.getFileName().authorization = "Authorisation unknown";
    time_t t = time(0);
    char* dt = ctime(&t);
    header.getFileName().timeStamp = dt;
    header.getFileName().preprocessorVersion = "1.0.0";
    header.getFileSchema().schemaIdentifiers.push_back("IFC2X3");

    expressDataSet->setHeader(header);

    //build the project
    Step::RefPtr <ifc2x3::IfcProject> project = expressDataSet->createIfcProject();
    project->setDescription("Projet de test");

    //build the project owneer history
    Step::RefPtr< ifc2x3::IfcOwnerHistory > ownerHistory = expressDataSet->createIfcOwnerHistory();
    // - build the owning user
    Step::RefPtr<ifc2x3::IfcPersonAndOrganization> personOrg = expressDataSet->createIfcPersonAndOrganization();
    Step::RefPtr<ifc2x3::IfcPerson> person = expressDataSet->createIfcPerson();
    personOrg->setThePerson(person);
    Step::RefPtr<ifc2x3::IfcOrganization> organization = expressDataSet->createIfcOrganization();
    personOrg->setTheOrganization(organization);
    ownerHistory->setOwningUser(personOrg);
    // - build the owning application
    Step::RefPtr<ifc2x3::IfcApplication> theApplication = expressDataSet->createIfcApplication();
    theApplication->setApplicationDeveloper(organization);
    ownerHistory->setOwningApplication(theApplication);
    // Init the owner history
    initOwnerHistory(ownerHistory.get(), "AC", "Antoine", "CACHEUX", "PROCAL_id",
        "PROCAL", "1.0.0", "ENVELOP3D_id", "ENVELOP3D");
    project->setOwnerHistory(ownerHistory);

    // set the units
    Step::RefPtr<ifc2x3::IfcUnitAssignment> projectUnits = expressDataSet->createIfcUnitAssignment();
    Step::RefPtr< ifc2x3::IfcUnit > unitSelect = new ifc2x3::IfcUnit();
    Step::RefPtr< ifc2x3::IfcSIUnit > unit = expressDataSet->createIfcSIUnit();
    unit->setUnitType(ifc2x3::IfcUnitEnum_LENGTHUNIT);
    unit->setName(ifc2x3::IfcSIUnitName_METRE);
    unitSelect->setIfcNamedUnit(unit.get());
    projectUnits->getUnits().insert(unitSelect);
    project->setUnitsInContext(projectUnits);


    // ** Write the file        
    ifc2x3::SPFWriter writer(expressDataSet.get());
    std::ofstream filestream("C:\\Users\\AntoineCACHEUX\\source\\repos\\ImportIFC_IFCCADPRO\\TestExport.txt", std::ofstream::out);

    bool status = writer.write(filestream);
    filestream.close();

    acutPrintf(_T("EXPORT DONE"));

}

