#pragma once
#include <rxdefs.h>

void initApp();
void initAppEx();
void unloadApp();
void unloadAppEx();
extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt);