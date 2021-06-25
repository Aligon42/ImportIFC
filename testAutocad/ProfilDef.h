#pragma once
#include <string>
#include <vectorial/config.h>
#include <vectorial/vectorial.h>
#include <vectorial/simd4f.h>
#include <mathfu/vector_3.h>
#include <mathfu/matrix_4x4.h>

#include "Object.h"

typedef mathfu::Vector<float, 3> Vec3;
typedef mathfu::Matrix<float, 4> Matrix4;

struct ProfilDef
{
    std::string Name;
    std::string Entity;
    Vec3 VecteurExtrusion;
    float HauteurExtrusion;
    Matrix4 Transform;
    bool IsMappedItem;
    Matrix4 TransformationOperator3D;

    virtual void createSolid3dProfil(Style styleDessin) = 0;
};

struct I_profilDef : public ProfilDef
{
    float OverallWidth;
    float OverallDepth;
    float webThickness;
    float flangeThickness;
    float filletRadius;
    float flangeEdgeRadius;
    float FlangeSlope;
    int nbArg;

    void createSolid3dProfil(Style styleDessin) override;
};

struct L_profilDef : public ProfilDef
{
    float Depth;
    float Width;
    float Thickness;
    float FilletRadius;
    float EdgeRadius;
    float LegSlope;
    int nbArg;

    void createSolid3dProfil(Style styleDessin) override;
};

struct T_profilDef : public ProfilDef
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

    void createSolid3dProfil(Style styleDessin) override;
};

struct U_profilDef : public ProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;
    float FlangeSlope;
    int nbArg;

    void createSolid3dProfil(Style styleDessin) override;
};

struct C_profilDef : public ProfilDef
{
    float Depth;
    float Width;
    float WallThickness;
    float Girth;
    float InternalFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Z_profilDef : public ProfilDef
{
    float Depth;
    float FlangeWidth;
    float WebThickness;
    float FlangeThickness;
    float FilletRadius;
    float EdgeRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct AsymmetricI_profilDef : public ProfilDef
{
    float OverallWidth;
    float OverallDepth;
    float WebThickness;
    float FlangeThickness;
    float FlangeFilletRadius;
    float TopFlangeWidth;
    float TopFlangeThickness;
    float TopFlangeFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct CircleHollow_profilDef : public ProfilDef
{
    float Radius;
    float WallThickness;

    void createSolid3dProfil(Style styleDessin) override;
};

struct RectangleHollow_profilDef : public ProfilDef
{
    float XDim;
    float YDim;
    float WallThickness;
    float InnerFilletRadius;
    float OuteerFilletRadius;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Rectangle_profilDef : public ProfilDef
{
    float XDim;
    float YDim;

    void createSolid3dProfil(Style styleDessin) override;
};

struct Circle_profilDef : public ProfilDef
{
    float Radius;

    void createSolid3dProfil(Style styleDessin) override;
};