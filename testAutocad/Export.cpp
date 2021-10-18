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
#include "CreateGeometricRepresentationVisitor.h"
#include "UUIDGenerator.h"
#include "Export.h"

#define DISTANCE_TOLERANCE 0.001

namespace Utils
{
    double RadianToDegree(const double rad)
    {
        return rad * (180.0 / PI);
    }

    double DegreeToRadian(const double deg)
    {
        return deg * (PI / 180.0);
    }
}

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

void initRootProperties(ifc2x3::IfcRoot* entity, std::string name)
{
    entity->setGlobalId(UUIDGenerator::generateIfcGloballyUniqueId());
    entity->setName(name);
}

ifc2x3::IfcRelAggregates* linkByAggregate(
    ifc2x3::IfcObjectDefinition* relating,
    ifc2x3::IfcObjectDefinition* related)
{
    static unsigned int count = 0;

    char buffer[128];
#ifdef WIN32
    sprintf_s(buffer, "%d", count);
#else
    sprintf(buffer, "%d", count);
#endif

    Step::RefPtr< ifc2x3::ExpressDataSet > mDataSet = (ifc2x3::ExpressDataSet*)relating->getExpressDataSet();
    ifc2x3::IfcRelAggregates* agg = mDataSet->createIfcRelAggregates().get();
    initRootProperties(agg, std::string("Aggregate") + buffer);

    agg->setRelatingObject(relating);
    agg->getRelatedObjects().insert(related);
    return agg;
}

ifc2x3::IfcRelContainedInSpatialStructure* linkByContainedInSpatial(
    ifc2x3::IfcSpatialStructureElement* relating,
    ifc2x3::IfcProduct* related)
{
    static unsigned int count = 0;
    char buffer[128];
    Step::RefPtr< ifc2x3::ExpressDataSet > mDataSet = (ifc2x3::ExpressDataSet*)relating->getExpressDataSet();
    ifc2x3::IfcRelContainedInSpatialStructure* contained =
        mDataSet->createIfcRelContainedInSpatialStructure().get();
#ifdef WIN32
    sprintf_s(buffer, "%d", count);
#else
    sprintf(buffer, "%d", count);
#endif
    initRootProperties(contained, std::string("ContainedInSpatial") + buffer);

    contained->setRelatingStructure(relating);
    contained->getRelatedElements().insert(related);
    return contained;
}

ifc2x3::IfcRelConnectsPathElements* linkByConnectsPathElements(
    ifc2x3::IfcElement* relating, ifc2x3::IfcConnectionTypeEnum relatingConnectionType,
    ifc2x3::IfcElement* related, ifc2x3::IfcConnectionTypeEnum relatedConnectionType)
{
    static unsigned int count = 0;
    char buffer[128];
    Step::RefPtr< ifc2x3::ExpressDataSet > mDataSet = (ifc2x3::ExpressDataSet*)relating->getExpressDataSet();
    ifc2x3::IfcRelConnectsPathElements* connects =
        mDataSet->createIfcRelConnectsPathElements().get();
#ifdef WIN32
    sprintf_s(buffer, "%d", count);
#else
    sprintf(buffer, "%d", count);
#endif
    initRootProperties(connects, std::string("ConnectsPathElements") + buffer);

    connects->setRelatingElement(relating);
    connects->setRelatingConnectionType(relatingConnectionType);
    connects->setRelatedElement(related);
    connects->setRelatedConnectionType(relatedConnectionType);

    return connects;
}

ifc2x3::IfcRelSpaceBoundary* linkBySpaceBoundary(
    ifc2x3::IfcSpace* relating,
    ifc2x3::IfcElement* related)
{
    static unsigned int count = 0;
    char buffer[128];
    Step::RefPtr< ifc2x3::ExpressDataSet > mDataSet = (ifc2x3::ExpressDataSet*)relating->getExpressDataSet();
    ifc2x3::IfcRelSpaceBoundary* connects =
        mDataSet->createIfcRelSpaceBoundary().get();
#ifdef WIN32
    sprintf_s(buffer, "%d", count);
#else
    sprintf(buffer, "%d", count);
#endif
    initRootProperties(connects, std::string("SpaceBoundary") + buffer);

    connects->setRelatingSpace(relating);
    connects->setRelatedBuildingElement(related);
    connects->setPhysicalOrVirtualBoundary(ifc2x3::IfcPhysicalOrVirtualEnum_PHYSICAL);
    connects->setInternalOrExternalBoundary(ifc2x3::IfcInternalOrExternalEnum_INTERNAL);

    return connects;
}

