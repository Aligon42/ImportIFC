#pragma once

#include "Utils/Object.h"
#include "ComputePlacementVisitor.h"

#include <fstream>

#include <ifc2x3/SPFReader.h>
#include <ifc2x3/ExpressDataSet.h>

#define USETHREADS
#ifdef USETHREADS
#include <thread>
#endif

class IfcReader
{
public:
	IfcReader(const std::string& filePath, std::ifstream* stream);
	bool Read();
	void GetReaderErrors(std::vector<std::string>& errors);
	void GenerateExpressDataSetAndValidate();
	void Visit();

private:
	void ExploreElement(std::map<Step::Id, Step::BaseObjectPtr>* elements, std::vector<std::shared_ptr<IFCObject>>& objects);
	void GetAllIfcRelVoids();
	void GetAllIfcStyledItems();
	void GetAllIfcSites();
	void GetAllIfcElements();
	void DrawElements();

private:
	std::string m_FilePath;
	std::ifstream* m_FileStream;
	ifc2x3::SPFReader m_Reader;
	ifc2x3::ExpressDataSet* m_ExpressDataSet;

	std::map<std::string, std::vector<std::shared_ptr<IFCObject>>> m_Objects;
	ComputePlacementVisitor m_PlacementVisitor;

#ifdef USETHREADS
	std::vector<std::thread> m_Threads;
#endif
};