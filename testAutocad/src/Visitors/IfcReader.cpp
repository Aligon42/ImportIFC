#include "IfcReader.h"

#include "ObjectVisitor.h"
#include "Utils/Utils.h"
#include "Autocad/Autocad.h"

#include <chrono>

IfcReader::IfcReader(const std::string& filePath, std::ifstream* stream)
	: m_FilePath(filePath), m_FileStream(stream) { }

bool IfcReader::Read()
{
	// TODO: A voir pour le flag inMemory
	return m_Reader.read(*m_FileStream, (std::ifstream::pos_type)0);
}

void IfcReader::GetReaderErrors(std::vector<std::string>& errors)
{
	errors = m_Reader.errors();
}

void IfcReader::GenerateExpressDataSetAndValidate()
{
	m_ExpressDataSet = dynamic_cast<ifc2x3::ExpressDataSet*>(m_Reader.getExpressDataSet());

	if (m_ExpressDataSet == NULL)
	{
		LogAutocad("Ho no ... there is no ExpressDataSet.\n");
	}

	// TODO: shouldWrite
	//if (shouldWrite)
	//{
	//	// ** Instantiate the model if we want to write it
	//	m_ExpressDataSet->instantiateAll(/*&cb*/);
	//}

	Step::RefLinkedList< ifc2x3::IfcProject > projects = m_ExpressDataSet->getAllIfcProject();
	if (projects.size() == 0) {
		LogAutocad("Strange ... there is no IfcProject\n");
	}
	else if (projects.size() > 1) {
		LogAutocad("Strange ... there more than one IfcProject\n");
	}
	else {
		Step::RefPtr< ifc2x3::IfcProject > project = &*(projects.begin());

		LogAutocad("Project name is: %s\n", CharToWideChar((project->getName().toISO_8859(Step::String::Western_European)).c_str()));
		if (Step::isUnset(project->getLongName().toISO_8859(Step::String::Western_European))) {
			project->setLongName("Je lui donne le nom que je veux");
		}
		LogAutocad("Project long name is: %s\n", CharToWideChar((project->getLongName().toISO_8859(Step::String::Western_European)).c_str()));
	}
}

void IfcReader::Visit()
{
	m_ExpressDataSet->instantiateAll();

	int startIndex = m_FilePath.find_last_of("\\") + 1;
	std::string filename = m_FilePath.substr(startIndex, m_FilePath.length() - 4 - startIndex) + ".txt";

	Autocad::s_LogPath = filename;
	Autocad::s_Logs.emplace(std::make_pair("Rendu", std::vector<std::string>()));
	Autocad::s_Logs.emplace(std::make_pair("Instantiation", std::vector<std::string>()));

	const auto& allElements = m_ExpressDataSet->getAllIfcElement().m_refList;

	int total = 0;
	for (auto& element : allElements)
	{
		if (element->size() == 0) continue;
		std::string type = (*element->begin()).second->type();
		if (type == "IfcOpeningElement") continue;
		total += element->size();
	}

	std::ofstream fw(Autocad::s_LogPath, std::ofstream::out);
	if (fw.is_open())
	{
		fw << "Nombre éléments : " << total << "\n";
		fw.close();
	}

#ifdef USETHREADS
	m_Threads.push_back(std::thread(&IfcReader::GetAllIfcRelVoids, this));
	m_Threads.push_back(std::thread(&IfcReader::GetAllIfcStyledItems, this));
#else
	GetAllIfcRelVoids();
	GetAllIfcStyledItems();
#endif

	GetAllIfcSites();
	GetAllIfcElements();

#ifdef USETHREADS
	for (auto& thread : m_Threads)
	{
		thread.join();
	}

	m_Threads.clear();
#endif

	DrawElements();

	WriteLogs();

	for (auto& type : m_Objects)
	{
		type.second.clear();
	}

	for (auto& el : Autocad::s_ObjectVoids)
		el.second.clear();

	Autocad::s_ObjectVoids.clear();
	Autocad::s_Styles.clear();
	Autocad::s_Logs.clear();

	delete m_ExpressDataSet;
}

