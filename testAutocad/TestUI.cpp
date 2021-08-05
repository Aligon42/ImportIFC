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
#include "ObjectVisitor.h"
#include "MethodeConstruction.h"
#include "Construction.h"

#include <adscodes.h>

#include <iostream>
#include <thread>

void initApp();
void unloadApp();

const wchar_t* GetWC(const char* c, ...)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

void ExploreElement(std::map<Step::Id, Step::BaseObjectPtr>* elements, std::vector<IFCObject*>& objects)
{
	ComputePlacementVisitor placementVisitor;
	auto it = elements->begin();

	while (it != elements->end())
	{
		auto& buildingElement = *(it->second);

		int key = it->first;
		std::string entity = buildingElement.getType().getName();

		ObjectVisitor visitor;
		buildingElement.acceptVisitor(&visitor);

		auto obj = visitor.getIfcObject();

		buildingElement.acceptVisitor(&placementVisitor);
		obj->LocalTransform = placementVisitor.getTransformation();

		objects.push_back(obj);

		it++;
	}
}

void loadIfc()
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

	bool shouldWrite = false;
	bool inMemory = false;

	// ** open, load, close the file
	std::ifstream ifcFile;
	ifcFile.open(fname);

	ifc2x3::SPFReader reader;
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
	std::vector<std::thread> threads;
	std::map<int, Style> listStyles;
	std::map<int, MappedItem> dicoMappedItem;

	//threads.push_back(std::thread([&]()
	//{
	//	// IfcRelVoidsElement
	//	for (auto& voids : expressDataSet->getAllIfcRelVoidsElement())
	//	{
	//		CreateConstructionPointVisitor visitor;
	//		int key = (int)voids.getKey();

	//		voids.acceptVisitor(&visitor);

	//		ObjectVoid objectVoid;
	//		objectVoid.keyForVoid = visitor.getkeyForVoid();
	//		objectVoid.NameProfilDef = visitor.getNameProfildef();
	//		if (objectVoid.NameProfilDef == "IfcArbitraryClosedProfileDef")
	//		{
	//			objectVoid.points1 = visitor.getPoints();
	//			objectVoid.nbArg = visitor.getNbArgPolyline();
	//		}
	//		else if (objectVoid.NameProfilDef == "IfcCircleProfileDef")
	//		{
	//			objectVoid.radius = (static_cast<Circle_profilDef*>(visitor.getProfilDef().get()))->Radius;
	//		}
	//		else if (objectVoid.NameProfilDef == "IfcRectangleProfileDef")
	//		{
	//			objectVoid.XDim = (static_cast<Rectangle_profilDef*>(visitor.getProfilDef().get()))->XDim;
	//			objectVoid.YDim = (static_cast<Rectangle_profilDef*>(visitor.getProfilDef().get()))->YDim;
	//		}
	//		objectVoid.VecteurExtrusion = visitor.getVectorDirection();
	//		objectVoid.hauteurExtrusion = visitor.getHauteurExtrusion();
	//		objectVoid.listPlan = visitor.getPlanPolygonal();
	//		objectVoid.listLocationPolygonal = visitor.getLocationPolygonal();
	//		objectVoid.AgreementHalf = visitor.getAgreementHalfBool();
	//		objectVoid.AgreementPolygonal = visitor.getAgreementPolygonalBool();
	//		objectVoid.listEntityHalf = visitor.getListEntityHalf();
	//		objectVoid.listEntityPolygonal = visitor.getListEntityPolygonal();

	//		voids.acceptVisitor(&placementVisitor);
	//		objectVoid.transform1 = placementVisitor.getTransformation();
	//		Matrix4 transformation = visitor.getTransformation();

	//		objectVoid.transform1 *= transformation;

	//		Construction::s_ObjectVoids.insert(std::make_pair(objectVoid.keyForVoid, objectVoid));
	//	}
	//}));

	//threads.push_back(std::thread([&]()
	//{
	//	// IfcStyledItem
	//	for (auto& styles : expressDataSet->getAllIfcStyledItem())
	//	{
	//		CreateConstructionPointVisitor visitor1;
	//		int key = styles.getKey();

	//		styles.acceptVisitor(&visitor1);
	//		Style style = visitor1.getStyle();
	//		listStyles.emplace(std::make_pair(style.keyItem, style));
	//	}
	//}));

	std::map<std::string, std::vector<IFCObject*>> objects;

	/*for (auto ifcSite : expressDataSet->getAllIfcSite().m_refList)
	{
		if (ifcSite->size() > 0)
		{
			std::vector<IFCObject> vector;
			std::string type = (*ifcSite->begin()).second->type();

			objects.emplace(std::make_pair(type, vector));

			threads.push_back(std::thread(ExploreElement, ifcSite, std::ref(objects[type])));
		}
	}*/

	for (auto element : expressDataSet->getAllIfcWall().m_refList)
	{
		if (element->size() > 0)
		{
			std::vector<IFCObject*> vector;
			std::string type = (*element->begin()).second->type();

			objects.emplace(std::make_pair(type, vector));

			threads.push_back(std::thread(ExploreElement, element, std::ref(objects[type])));
		}
	}

	/*for (auto mappedItems : expressDataSet->getAllIfcMappedItem().m_refList)
	{
		if (mappedItems->size() > 0)
		{
			std::vector<IFCObject> vector;
			std::string type = (*mappedItems->begin()).second->type();

			objects.emplace(std::make_pair(type, vector));

			threads.push_back(std::thread(ExploreElement, mappedItems, std::ref(objects[type])));
		}
	}*/

	for (auto& thread : threads)
	{
		thread.join();
	}

	for (auto& type : objects)
	{
		for (auto& obj : type.second)
		{
			for (auto& shape : obj->ShapeRepresentations)
			{
				if (obj->Entity != "IfcColumn" && obj->Entity != "IfcBeam")
				{
					if (shape.EntityType == "IfcExtrudedAreaSolid" && !obj->IsMappedItem)
					{
						if (shape.ProfilDefName != "IfcArbitraryClosedProfileDef")
						{
							//shape.ProfilDef->createSolid3dProfil({});
						}
						else
						{
							Construction construction(obj);
							construction.Extrusion();
						}
					}
					else if (shape.EntityType == "IfcBooleanClippingResult" || shape.EntityType == "IfcBooleanResult")
					{
						if (shape.ProfilDefName != "IfcArbitraryClosedProfileDef")
						{
							//shape.ProfilDef->createSolid3dProfil({});
						}
						else
						{
							Construction construction(obj);
							construction.Extrusion();
						}
					}
					else if (shape.EntityType == "IfcFacetedBrep" || shape.EntityType == "IfcFaceBasedSurfaceModel" || shape.EntityType == "IfcShellBasedSurfaceModel" && !obj->IsMappedItem)
					{
						Construction construction(obj);
						construction.CreationFaceSolid();
					}
					else if (shape.EntityType == "IfcBoundingBox" && !obj->IsMappedItem)
					{
						// TODO createBoundingBox(box, entity, keyProfilDef, listStyle);
					}
					else if (shape.EntityType == "IfcMappedItem" && obj->IsMappedItem)
					{
						for (int j = 0; j < obj->KeyMappedItems.size(); j++)
						{
							MappedItem& map = dicoMappedItem[obj->KeyMappedItems[j]];

							if (map.nameItemsMap[0] == "IfcExtrudedAreaSolid")
							{
								map.transform1Map *= obj->LocalTransform;
								map.transform1Map *= shape.Transformation;

								if (shape.ProfilDefName != "IfcArbitraryClosedProfileDef")
								{
									//shape.ProfilDef->createSolid3dProfil({});
								}
								else
								{
									Construction construction(obj);
									construction.Extrusion();
								}
							}
							else if (map.nameItemsMap[0] == "IfcBooleanClippingResult")
							{
								Construction construction(obj);
								construction.Extrusion();
							}
							else if (map.nameItemsMap[0] == "IfcFacetedBrep" || map.nameItemsMap[0] == "IfcFaceBasedSurfaceModel" || map.nameItemsMap[0] == "IfcShellBasedSurfaceModel")
							{
								//shape.ProfilDef->createSolid3dProfil({});
							}
							else if (map.nameItemsMap[0] == "IfcBoundingBox")
							{
								// TODO createBoundingBox(map.boxMap, entity, map.keyItemsMap[j], listStyle);
							}
						}
					}
				}
				else if (obj->Entity == "IfcColumn" || obj->Entity == "IfcBeam")
				{
					if (shape.EntityType == "IfcMappedItem" && obj->IsMappedItem)
					{
						for (int j = 0; j < obj->KeyMappedItems.size(); j++)
						{
							MappedItem& map = dicoMappedItem[obj->KeyMappedItems[j]];
							//shape.ProfilDef->createSolid3dProfil({});
						}
					}
					else
					{
						//shape.ProfilDef->createSolid3dProfil({});
					}
				}
			}
		}
	}

	//if (shouldWrite)
	//{
	//    // ** Write the file
	//    ifc2x3::SPFWriter writer(expressDataSet);
	//    std::ofstream filestream;
	//    filestream.open(writeFile);

	//    bool status = writer.write(filestream);
	//    filestream.close();
	//}
	//listVoid.clear();

	for (auto& type : objects)
	{
		for (auto* obj : type.second)
			delete obj;

		type.second.clear();
	}

	listStyles.clear();
	delete expressDataSet;
}

void initApp()
{
	// register a command with the AutoCAD command mechanism
	acedRegCmds->addCommand(_T("IMPORT_COMMANDS"),
		_T("Import"),
		_T("Import"),
		ACRX_CMD_TRANSPARENT,
		loadIfc);
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
