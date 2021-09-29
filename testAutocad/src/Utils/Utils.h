#pragma once

#include <string>

const wchar_t* CharToWideChar(const char* c, ...);
void WriteLogs();
const std::string WideCharToString(const wchar_t* c);