void IfcReader::ExploreElement(std::map<Step::Id, Step::BaseObjectPtr>* elements, std::vector<std::shared_ptr<IFCObject>>& objects)
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

		//if (key != 110771) continue;

		ObjectVisitor visitor;
		buildingElement.acceptVisitor(&visitor);

		auto obj = visitor.getIfcObject();

		buildingElement.acceptVisitor(&placementVisitor);
		obj->LocalTransform = placementVisitor.getTransformation();

		objects.push_back(obj);

		auto end = std::chrono::high_resolution_clock::now() - start;
		std::stringstream ss;
		ss << "Key: " << key << " - " << entity << " - " << std::chrono::duration_cast<std::chrono::microseconds>(end).count() / 1000.0 << " ms. \n";

		Autocad::s_Logs[entity].push_back(ss.str());
	}
}

void IfcReader::GetAllIfcRelVoids()
{
	// IfcRelVoidsElement
	for (auto& voids : m_ExpressDataSet->getAllIfcRelVoidsElement())
	{
		ObjectVisitor visitor;
		voids.acceptVisitor(&visitor);
		auto obj = visitor.getIfcObject();

		voids.acceptVisitor(&m_PlacementVisitor);
		obj->LocalTransform = m_PlacementVisitor.getTransformation();

		obj->LocalTransform *= (*obj->ShapeRepresentations.begin()).Transformation;

		if (Autocad::s_ObjectVoids.find(obj->VoidKey) != Autocad::s_ObjectVoids.end())
		{
			Autocad::s_ObjectVoids[obj->VoidKey].push_back(obj);
		}
		else
		{
			std::vector<std::shared_ptr<IFCObject>> vec;
			vec.push_back(obj);
			Autocad::s_ObjectVoids.insert(std::make_pair(obj->VoidKey, vec));
		}
	}
}

void IfcReader::GetAllIfcStyledItems()
{
	// IfcStyledItem
	for (auto& styles : m_ExpressDataSet->getAllIfcStyledItem())
	{
		ObjectVisitor visitor;
		int key = styles.getKey();

		styles.acceptVisitor(&visitor);
		Style style = visitor.getStyle();
		Autocad::s_Styles.emplace(std::make_pair(style.keyItem, style));
	}
}

void IfcReader::GetAllIfcSites()
{
	for (auto ifcSite : m_ExpressDataSet->getAllIfcSite().m_refList)
	{
		if (ifcSite->size() > 0)
		{
			std::string type = (*ifcSite->begin()).second->type();

			if (type == "IfcOpeningElement") continue;

			Autocad::s_Logs.emplace(std::make_pair(type, std::vector<std::string>()));
			m_Objects.emplace(std::make_pair(type, std::vector<std::shared_ptr<IFCObject>>()));

#ifdef USETHREADS
			m_Threads.push_back(std::thread(&IfcReader::ExploreElement, this, ifcSite, std::ref(m_Objects[type])));
#else
			ExploreElement(ifcSite, std::ref(m_Objects[type]));
#endif
		}
	}
}

void IfcReader::GetAllIfcElements()
{
	for (auto element : m_ExpressDataSet->getAllIfcElement().m_refList)
	{
		if (element->size() > 0)
		{
			std::string type = (*element->begin()).second->type();

			if (type == "IfcOpeningElement") continue;

			Autocad::s_Logs.emplace(std::make_pair(type, std::vector<std::string>()));
			m_Objects.emplace(std::make_pair(type, std::vector<std::shared_ptr<IFCObject>>()));

#ifdef USETHREADS
			m_Threads.push_back(std::thread(&IfcReader::ExploreElement, this, element, std::ref(m_Objects[type])));
#else
			ExploreElement(element, std::ref(m_Objects[type]));
#endif
		}
	}
}

void IfcReader::DrawElements()
{
	for (auto& type : m_Objects)
	{
		for (auto& obj : type.second)
		{
			//if (obj->Key != 5557) continue;

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
							Autocad construction(obj);
							construction.Extrusion();
						}
					}
					else if (subShape.EntityType == "IfcBooleanClippingResult")
					{
						Autocad construction(obj);
						construction.Extrusion();
					}
					else if (subShape.EntityType == "IfcFacetedBrep" || subShape.EntityType == "IfcFaceBasedSurfaceModel" || subShape.EntityType == "IfcShellBasedSurfaceModel")
					{
						Autocad construction(obj);
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

						Autocad construction(obj);
						construction.Extrusion();
					}
				}
				else if (shape.EntityType == "IfcFacetedBrep" || shape.EntityType == "IfcFaceBasedSurfaceModel" || shape.EntityType == "IfcShellBasedSurfaceModel" && !obj->IsMappedItem)
				{
					Autocad construction(obj);
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
}
