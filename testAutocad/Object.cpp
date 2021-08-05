#include "Object.h"

#include "Construction.h"

void I_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	if (nbArg == 5)
		construction.CreateSolid3dProfilIPE(*this, styleDessin);
	else
		construction.CreateSolid3dProfilIPE(*this, styleDessin);
}

void L_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	if (nbArg == 5)
		construction.CreateSolid3dProfilL8(*this, styleDessin);
	else
		construction.CreateSolid3dProfilL9(*this, styleDessin);
}

void T_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	if (nbArg == 7)
		construction.CreateSolid3dProfilT10(*this, styleDessin);
	else
		construction.CreateSolid3dProfilT12(*this, styleDessin);
}

void U_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	if (nbArg == 5)
		construction.CreateSolid3dProfilUPE(*this, styleDessin);
	else
		construction.CreateSolid3dProfilUPN(*this, styleDessin);
}

void C_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	construction.CreateSolid3dProfilC(*this, styleDessin);
}

void Z_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	construction.CreateSolid3dProfilZ(*this, styleDessin);
}

void AsymmetricI_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	construction.CreateSolid3dProfilAsyI(*this, styleDessin);
}

void CircleHollow_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	construction.CreateSolid3dProfilCircHollow(*this, styleDessin);
}

void RectangleHollow_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	construction.CreateSolid3dProfilRectHollow(*this, styleDessin);
}

void Rectangle_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	construction.CreateSolid3dProfilRectangle(*this, styleDessin);
}

void Circle_profilDef::createSolid3dProfil(Style styleDessin)
{
	Construction construction;

	construction.CreateSolid3dProfilCircle(*this, styleDessin);
}
