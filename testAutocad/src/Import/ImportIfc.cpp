#include "ImportIfc.h"

#include "Autocad/Autocad.h"
#include "Visitors/IfcReader.h"

ImportIfc::~ImportIfc()
{
}

void ImportIfc::LoadIfc()
{
	SelectFile();
	if (m_FilePath == "") return;

	ReadFile();
}

void ImportIfc::ReadFile()
{
	// ** open, load, close the file
	std::ifstream ifcFile;
	ifcFile.open(m_FilePath);

	if (ifcFile.is_open())
	{
		LogAutocad("\nreading file ...");
	}
	else
	{
		// TODO: vérifier que ça écrit corectement
		LogAutocad("ERROR: failed to open <%s>\n", m_FilePath);
	}

	ifcFile.seekg(0, ifcFile.end);
	std::ifstream::pos_type length = ifcFile.tellg();
	ifcFile.seekg(0, ifcFile.beg);

	IfcReader reader(m_FilePath, &ifcFile);
	bool result = reader.Read();
	ifcFile.close();

	if (result)
	{
		LogAutocad("OK!!\n");
	}
	else
	{
		std::vector<std::string> errors;
		reader.GetReaderErrors(errors);

		std::vector<std::string>::iterator it = errors.begin();
		while (it != errors.end())
		{
			LogAutocad("%s\n", *it);
			++it;
		}
	}

	reader.GenerateExpressDataSetAndValidate();
	reader.Visit();
}

void ImportIfc::SelectFile()
{
	m_FilePath = Autocad::OpenAndSelectFile();
}

