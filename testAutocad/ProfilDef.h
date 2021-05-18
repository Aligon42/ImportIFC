#pragma once

struct BaseProfilDef
{
    const char* Name;

    BaseProfilDef(const char* name) : Name(name) {}
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

    I_profilDef(const char* name) : BaseProfilDef(name) { }
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

    L_profilDef(const char* name) : BaseProfilDef(name) { }
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

    T_profilDef(const char* name) : BaseProfilDef(name) { }
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

    U_profilDef(const char* name) : BaseProfilDef(name) { }
};

struct C_profilDef : public BaseProfilDef
{
    float Depth;
    float Width;
    float WallThickness;
    float Girth;
    float InternalFilletRadius;

    C_profilDef(const char* name) : BaseProfilDef(name) { }
};

struct Z_profilDef : public BaseProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;

    Z_profilDef(const char* name) : BaseProfilDef(name) { }
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

    AsymmetricI_profilDef(const char* name) : BaseProfilDef(name) { }
};

struct CircleHollow_profilDef : public BaseProfilDef
{
    float Radius;
    float WallThickness;

    CircleHollow_profilDef(const char* name) : BaseProfilDef(name) { }
};

struct RectangleHollow_profilDef : public BaseProfilDef
{
    float XDim;
    float YDim;
    float WallThickness;
    float InnerFilletRadius;
    float OuteerFilletRadius;

    RectangleHollow_profilDef(const char* name) : BaseProfilDef(name) { }
};

struct Rectangle_profilDef : public BaseProfilDef
{
    float XDim;
    float YDim;

    Rectangle_profilDef(const char* name) : BaseProfilDef(name) { }
};

struct Circle_profilDef : public BaseProfilDef
{
    float Radius;

    Circle_profilDef(const char* name) : BaseProfilDef(name) { }
};