#pragma once

const wchar_t* PathBlock(const ACHAR* cheminBlock, const ACHAR* nomBlock);
const wchar_t* nomBlockH(const ACHAR* nomBlock, int m, int longueur, int largeur);
int block(void);
AcDbObjectIdArray postToDatabase(AcDbVoidPtrArray eSet, AcDb3dSolid* pSolid, int typeBool, double rotation, AcGePoint3d origin);
int ajusterBlocAvecContour(void);