ifc2x3::IfcRelVoidsElement* linkByVoidsElement(
    ifc2x3::IfcElement* relatingBuildingElement,
    ifc2x3::IfcFeatureElementSubtraction* relatedOpeningElement)
{
    static unsigned int count = 0;
    char buffer[128];
    Step::RefPtr< ifc2x3::ExpressDataSet > mDataSet =
        (ifc2x3::ExpressDataSet*)relatingBuildingElement->getExpressDataSet();
    ifc2x3::IfcRelVoidsElement* relVoidsElement =
        mDataSet->createIfcRelVoidsElement().get();
#ifdef WIN32
    sprintf_s(buffer, "%d", count);
#else
    sprintf(buffer, "%d", count);
#endif
    initRootProperties(relVoidsElement, std::string("VoidsElement") + buffer);

    ifc2x3::IfcLocalPlacement* localPlacement = (ifc2x3::IfcLocalPlacement*)relatedOpeningElement->getObjectPlacement();
    localPlacement->setPlacementRelTo(relatingBuildingElement->getObjectPlacement());

    relVoidsElement->setRelatingBuildingElement(relatingBuildingElement);
    relVoidsElement->setRelatedOpeningElement(relatedOpeningElement);

    return relVoidsElement;
}

ifc2x3::IfcRelFillsElement* linkByFillsElement(
    ifc2x3::IfcOpeningElement* relatingOpeningElement,
    ifc2x3::IfcElement* relatedBuildingElement)
{
    static unsigned int count = 0;
    char buffer[128];
    Step::RefPtr< ifc2x3::ExpressDataSet > mDataSet =
        (ifc2x3::ExpressDataSet*)relatingOpeningElement->getExpressDataSet();
    ifc2x3::IfcRelFillsElement* relFillsElement =
        mDataSet->createIfcRelFillsElement().get();
#ifdef WIN32
    sprintf_s(buffer, "%d", count);
#else
    sprintf(buffer, "%d", count);
#endif
    initRootProperties(relFillsElement, std::string("FillsElement") + buffer);

    ifc2x3::IfcLocalPlacement* localPlacement = (ifc2x3::IfcLocalPlacement*)relatedBuildingElement->getObjectPlacement();
    localPlacement->setPlacementRelTo(relatingOpeningElement->getObjectPlacement());

    relFillsElement->setRelatingOpeningElement(relatingOpeningElement);
    relFillsElement->setRelatedBuildingElement(relatedBuildingElement);

    return relFillsElement;
}

