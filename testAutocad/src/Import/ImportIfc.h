#pragma once

#include "Utils/Object.h"

#include <map>

class ImportIfc
{
public:
	ImportIfc() {}
	~ImportIfc();

	void LoadIfc();

private:
	void SelectFile();
	void ReadFile();

private:
	std::string m_FilePath;
};