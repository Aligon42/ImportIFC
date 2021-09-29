#include "Object.h"

#include "Autocad/Autocad.h"

void I_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	if (nbArg == 5)
		construction.CreateSolid3dProfilIPE(*this);
	else
		construction.CreateSolid3dProfilIPE(*this);
}

void L_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	if (nbArg == 5)
		construction.CreateSolid3dProfilL8(*this);
	else
		construction.CreateSolid3dProfilL9(*this);
}

void T_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	if (nbArg == 7)
		construction.CreateSolid3dProfilT10(*this);
	else
		construction.CreateSolid3dProfilT12(*this);
}

void U_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	if (nbArg == 5)
		construction.CreateSolid3dProfilUPE(*this);
	else
		construction.CreateSolid3dProfilUPN(*this);
}

void C_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	construction.CreateSolid3dProfilC(*this);
}

void Z_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	construction.CreateSolid3dProfilZ(*this);
}

void AsymmetricI_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	construction.CreateSolid3dProfilAsyI(*this);
}

void CircleHollow_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	construction.CreateSolid3dProfilCircHollow(*this);
}

void RectangleHollow_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	construction.CreateSolid3dProfilRectHollow(*this);
}

void Rectangle_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	construction.CreateSolid3dProfilRectangle(*this);
}

void Circle_profilDef::createSolid3dProfil()
{
	Autocad construction(this->ParentObject);

	construction.CreateSolid3dProfilCircle(*this);
}
