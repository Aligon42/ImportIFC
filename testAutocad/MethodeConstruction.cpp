#include "MethodeConstruction.h"




void createSolid3d(std::list<Vec3> points1, Vec3 VecteurExtrusion, Matrix4 tranform1)
{
    Acad::ErrorStatus es;

    ads_name polyName;

    ads_point ptres;

    AcGePoint3dArray ptArr;
    ptArr.setLogicalLength((int)(points1.size()) - 1);

    Vec3 pointOrigine = *points1.begin();
    points1.pop_front();

    int i = 0;

    for (const auto& point : points1)
    {
        ptArr[i].set(point.x(), point.y(), point.z());
        i++;
    }
    
    AcDb2dPolyline* pNewPline = new AcDb2dPolyline(
        AcDb::k2dSimplePoly, ptArr, 0.0, Adesk::kTrue);
    pNewPline->setColorIndex(3);

    //get the boundary curves of the polyline
    AcDbEntity* pEntity = NULL;
    if (pNewPline == NULL)
    {
        pEntity->close();
        return;
    }
    AcDbVoidPtrArray lines;
    pNewPline->explode(lines);
    pNewPline->close();

    // Create a region from the set of lines.
    AcDbVoidPtrArray regions;
    es = AcDbRegion::createFromCurves(lines, regions);
    if (Acad::eOk != es)
    {
        pNewPline->close();
        acutPrintf(L"\nFailed to create region\n");
        return;
    }

    AcDbRegion* pRegion = AcDbRegion::cast((AcRxObject*)regions[0]);
    // Extrude the region to create a solid.
    AcDb3dSolid* pSolid = new AcDb3dSolid();
    es = pSolid->extrude(pRegion, VecteurExtrusion.z(), 0.0);

    for (int i = 0; i < lines.length(); i++)
    {
        delete (AcRxObject*)lines[i];
    }

    for (int ii = 0; ii < regions.length(); ii++)
    {
        delete (AcRxObject*)regions[ii];
    }

    DeplacementObjet3D(pSolid, tranform1);

    AcDbObjectId savedExtrusionId = AcDbObjectId::kNull;
    if (Acad::eOk == es)
    {
        AcDbDatabase* pDb = curDoc()->database();
        AcDbObjectId modelId;
        modelId = acdbSymUtil()->blockModelSpaceId(pDb);
        AcDbBlockTableRecord* pBlockTableRecord;
        acdbOpenAcDbObject((AcDbObject*&)pBlockTableRecord, modelId, AcDb::kForWrite);
        pBlockTableRecord->appendAcDbEntity(pSolid);
        pBlockTableRecord->close();
        pSolid->close();
    }
    else
    {
        delete pSolid;
        acutPrintf(_T("Je ne fais rien du tout"));
        return;
    }
}


static void DeplacementObjet3D(AcDb3dSolid* pSolid, Matrix4 transform1) {

    // 3 source points
    AcGePoint3d srcpt1 = AcGePoint3d::AcGePoint3d(0, 0, 0);
    AcGePoint3d srcpt2 = AcGePoint3d::AcGePoint3d(0, 0, 1);
    AcGePoint3d srcpt3 = AcGePoint3d::AcGePoint3d(1, 0, 0);

    //double x1 = System.Math.Round(double.Parse(PointDeplacement[0]), 3); 
    double x1 = round(transform1.operator()(12));  //PointDeplacement x

    //double y1 = System.Math.Round(double.Parse(PointDeplacement[1]), 3);
    double y1 = round(transform1.operator()(13)); //PointDeplacement y

    //double z1 = System.Math.Round(double.Parse(PointDeplacement[2]), 3);
    double z1 = round(transform1.operator()(14)); //PointDeplacement z

    //double x2 = System.Math.Round(double.Parse(Direction1[0]), 3);
    double x2 = round(transform1.operator()(8)); //Direction1 x

    //double y2 = System.Math.Round(double.Parse(Direction1[1]), 3);
    double y2 = round(transform1.operator()(9)); //Direction1 y

    //double z2 = System.Math.Round(double.Parse(Direction1[2]), 3);
    double z2 = round(transform1.operator()(10)); //Direction1 z

    //double x3 = System.Math.Round(double.Parse(Direction2[0]), 3);
    double x3 = round(transform1.operator()(0)); //Direction2 x

    //double y3 = System.Math.Round(double.Parse(Direction2[1]), 3);
    double y3 = round(transform1.operator()(1)); //Direction2 y

    //double z3 = System.Math.Round(double.Parse(Direction2[2]), 3);
    double z3 = round(transform1.operator()(2)); //Direction2 z

    // 3 destination points
    AcGePoint3d destpt1 = AcGePoint3d::AcGePoint3d(x1, y1, z1);
    AcGePoint3d destpt2 = AcGePoint3d::AcGePoint3d(x1 + x2, y1 + y2, z1 + z2);
    AcGePoint3d destpt3 = AcGePoint3d::AcGePoint3d(x1 + x3, y1 + y3, z1 + z3);

    AcGePoint3d fromOrigin = srcpt1;
    AcGeVector3d fromXaxis = srcpt2 - srcpt1;
    AcGeVector3d fromYaxis = srcpt3 - srcpt1;
    AcGeVector3d fromZaxis = fromXaxis.crossProduct(fromYaxis);

    //ed.WriteMessage("\nVecteur Origine X : " + fromXaxis.X + " , " + fromXaxis.Y + " , " + fromXaxis.Z);
    //ed.WriteMessage("\nVecteur Origine Y : " + fromYaxis.X + " , " + fromYaxis.Y + " , " + fromYaxis.Z);
    //ed.WriteMessage("\nVecteur Origine Z : " + fromZaxis.X + " , " + fromZaxis.Y + " , " + fromZaxis.Z);

    AcGePoint3d toOrigin = destpt1;
    AcGeVector3d toXaxis = (destpt2 - destpt1).normal() * (fromXaxis.length());
    AcGeVector3d toYaxis = (destpt3 - destpt1).normal() * (fromYaxis.length());
    AcGeVector3d toZaxis = toXaxis.crossProduct(toYaxis);

    //ed.WriteMessage("\nVecteur Destination X : " + toXaxis.X + " , " + toXaxis.Y + " , " + toXaxis.Z);
    //ed.WriteMessage("\nVecteur Destination Y : " + toYaxis.X + " , " + toYaxis.Y + " , " + toYaxis.Z);
    //ed.WriteMessage("\nVecteur Destination Z : " + toZaxis.X + " , " + toZaxis.Y + " , " + toZaxis.Z);

    // Get the transformation matrix for aligning coordinate systems
    AcGeMatrix3d mat = AcGeMatrix3d::AcGeMatrix3d();
    mat = mat.alignCoordSys(fromOrigin, fromXaxis, fromYaxis, fromZaxis, toOrigin, toXaxis, toYaxis, toZaxis);

    pSolid->transformBy(mat);

}