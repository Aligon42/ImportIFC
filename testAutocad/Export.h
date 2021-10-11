#pragma once

namespace Utils
{
	double RadianToDegree(const double rad);
	double DegreeToRadian(const double deg);
}

void ExportIFC();

static bool isCoveringExp;
static bool isPlateExp;
//static int sizeListePoints;
//static int sizeFaceArray;
static bool isPolyline;
static int iteratorPolyline = 0;
static std::vector<std::string> typeLoop;
static int indexTypeLoop;


