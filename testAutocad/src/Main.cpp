#include "Import/ImportIfc.h"

#include <aced.h>
#include <rxregsvc.h> 
#include <tchar.h>

void importIfc()
{
	ImportIfc importIfc;
	importIfc.LoadIfc();
}

void exportIfc()
{
	// TODO: Export
}

void initApp()
{
	// register a command with the AutoCAD command mechanism
	acedRegCmds->addCommand(_T("IMPORT_COMMANDS"),
		_T("ImportIFC"),
		_T("ImportIFC"),
		ACRX_CMD_TRANSPARENT,
		importIfc);

	acedRegCmds->addCommand(_T("EXPORT_COMMANDS"),
		_T("ExportIFC"),
		_T("ExportIFC"),
		ACRX_CMD_TRANSPARENT,
		exportIfc);
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