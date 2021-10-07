#pragma once
#include <rxdefs.h>

void initApp();
void initAppEx();
void unloadApp();
void unloadAppEx();
void initAppScale();
void unloadAppScale();
extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt);