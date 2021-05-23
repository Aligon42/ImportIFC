#pragma once
#include <string>

#include "DataObject.h"

struct BaseProfilDef : public BaseObject
{
    std::string Name;
    Vec3 VecteurExtrusion;
    Matrix4 Transform;
};

struct I_profilDef : public BaseProfilDef
{
    float OverallWidth;
    float OverallDepth;
    float webThickness;
    float flangeThickness;
    float filletRadius;
    float flangeEdgeRadius;
    float FlangeSlope;
    int nbArg;
};

struct L_profilDef : public BaseProfilDef
{
    float Depth;
    float Width;
    float Thickness;
    float FilletRadius;
    float EdgeRadius;
    float LegSlope;
    int nbArg;
};

struct T_profilDef : public BaseProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float FlangeEdgeRadius;
    float WebEdgeRadius;
    float WebSlope;
    float FlangeSlope;
    int nbArg;
};

struct U_profilDef : public BaseProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;
    float FlangeSlope;
    int nbArg;
};

struct C_profilDef : public BaseProfilDef
{
    float Depth;
    float Width;
    float WallThickness;
    float Girth;
    float InternalFilletRadius;
};

struct Z_profilDef : public BaseProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;
};

struct AsymmetricI_profilDef : public BaseProfilDef
{
    float OverallWidth;
    float OverallDepth;
    float WebThickness;
    float FlangeThickness;
    float FlangeFilletRadius;
    float TopFlangeWidth;
    float TopFlangeThickness;
    float TopFlangeFilletRadius;
};

struct CircleHollow_profilDef : public BaseProfilDef
{
    float Radius;
    float WallThickness;
};

struct RectangleHollow_profilDef : public BaseProfilDef
{
    float XDim;
    float YDim;
    float WallThickness;
    float InnerFilletRadius;
    float OuteerFilletRadius;
};

struct Rectangle_profilDef : public BaseProfilDef
{
    float XDim;
    float YDim;
};

struct Circle_profilDef : public BaseProfilDef
{
    float Radius;
};