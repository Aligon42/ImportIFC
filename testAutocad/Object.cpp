#include "Object.h"

#include "MethodeConstruction.h"

void I_profilDef::createSolid3dProfil(Style styleDessin)
{
	if (nbArg == 5)
		createSolid3dProfilIPE(*this, styleDessin);
	else
		createSolid3dProfilIPN(*this, styleDessin);
}

void L_profilDef::createSolid3dProfil(Style styleDessin)
{
	if (nbArg == 5)
		createSolid3dProfilL8(*this, styleDessin);
	else
		createSolid3dProfilL9(*this, styleDessin);
}

void T_profilDef::createSolid3dProfil(Style styleDessin)
{
	if (nbArg == 7)
		createSolid3dProfilT10(*this, styleDessin);
	else
		createSolid3dProfilT12(*this, styleDessin);
}

void U_profilDef::createSolid3dProfil(Style styleDessin)
{
	if (nbArg == 5)
		createSolid3dProfilUPE(*this, styleDessin);
	else
		createSolid3dProfilUPN(*this, styleDessin);
}

void C_profilDef::createSolid3dProfil(Style styleDessin)
{
	createSolid3dProfilC(*this, styleDessin);
}

void Z_profilDef::createSolid3dProfil(Style styleDessin)
{
	createSolid3dProfilZ(*this, styleDessin);
}

void AsymmetricI_profilDef::createSolid3dProfil(Style styleDessin)
{
	createSolid3dProfilAsyI(*this, styleDessin);
}

void CircleHollow_profilDef::createSolid3dProfil(Style styleDessin)
{
	createSolid3dProfilCircHollow(*this, styleDessin);
}

void RectangleHollow_profilDef::createSolid3dProfil(Style styleDessin)
{
	createSolid3dProfilRectHollow(*this, styleDessin);
}

void Rectangle_profilDef::createSolid3dProfil(Style styleDessin)
{
	createSolid3dProfilRectangle(*this, styleDessin);
}

void Circle_profilDef::createSolid3dProfil(Style styleDessin)
{
	createSolid3dProfilCircle(*this, styleDessin);
}