void ExportIFC()
{

    ads_name listeBlock;
    Adesk::Int32 nbBlock;
    AcDbVoidPtrArray solidArray;

    
    // ** First build an ExpressDataSet
    Step::RefPtr <ifc2x3::ExpressDataSet> expressDataSet = new ifc2x3::ExpressDataSet();

    // ** Geometric representation stuff
    std::vector<double> points, points2;
    std::vector<double> position, placement;
    CreateGeometricRepresentationVisitor cwrv(expressDataSet.get());

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
    initRootProperties(project.get(), "Project test");
    project->setDescription("c'est un projet de test");
    project->setLongName("Projet test");

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
    unit->setPrefix(ifc2x3::IfcSIPrefix_MILLI);
    unit->setName(ifc2x3::IfcSIUnitName_METRE);
    unitSelect->setIfcNamedUnit(unit.get());
    projectUnits->getUnits().insert(unitSelect);
    project->setUnitsInContext(projectUnits);

    // Build an Ifc site
    Step::RefPtr<ifc2x3::IfcSite> site = expressDataSet->createIfcSite();
    initRootProperties(site.get(), "The site");
    site->setCompositionType(ifc2x3::IfcElementCompositionEnum_ELEMENT);
    // Create representation
    points.clear();
    position.clear();
    placement.clear();
    cwrv.init();
    points.push_back(-100.0); points.push_back(-100.0);
    points.push_back(100.0); points.push_back(-100.0);
    points.push_back(100.0); points.push_back(100.0);
    points.push_back(-100.0); points.push_back(100.0);
    cwrv.set2DPolyline(points);
    if (!site->acceptVisitor(&cwrv)) {
        std::cerr << "ERROR while creating site representation" << std::endl;
    }
    // Link (Project <--> Site)
    linkByAggregate(project.get(), site.get());

    // Build an Ifc Building
    Step::RefPtr<ifc2x3::IfcBuilding> building = expressDataSet->createIfcBuilding();
    initRootProperties(building.get(), "The building");
    building->setCompositionType(ifc2x3::IfcElementCompositionEnum_ELEMENT);
    // Create representation
    points.clear();
    position.clear();
    placement.clear();
    cwrv.init();
    points.push_back(-10.0); points.push_back(-10.0);
    points.push_back(30.0); points.push_back(-10.0);
    points.push_back(30.0); points.push_back(30.0);
    points.push_back(-10.0); points.push_back(30.0);
    cwrv.set2DPolyline(points);
    if (!building->acceptVisitor(&cwrv)) {
        std::cerr << "ERROR while creating building representation" << std::endl;
    }
    // Link (Site <--> Building)
    linkByAggregate(site.get(), building.get());

    // Build an Ifc Building Storey
    Step::RefPtr<ifc2x3::IfcBuildingStorey> groundFloor = expressDataSet->createIfcBuildingStorey();
    initRootProperties(groundFloor.get(), "The ground floor");
    groundFloor->setCompositionType(ifc2x3::IfcElementCompositionEnum_ELEMENT);
    // Create representation
    points.clear();
    position.clear();
    placement.clear();
    cwrv.init();
    points.push_back(0.0); points.push_back(0.0);
    points.push_back(20.0); points.push_back(0.0);
    points.push_back(20.0); points.push_back(20.0);
    points.push_back(0.0); points.push_back(20.0);
    cwrv.set2DPolyline(points);
    if (!groundFloor->acceptVisitor(&cwrv)) {
        std::cerr << "ERROR while creating building storey representation" << std::endl;
    }
    // Link (Building <--> Storey)
    linkByAggregate(building.get(), groundFloor.get());


    Step::RefPtr<ifc2x3::IfcCoveringType> coveringtypeTest = expressDataSet->createIfcCoveringType();
    coveringtypeTest->setPredefinedType(ifc2x3::IfcCoveringTypeEnum_CLADDING);

    // Prepare selection set
    int err = acedSSGet(NULL, NULL, NULL, NULL, listeBlock);
    if (err != RTNORM)
    {
        nbBlock = 0;
    }
    else {
        acedSSLength(listeBlock, &nbBlock);
    }

    for (int j = 0; j < nbBlock; j++)
    {
        // add to selection set
        ads_name block;
        acedSSName(listeBlock, j, block);
        AcDbObjectId blockId;
        acdbGetObjectId(blockId, block);

        AcDbEntity* pEntBlock;
        acdbOpenAcDbEntity(pEntBlock, blockId, AcDb::kForRead);

        //TEST SCALE
        //pEntBlock->setLinetypeScale(1000);

        //AcDbBlockReference* pCloneBlock = (AcDbBlockReference*)pEntBlock->clone();
        
        pEntBlock->explode(solidArray);

        for (int i = 0; i < solidArray.length(); i++)
        {
            AcDbVoidPtrArray faceArray;

            AcDbIntArray snapModes;
            AcDbIntArray geomlds;

            AcDb3dSolid* pCloneSolid = (AcDb3dSolid*)solidArray.at(i);
            faceArray.removeAll();
            pCloneSolid->explode(faceArray); 
            AcString calque;
            pCloneSolid->layer(calque);

            AcString::Encoding encoding = AcString::Encoding::Utf8;
            int iso = calque.match("ISO", encoding);
            int pla = calque.match("PLA", encoding);
            int sup = calque.match("SUP", encoding);

            Step::RefPtr<ifc2x3::IfcCovering> objectCovering;
            Step::RefPtr<ifc2x3::IfcPlate> objectPlate;

            bool isPlate = false;

            if (iso > 0 )
            {
                objectCovering = expressDataSet->createIfcCovering();
            }
            else if (pla > 0)
            {
                objectPlate = expressDataSet->createIfcPlate();
                isPlate = true;
            }
            else if (sup > 0)
            {
                objectPlate = expressDataSet->createIfcPlate();
                isPlate = true;
            }
            else 
            {
                objectCovering = expressDataSet->createIfcCovering();
            }

            points.clear();
            cwrv.init();

            std::vector<Face*> listFaces;

            for (int k = 0; k < faceArray.length(); k++)
            {
                Acad::ErrorStatus es;
                AcDbVoidPtrArray polylineArray;
                AcGePoint3dArray listePoints;
                AcGePoint3dArray listePointsPolyline;
                AcDbFace* pCloneFace = (AcDbFace*)faceArray.at(k);
                pCloneFace->getGripPoints(listePoints, snapModes, geomlds);
                pCloneFace->explode(polylineArray);

                cwrv.setNbEdge(polylineArray.length());

                double angle;
                Step::RefPtr <ifc2x3::IfcCartesianPoint> EdgeStart;
                Step::RefPtr <ifc2x3::IfcCartesianPoint> EdgeEnd;
                bool isCompositeCurve = false;

                Face* face = nullptr;

                for (int t = 0; t < polylineArray.length(); t++)
                {
                    AcDbArc* pCloneArc = (AcDbArc*)polylineArray.at(t);

                    angle = pCloneArc->totalAngle();
                    isCompositeCurve = angle > 0;

                    if (isCompositeCurve) break;
                }

                if (isCompositeCurve)
                {
                    face = new EdgeLoop();

                    for (int t = 0; t < polylineArray.length(); t++) //d�composer les faces pour avoir des polylines afin de verifier si c'est des arcs ou des droites 
                    {
                        AcDbArc* pCloneArc = (AcDbArc*)polylineArray.at(t);
                        AcDbPolyline* pClonePolyline = (AcDbPolyline*)polylineArray.at(t);

                        listePointsPolyline.removeAll();
                        pClonePolyline->getGripPoints(listePointsPolyline, snapModes, geomlds);

                        angle = pCloneArc->totalAngle();

                        if (angle == 0)
                        {
                            //auto polyline = new CompositeCurveSegmentPolyline();
                            EdgeLoop* edgeLoop = static_cast<EdgeLoop*>(face);

                            /*for (size_t j = 0; j < 2; j++)
                            {
                                polyline->Points.push_back({ listePointsPolyline[j].x * 0.001, listePointsPolyline[j].y * 0.001, listePointsPolyline[j].z * 0.001 });
                            }*/

                            Edge* edge = new Edge("edge");
                            Vertex EdgeS;
                            Vertex EdgeE;
                            pCloneArc->getStartPoint(EdgeS.VertexGeometry);
                            pCloneArc->getEndPoint(EdgeE.VertexGeometry);

                            edge->EdgeStart = EdgeS;
                            edge->EdgeEnd = EdgeE;
                            edge->TypeEdge = "edge";
                            
                            
                            edgeLoop->EdgeList.push_back(edge);

                           /* CompositeCurve* curve = static_cast<CompositeCurve*>(face);
                            curve->CompositeCurveSegments.push_back(polyline);*/
                        }
                        else if (angle != 0)
                        {
                            //CompositeCurve* curve = static_cast<CompositeCurve*>(face);
                            //TrimmedCurveEx* trimmedCurve = new TrimmedCurveEx;

                            EdgeLoop* edgeLoop = static_cast<EdgeLoop*>(face);
                            EdgeCurve* edgeCurve = new EdgeCurve();
                            Vertex EdgeS;
                            Vertex EdgeE;
                            pCloneArc->getStartPoint(EdgeS.VertexGeometry);
                            pCloneArc->getEndPoint(EdgeE.VertexGeometry);

                            edgeCurve->EdgeStart = EdgeS;
                            edgeCurve->EdgeEnd = EdgeE;
                            edgeCurve->Circle.Centre = pCloneArc->center();
                            edgeCurve->Circle.Rayon = pCloneArc->radius();
                            edgeCurve->SameSense = Step::Boolean::BFalse; //� r�cup
                            edgeCurve->TypeEdge = "edgeCurve";

                            edgeLoop->EdgeList.push_back(edgeCurve);

                            ////set circle 
                            //trimmedCurve->Circle.Centre = pCloneArc->center();
                            //trimmedCurve->Circle.Rayon = pCloneArc->radius();

                            ////set trimmedCurve (#112767)
                            //trimmedCurve->Trim1 = (Utils::RadianToDegree(pCloneArc->startAngle()));
                            //trimmedCurve->Trim2 = (Utils::RadianToDegree(pCloneArc->endAngle()));
                            //trimmedCurve->Preference = ifc2x3::IfcTrimmingPreference::IfcTrimmingPreference_PARAMETER;
                            //trimmedCurve->SenseAgreement = Step::Boolean::BFalse; //� r�cup

                            ////set compositeCurveSegment
                            //trimmedCurve->Transition = ifc2x3::IfcTransitionCode::IfcTransitionCode_CONTINUOUS;
                            //trimmedCurve->SameSense = Step::Boolean::BTrue; //� r�cup

                            //curve->CompositeCurveSegments.push_back(trimmedCurve);
                        }
                    }
                }
                else if (!isCompositeCurve)
                {
                    PolylineEx* polyline = new PolylineEx();

                    for (int l = 0; l < listePoints.length(); l++)
                    {
                        AcGePoint3d point = listePoints.at(l);

                        // Init root properties
                        if (iso > 0)
                        {
                            initRootProperties(objectCovering.get(), "Covering_Isolation");
                            objectCovering->setOwnerHistory(ownerHistory);
                        }
                        else if (pla > 0)
                        {
                            initRootProperties(objectPlate.get(), "Plate_Plateau");
                            objectPlate->setOwnerHistory(ownerHistory);
                        }
                        else if (sup > 0)
                        {
                            initRootProperties(objectPlate.get(), "Plate_Support");
                            objectPlate->setOwnerHistory(ownerHistory);
                        }
                        else
                        {
                            initRootProperties(objectCovering.get(), "Covering");
                            objectCovering->setOwnerHistory(ownerHistory);
                        }

                        if (iso > 0)
                        {
                            objectCovering->setPredefinedType(ifc2x3::IfcCoveringTypeEnum_INSULATION);
                        }
                        else if (isPlate == false)
                        {
                            objectCovering->setPredefinedType(ifc2x3::IfcCoveringTypeEnum_CLADDING);
                        }

                        polyline->Points.push_back({ point[0] * 0.001, point[1] * 0.001, point[2] * 0.001 });
                    }

                    face = polyline;
                }

                listFaces.push_back(face);
            }

            cwrv.AjoutFace(listFaces);

            position.clear();
            placement.clear();
            position.push_back(0.0);
            position.push_back(0.0);
            position.push_back(0.0);
            cwrv.setPosition(position); 

            if (iso > 0)
            {
                if (!objectCovering->acceptVisitor(&cwrv)) {
                    std::cerr << "ERROR while creating covering representation" << std::endl;
                }
                linkByContainedInSpatial(groundFloor.get(), objectCovering.get());
            }
            else if (pla > 0)
            {
                if (!objectPlate->acceptVisitor(&cwrv)) {
                    std::cerr << "ERROR while creating covering representation" << std::endl;
                }
                linkByContainedInSpatial(groundFloor.get(), objectPlate.get());
            }
            else if (sup > 0)
            {
                if (!objectPlate->acceptVisitor(&cwrv)) {
                    std::cerr << "ERROR while creating covering representation" << std::endl;
                }
                linkByContainedInSpatial(groundFloor.get(), objectPlate.get());
            }
            else
            {
                if (!objectCovering->acceptVisitor(&cwrv)) {
                    std::cerr << "ERROR while creating covering representation" << std::endl;
                }
                linkByContainedInSpatial(groundFloor.get(), objectCovering.get());
            }
        }

        solidArray.removeAll();
    }

    // On cr�e le fichier seulement si il y avait un moins un objet
    if (nbBlock > 0)
    {
        // ** Write the file        
        ifc2x3::SPFWriter writer(expressDataSet.get());
        std::ofstream filestream("TestExport.ifc", std::ofstream::out);

        bool status = writer.write(filestream);
        filestream.close();

        acedAlert(_T("EXPORT DONE"));
    }
}
