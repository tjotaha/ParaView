/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "GeomF.hh"

// Description:
// Construct with all types of clipping turned off.
vlGeometryFilter::vlGeometryFilter()
{
  this->PointMinimum = 0;
  this->PointMaximum = LARGE_INTEGER;

  this->CellMinimum = 0;
  this->CellMaximum = LARGE_INTEGER;

  this->Extent[0] = -LARGE_FLOAT;
  this->Extent[1] = LARGE_FLOAT;
  this->Extent[2] = -LARGE_FLOAT;
  this->Extent[3] = LARGE_FLOAT;
  this->Extent[4] = -LARGE_FLOAT;
  this->Extent[5] = LARGE_FLOAT;

  this->PointClipping = 0;
  this->CellClipping = 0;
  this->ExtentClipping = 0;
}

void vlGeometryFilter::SetExtent(float xMin, float xMax, float yMin,
                                     float yMax, float zMin, float zMax)
{
  float extent[6];

  extent[0] = xMin;
  extent[1] = xMax;
  extent[2] = yMin;
  extent[3] = yMax;
  extent[4] = zMin;
  extent[5] = zMax;

  this->SetExtent(extent);
}

// Description:
// Specify a (xmin,xmax, ymin,ymax, zmin,zmax) bounding box to clip data.
void vlGeometryFilter::SetExtent(float *extent)
{
  int i;

  if ( extent[0] != this->Extent[0] || extent[1] != this->Extent[1] ||
  extent[2] != this->Extent[2] || extent[3] != this->Extent[3] ||
  extent[4] != this->Extent[4] || extent[5] != this->Extent[5] )
    {
    this->Modified();
    for (i=0; i<3; i++)
      {
      if ( extent[2*i] < 0 ) extent[2*i] = 0;
      if ( extent[2*i+1] < extent[2*i] ) extent[2*i+1] = extent[2*i];
      this->Extent[2*i] = extent[2*i];
      this->Extent[2*i+1] = extent[2*i+1];
      }
    }
}

void vlGeometryFilter::Execute()
{
  int cellId, i, j;
  int numPts=this->Input->GetNumberOfPoints();
  int numCells=this->Input->GetNumberOfCells();
  char *cellVis;
  vlCell *cell, *face, *cellCopy;
  float *x;
  vlIdList *ptIds;
  static vlIdList cellIds(MAX_CELL_SIZE);
  vlFloatPoints *newPts;
  int ptId;
  int npts, pts[MAX_CELL_SIZE];
  vlPointData *pd = this->Input->GetPointData();
  int allVisible;

  this->Initialize();

  if ( (!this->CellClipping) && (!this->PointClipping) && 
  (!this->ExtentClipping) )
    {
    allVisible = 1;
    cellVis = NULL;
    }
  else
    {
    allVisible = 0;
    cellVis = new char[numCells];
    }
//
// Mark cells as being visible or not
//
  if ( ! allVisible )
    {
    for(cellId=0; cellId < numCells; cellId++)
      {
      if ( this->CellClipping && cellId < this->CellMinimum ||
      cellId > this->CellMaximum )
        {
        cellVis[cellId] = 0;
        }
      else
        {
        cell = this->Input->GetCell(cellId);
        ptIds = cell->GetPointIds();
        for (i=0; i < ptIds->GetNumberOfIds(); i++) 
          {
          ptId = ptIds->GetId(i);
          x = this->Input->GetPoint(ptId);

          if ( (this->PointClipping && (ptId < this->PointMinimum ||
          ptId > this->PointMaximum) ) &&
          (this->ExtentClipping && 
          (x[0] < this->Extent[0] || x[0] > this->Extent[1] ||
          x[1] < this->Extent[2] || x[1] > this->Extent[3] ||
          x[2] < this->Extent[4] || x[2] > this->Extent[5] )) )
            {
            cellVis[cellId] = 0;
            break;
            }
          }
        if ( i >= ptIds->GetNumberOfIds() ) cellVis[cellId] = 1;
        }
      }
    }
//
// Allocate
//
  newPts = new vlFloatPoints(numPts,numPts/2);
  this->Allocate(4*numCells,numCells/2);
  this->PointData.CopyAllocate(pd,numPts,numPts/2);
//
// Traverse cells to extract geometry
//
  for(cellId=0; cellId < numCells; cellId++)
    {
    if ( allVisible || cellVis[cellId] )
      {
      cell = this->Input->GetCell(cellId);
      switch (cell->GetCellDimension())
        {
        // create new points and then cell
        case 0: case 1: case 2:
          
          npts = cell->GetNumberOfPoints();
          for ( i=0; i < npts; i++)
            {
            ptId = cell->GetPointId(i);
            x = this->Input->GetPoint(ptId);
            pts[i] = newPts->InsertNextPoint(x);
            this->PointData.CopyData(pd,ptId,pts[i]);
            }
          this->InsertNextCell(cell->GetCellType(), npts, pts);
          break;

        case 3:
          cellCopy = cell->MakeObject();
          for (j=0; j < cellCopy->GetNumberOfFaces(); j++)
            {
            face = cellCopy->GetFace(j);
            this->Input->GetCellNeighbors(cellId, face->PointIds, cellIds);
            if ( cellIds.GetNumberOfIds() <= 0 || 
            (!allVisible && !cellVis[cellIds.GetId(0)]) )
              {
              npts = face->GetNumberOfPoints();
              for ( i=0; i < npts; i++)
                {
                ptId = face->GetPointId(i);
                x = this->Input->GetPoint(ptId);
                pts[i] = newPts->InsertNextPoint(x);
                this->PointData.CopyData(pd,ptId,pts[i]);
                }
              this->InsertNextCell(face->GetCellType(), npts, pts);
              }
            }
            delete cellCopy;
          break;

        } //switch
      } //if visible
    } //for all cells
//
// Update ourselves
//
  this->SetPoints(newPts);
  this->Squeeze();

  if ( cellVis ) delete [] cellVis;

}

void vlGeometryFilter::PrintSelf(ostream& os, vlIndent indent)
{
  vlDataSetToPolyFilter::PrintSelf(os,indent);

  os << indent << "Point Minimum : " << this->PointMinimum << "\n";
  os << indent << "Point Maximum : " << this->PointMaximum << "\n";

  os << indent << "Cell Minimum : " << this->CellMinimum << "\n";
  os << indent << "Cell Maximum : " << this->CellMaximum << "\n";

  os << indent << "Extent: \n";
  os << indent << "  Xmin,Xmax: (" << this->Extent[0] << ", " << this->Extent[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->Extent[2] << ", " << this->Extent[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->Extent[4] << ", " << this->Extent[5] << ")\n";

  os << indent << "PointClipping: " << (this->PointClipping ? "On\n" : "Off\n");
  os << indent << "CellClipping: " << (this->CellClipping ? "On\n" : "Off\n");
  os << indent << "ExtentClipping: " << (this->ExtentClipping ? "On\n" : "Off\n");

}

