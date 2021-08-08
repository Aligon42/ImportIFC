#include "tchar.h"
#include <aced.h>
#include <rxregsvc.h> 
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>

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

#include "TestUI.h"

const wchar_t* GetWC(const char* c, ...)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

void WriteLogs()
{
	std::ofstream fw(Construction::s_LogPath, std::ofstream::app);
	if (fw.is_open())
	{
		for (auto& vec : Construction::s_Logs)
		{
			for (auto& str : vec.second)
				fw << str;

			vec.second.clear();
		}

		fw.close();
	}
}

void ExploreElement(std::map<Step::Id, Step::BaseObjectPtr>* elements, std::vector<IFCObject*>& objects)
{
	ComputePlacementVisitor placementVisitor;
	auto it = elements->begin();

	while (it != elements->end())
	{
		auto start = std::chrono::high_resolution_clock::now();

		auto& buildingElement = *(it->second);

		int key = it->first;
		std::string entity = buildingElement.getType().getName();

		it++;

		//if (key != 5557) continue;
		//if (key != 16251) continue;

		ObjectVisitor visitor;
		buildingElement.acceptVisitor(&visitor);

		auto obj = visitor.getIfcObject();

		buildingElement.acceptVisitor(&placementVisitor);
		obj->LocalTransform = placementVisitor.getTransformation();

		objects.push_back(obj);

		auto end = std::chrono::high_resolution_clock::now() - start;
		std::stringstream ss;
		ss <<"Key: "<< key << " - " << entity << " - " << std::chrono::duration_cast<std::chrono::microseconds>(end).count() / 1000.0 << " ms. \n";
		Construction::s_Logs[entity].push_back(ss.str());
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

	std::wstring fpath(fname);
	std::string path(fpath.begin(), fpath.end());
	TCHAR NPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, NPath);
	std::wstring curPath(NPath);

	Construction::s_LogPath = std::string(curPath.begin(), curPath.end());

	int startIndex = path.find_last_of("\\") + 1;
	Construction::s_LogPath += "\\" + path.substr(startIndex, path.length() - 4 - startIndex) + ".txt";

	const auto& apfaoifznbdpkfm = expressDataSet->getAllIfcElement().m_refList;

	int total = 0;
	for (auto& test : apfaoifznbdpkfm)
	{
		if (test->size() == 0) continue;
		std::string type = (*test->begin()).second->type();
		if (type == "IfcOpeningElement") continue;
		total += test->size();
	}

	std::ofstream fw(Construction::s_LogPath, std::ofstream::out);
	if (fw.is_open())
	{
		fw << "Nombre éléments : " << total << "\n";
		fw.close();
	}

	threads.push_back(std::thread([&]()
	{
		// IfcRelVoidsElement
		for (auto& voids : expressDataSet->getAllIfcRelVoidsElement())
		{
			ObjectVisitor visitor;
			voids.acceptVisitor(&visitor);
			auto obj = visitor.getIfcObject();

			voids.acceptVisitor(&placementVisitor);
			obj->LocalTransform = placementVisitor.getTransformation();

			obj->LocalTransform *= (*obj->ShapeRepresentations.begin()).Transformation;

			if (Construction::s_ObjectVoids.find(obj->VoidKey) != Construction::s_ObjectVoids.end())
			{
				Construction::s_ObjectVoids[obj->VoidKey].push_back(obj);
			}
			else
			{
				std::vector<IFCObject*> vec;
				vec.push_back(obj);
				Construction::s_ObjectVoids.insert(std::make_pair(obj->VoidKey, vec));
			}
		}
	}));

	threads.push_back(std::thread([&]()
	{
		// IfcStyledItem
		for (auto& styles : expressDataSet->getAllIfcStyledItem())
		{
			ObjectVisitor visitor;
			int key = styles.getKey();

			styles.acceptVisitor(&visitor);
			Style style = visitor.getStyle();
			Construction::s_Styles.emplace(std::make_pair(style.keyItem, style));
		}
	}));

	std::map<std::string, std::vector<IFCObject*>> objects;

	for (auto ifcSite : expressDataSet->getAllIfcSite().m_refList)
	{
		if (ifcSite->size() > 0)
		{
			std::string type = (*ifcSite->begin()).second->type();

			if (type == "IfcOpeningElement") continue;

			objects.emplace(std::make_pair(type, std::vector<IFCObject*>()));
			Construction::s_Logs.emplace(std::make_pair(type, std::vector<std::string>()));

			objects.emplace(std::make_pair(type, std::vector<IFCObject*>()));
			threads.push_back(std::thread(ExploreElement, ifcSite, std::ref(objects[type])));
		}
	}

	/*for (auto mappedItems : expressDataSet->getAllIfcMappedItem().m_refList)
	{
		if (mappedItems->size() > 0)
		{
			std::vector<IFCObject*> vector;
			std::string type = (*mappedItems->begin()).second->type();

			objects.emplace(std::make_pair(type, vector));

			threads.push_back(std::thread(ExploreElement, mappedItems, std::ref(objects[type])));
		}
	}*/

	for (auto element : expressDataSet->getAllIfcElement().m_refList)
	{
		if (element->size() > 0)
		{
			std::string type = (*element->begin()).second->type();

			if (type == "IfcOpeningElement") continue;

			objects.emplace(std::make_pair(type, std::vector<IFCObject*>()));
			Construction::s_Logs.emplace(std::make_pair(type, std::vector<std::string>()));

			threads.push_back(std::thread(ExploreElement, element, std::ref(objects[type])));
		}
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	Construction::s_Logs.emplace(std::make_pair("Rendu", std::vector<std::string>()));
	Construction::s_Logs.emplace(std::make_pair("Instantiation", std::vector<std::string>()));

	for (auto& type : objects)
	{
		for (auto& obj : type.second)
		{
			auto& shape = *obj->ShapeRepresentations.begin();

			if (obj->ShapeRepresentations.begin() == obj->ShapeRepresentations.end()) continue;

			if (obj->Entity != "IfcColumn" && obj->Entity != "IfcBeam")
			{
				if (obj->IsMappedItem)
				{
					auto& subShape = *shape.SubShapeRepresentations.begin();

					if (shape.SubShapeRepresentations.begin() == shape.SubShapeRepresentations.end()) continue;

					if (subShape.EntityType == "IfcExtrudedAreaSolid")
					{
						if (subShape.ProfilDefName != "IfcArbitraryClosedProfileDef")
						{
							if (subShape.ProfilDef != nullptr)
								subShape.ProfilDef->createSolid3dProfil();
						}
						else
						{
							Construction construction(obj);
							construction.Extrusion();
						}
					}
					else if (subShape.EntityType == "IfcBooleanClippingResult")
					{
						Construction construction(obj);
						construction.Extrusion();
					}
					else if (subShape.EntityType == "IfcFacetedBrep" || subShape.EntityType == "IfcFaceBasedSurfaceModel" || subShape.EntityType == "IfcShellBasedSurfaceModel")
					{
						Construction construction(obj);
						construction.CreationFaceSolid();
					}
					else if (subShape.EntityType == "IfcBoundingBox")
					{
						// TODO createBoundingBox(map.boxMap, entity, map.keyItemsMap[j], listStyle);
					}
				}
				else if ((shape.EntityType == "IfcExtrudedAreaSolid" || shape.BooleanResult) && !obj->IsMappedItem)
				{
					if (shape.ProfilDefName != "IfcArbitraryClosedProfileDef")
					{
						if (shape.ProfilDef != nullptr)
							shape.ProfilDef->createSolid3dProfil();
					}
					else
					{
						if (!shape.BooleanResult)
							obj->LocalTransform *= shape.Transformation;

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
			}
			else if (obj->Entity == "IfcColumn" || obj->Entity == "IfcBeam")
			{
				if (obj->IsMappedItem)
				{
					// TODO MappedItem
					/*for (int j = 0; j < obj->KeyMappedItems.size(); j++)
					{
						shape.ProfilDef->createSolid3dProfil({});
					}*/
				}
				else
				{
					if (shape.ProfilDef != nullptr)
						shape.ProfilDef->createSolid3dProfil();
				}
			}
		}
	}

	WriteLogs();

	for (auto& type : objects)
	{
		for (auto* obj : type.second)
			delete obj;

		type.second.clear();
	}

	for (auto& el : Construction::s_ObjectVoids)
	{
		for (auto obj : el.second)
			delete obj;

		el.second.clear();
	}

	Construction::s_ObjectVoids.clear();
	Construction::s_Styles.clear();
	Construction::s_Logs.clear();

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