//void loadIfc()
//{
//	/*bool shouldWrite = false;
//	bool inMemory = false;
//
//	ifc2x3::SPFReader reader;*/
//	//    reader.setCallBack(&cb);
//
//	//if (ifcFile.is_open())
//	//{
//	//	acutPrintf(_T("\nreading file ..."));
//	//}
//	//else
//	//{
//	//	acutPrintf(_T("ERROR: failed to open <%s>\n"), fname);
//	//}
//
//	//// get length of file
//	//ifcFile.seekg(0, ifcFile.end);
//	//std::ifstream::pos_type length = ifcFile.tellg();
//	//ifcFile.seekg(0, ifcFile.beg);
//
//	//bool result = reader.read(ifcFile, inMemory ? length : (std::ifstream::pos_type)0);
//	//ifcFile.close();
//
//	//if (result)
//	//	acutPrintf(_T("OK!!\n"));
//	//else
//	//{
//	//	std::vector<std::string> errors = reader.errors();
//	//	std::vector<std::string>::iterator it = errors.begin();
//	//	while (it != errors.end())
//	//	{
//	//		acutPrintf(_T("%s\n"), *it);
//	//		++it;
//	//	}
//	//}
//
//	// ** get the model
//	/*ifc2x3::ExpressDataSet* expressDataSet = dynamic_cast<ifc2x3::ExpressDataSet*>(reader.getExpressDataSet());*/
//
//	/*if (expressDataSet == NULL)
//	{
//		acutPrintf(_T("Ho no ... there is no ExpressDataSet.\n"));
//	}*/
//
//	//if (shouldWrite)
//	//{
//	//	// ** Instantiate the model if we want to write it
//	//	expressDataSet->instantiateAll(/*&cb*/);
//	//}
//
//	// ** Check the root of the model
//	//Step::RefLinkedList< ifc2x3::IfcProject > projects = expressDataSet->getAllIfcProject();
//	//if (projects.size() == 0) {
//	//	acutPrintf(_T("Strange ... there is no IfcProject\n"));
//	//}
//	//else if (projects.size() > 1) {
//	//	acutPrintf(_T("Strange ... there more than one IfcProject\n"));
//	//}
//	//else {
//	//	Step::RefPtr< ifc2x3::IfcProject > project = &*(projects.begin());
//	//	//Log("Project name is: %s\n", GetWC(project->getName().toISO_8859(Step::String::Western_European)));
//	//	acutPrintf(_T("Project name is: %s\n"), GetWC((project->getName().toISO_8859(Step::String::Western_European)).c_str()));
//	//	if (Step::isUnset(project->getLongName().toISO_8859(Step::String::Western_European))) {
//	//		project->setLongName("Je lui donne le nom que je veux");
//	//	}
//	//	acutPrintf(_T("Project long name is: %s\n"), GetWC((project->getLongName().toISO_8859(Step::String::Western_European)).c_str()));
//	//}
//
//	//TEST_ASSERT(expressDataSet != nullptr);
//	/*expressDataSet->instantiateAll();
//	ComputePlacementVisitor placementVisitor;
//	std::vector<std::thread> threads;
//	std::map<std::string, std::vector<std::shared_ptr<IFCObject>>> objects;
//
//	std::wstring fpath(fname);
//	std::string path(fpath.begin(), fpath.end());
//	TCHAR NPath[MAX_PATH];
//	GetCurrentDirectory(MAX_PATH, NPath);
//	std::wstring curPath(NPath);
//
//	Construction::s_LogPath = std::string(curPath.begin(), curPath.end());
//
//	int startIndex = path.find_last_of("\\") + 1;
//	Construction::s_LogPath += "\\" + path.substr(startIndex, path.length() - 4 - startIndex) + ".txt";
//
//	Construction::s_Logs.emplace(std::make_pair("Rendu", std::vector<std::string>()));
//	Construction::s_Logs.emplace(std::make_pair("Instantiation", std::vector<std::string>()));
//
//	const auto& apfaoifznbdpkfm = expressDataSet->getAllIfcElement().m_refList;
//
//	int total = 0;
//	for (auto& test : apfaoifznbdpkfm)
//	{
//		if (test->size() == 0) continue;
//		std::string type = (*test->begin()).second->type();
//		if (type == "IfcOpeningElement") continue;
//		total += test->size();
//	}
//
//	std::ofstream fw(Construction::s_LogPath, std::ofstream::out);
//	if (fw.is_open())
//	{
//		fw << "Nombre éléments : " << total << "\n";
//		fw.close();
//	}*/
//
//	//threads.push_back(std::thread([&]()
//	//{
//	//	// IfcRelVoidsElement
//	//	for (auto& voids : expressDataSet->getAllIfcRelVoidsElement())
//	//	{
//	//		ObjectVisitor visitor;
//	//		voids.acceptVisitor(&visitor);
//	//		auto obj = visitor.getIfcObject();
//
//	//		voids.acceptVisitor(&placementVisitor);
//	//		obj->LocalTransform = placementVisitor.getTransformation();
//
//	//		obj->LocalTransform *= (*obj->ShapeRepresentations.begin()).Transformation;
//
//	//		if (Construction::s_ObjectVoids.find(obj->VoidKey) != Construction::s_ObjectVoids.end())
//	//		{
//	//			Construction::s_ObjectVoids[obj->VoidKey].push_back(obj);
//	//		}
//	//		else
//	//		{
//	//			std::vector<std::shared_ptr<IFCObject>> vec;
//	//			vec.push_back(obj);
//	//			Construction::s_ObjectVoids.insert(std::make_pair(obj->VoidKey, vec));
//	//		}
//	//	}
//	//}));
//
//	//threads.push_back(std::thread([&]()
//	//{
//	//	// IfcStyledItem
//	//	for (auto& styles : expressDataSet->getAllIfcStyledItem())
//	//	{
//	//		ObjectVisitor visitor;
//	//		int key = styles.getKey();
//
//	//		styles.acceptVisitor(&visitor);
//	//		Style style = visitor.getStyle();
//	//		Construction::s_Styles.emplace(std::make_pair(style.keyItem, style));
//	//	}
//	//}));
//
//	/*for (auto ifcSite : expressDataSet->getAllIfcSite().m_refList)
//	{
//		if (ifcSite->size() > 0)
//		{
//			std::string type = (*ifcSite->begin()).second->type();
//
//			if (type == "IfcOpeningElement") continue;
//
//			Construction::s_Logs.emplace(std::make_pair(type, std::vector<std::string>()));
//			objects.emplace(std::make_pair(type, std::vector<std::shared_ptr<IFCObject>>()));
//
//			threads.push_back(std::thread(ExploreElement, ifcSite, std::ref(objects[type])));
//		}
//	}*/
//
//	/*for (auto element : expressDataSet->getAllIfcElement().m_refList)
//	{
//		if (element->size() > 0)
//		{
//			std::string type = (*element->begin()).second->type();
//
//			if (type == "IfcOpeningElement") continue;
//
//			Construction::s_Logs.emplace(std::make_pair(type, std::vector<std::string>()));
//			objects.emplace(std::make_pair(type, std::vector<std::shared_ptr<IFCObject>>()));
//
//			threads.push_back(std::thread(ExploreElement, element, std::ref(objects[type])));
//		}
//	}*/
//
//	//for (auto& thread : threads)
//	//{
//	//	thread.join();
//	//}
//
//	//for (auto& type : objects)
//	//{
//	//	for (auto& obj : type.second)
//	//	{
//	//		auto& shape = *obj->ShapeRepresentations.begin();
//
//	//		if (obj->ShapeRepresentations.begin() == obj->ShapeRepresentations.end()) continue;
//
//	//		if (obj->Entity != "IfcColumn" && obj->Entity != "IfcBeam")
//	//		{
//	//			if (obj->IsMappedItem)
//	//			{
//	//				auto& subShape = *shape.SubShapeRepresentations.begin();
//
//	//				if (shape.SubShapeRepresentations.begin() == shape.SubShapeRepresentations.end()) continue;
//
//	//				if (subShape.EntityType == "IfcExtrudedAreaSolid")
//	//				{
//	//					if (subShape.ProfilDefName != "IfcArbitraryClosedProfileDef")
//	//					{
//	//						if (subShape.ProfilDef != nullptr)
//	//							subShape.ProfilDef->createSolid3dProfil();
//	//					}
//	//					else
//	//					{
//	//						Construction construction(obj);
//	//						construction.Extrusion();
//	//					}
//	//				}
//	//				else if (subShape.EntityType == "IfcBooleanClippingResult")
//	//				{
//	//					Construction construction(obj);
//	//					construction.Extrusion();
//	//				}
//	//				else if (subShape.EntityType == "IfcFacetedBrep" || subShape.EntityType == "IfcFaceBasedSurfaceModel" || subShape.EntityType == "IfcShellBasedSurfaceModel")
//	//				{
//	//					Construction construction(obj);
//	//					construction.CreationFaceSolid();
//	//				}
//	//				else if (subShape.EntityType == "IfcBoundingBox")
//	//				{
//	//					// TODO createBoundingBox(map.boxMap, entity, map.keyItemsMap[j], listStyle);
//	//				}
//	//			}
//	//			else if ((shape.EntityType == "IfcExtrudedAreaSolid" || shape.BooleanResult) && !obj->IsMappedItem)
//	//			{
//	//				if (shape.ProfilDefName != "IfcArbitraryClosedProfileDef")
//	//				{
//	//					if (shape.ProfilDef != nullptr)
//	//						shape.ProfilDef->createSolid3dProfil();
//	//				}
//	//				else
//	//				{
//	//					if (!shape.BooleanResult)
//	//						obj->LocalTransform *= shape.Transformation;
//
//	//					Construction construction(obj);
//	//					construction.Extrusion();
//	//				}
//	//			}
//	//			else if (shape.EntityType == "IfcFacetedBrep" || shape.EntityType == "IfcFaceBasedSurfaceModel" || shape.EntityType == "IfcShellBasedSurfaceModel" && !obj->IsMappedItem)
//	//			{
//	//				Construction construction(obj);
//	//				construction.CreationFaceSolid();
//	//			}
//	//			else if (shape.EntityType == "IfcBoundingBox" && !obj->IsMappedItem)
//	//			{
//	//				// TODO createBoundingBox(box, entity, keyProfilDef, listStyle);
//	//			}
//	//		}
//	//		else if (obj->Entity == "IfcColumn" || obj->Entity == "IfcBeam")
//	//		{
//	//			if (obj->IsMappedItem)
//	//			{
//	//				// TODO MappedItem
//	//				/*for (int j = 0; j < obj->KeyMappedItems.size(); j++)
//	//				{
//	//					shape.ProfilDef->createSolid3dProfil({});
//	//				}*/
//	//			}
//	//			else
//	//			{
//	//				if (shape.ProfilDef != nullptr)
//	//					shape.ProfilDef->createSolid3dProfil();
//	//			}
//	//		}
//	//	}
//	//}
//
//	/*WriteLogs();
//
//	for (auto& type : objects)
//	{
//		type.second.clear();
//	}
//
//	for (auto& el : Construction::s_ObjectVoids)
//		el.second.clear();
//
//	Construction::s_ObjectVoids.clear();
//	Construction::s_Styles.clear();
//	Construction::s_Logs.clear();
//
//	delete expressDataSet;*/
//}
