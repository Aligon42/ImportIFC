#include "Utils.h"

#include <stdlib.h>
#include <fstream>

#include "Autocad/Autocad.h"

const wchar_t* CharToWideChar(const char* c, ...)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

const std::string WideCharToString(const wchar_t* c)
{
	std::wstring wstring(c);
	std::string string(wstring.begin(), wstring.end());

	return string;
}

void WriteLogs()
{
	std::ofstream fw(Autocad::s_LogPath, std::ofstream::app);
	if (fw.is_open())
	{
		for (auto& vec : Autocad::s_Logs)
		{
			for (auto& str : vec.second)
				fw << str;

			vec.second.clear();
		}

		fw.close();
	}
}